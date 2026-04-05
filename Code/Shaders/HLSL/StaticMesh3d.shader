// StaticMesh3d.shader

#include "ShaderUtils.hlsli"

cbuffer VS_WVP_BUFFER : register(b0)
{
	row_major float4x4 mTrans;
	row_major float4x4 mView;
	row_major float4x4 mProj;
};

cbuffer VS_LIGHTS_BUFFER : register(b3)
{
	float4 vLightVec[MaxLights];   // w - light type: <0.5 - directional, >0.5 - point
	float4 vLightColor[MaxLights]; // w - distance if point light type
	uint numLights;
};

cbuffer SETTINGS_BUFFER : register(b1)
{
	float4 vGlobalTint;
	float4 vCameraPos;
	float4 vViewDir;
	float  envCubemapAmount; // 0-1
	uint   bHasEnvCubemap;
};

cbuffer MATERIAL_BUFFER : register(b2)
{
	uint  bHasBaseTexture;
	uint  bHasNormalTexture;
	uint  bHasORMTexture;
	float subSurfaceAmount; // is two-sided
};

struct VS_INPUT
{
	float3 vPosition : POSITION;
	float3 vNormal   : NORMAL;
    float3 vTangent  : TANGENT;
	float2 vTexUV    : TEXCOORD;
	float3 iPosition : INSTANCEPOS;
	float4 iRotation : INSTANCEROT;
	float3 iScale    : INSTANCESCALE;
	float3 iTint     : INSTANCECOLOR;
};

struct PS_INPUT
{
	float4 vPosition  : SV_POSITION;
	float3 vWorldPos  : POSITION;
	float2 vTexUV     : TEXCOORD;
	float3 vNormal    : NORMAL0;
    float3 vTangent   : TANGENT;
    float3 vBitangent : BITANGENT;
    float3 vLightDir  : NORMAL1;
	float3 vLightClr  : COLOR0;
	float4 iTint      : COLOR1;
};

PS_INPUT VShader(VS_INPUT input)
{
	PS_INPUT output;

	float3 vPos3 = (input.vPosition * input.iScale);
	float3 vRotatedPos3 = QuaternionRotate(input.iRotation, vPos3);
	float4 vWorldPos4 = float4(vRotatedPos3 + input.iPosition, 1.0);

	float4x4 mWVP = mul(mTrans, mul(mView, mProj));

	output.vPosition = mul(vWorldPos4, mWVP);
	output.vTexUV = input.vTexUV;
	output.iTint = float4(input.iTint, 1.0);

	// compute normals data
	float3x3 mWorld = (float3x3)mTrans;
	output.vWorldPos = mul(input.vPosition.xyz, mWorld);
	output.vNormal = normalize(mul(input.vNormal, mWorld));
	if (bHasNormalTexture)
	{
		output.vTangent = normalize(mul(input.vTangent, mWorld));
		output.vBitangent = normalize(cross(output.vNormal, output.vTangent));
	}

	// apply lights
	float3 vAccColor = float3(0.0, 0.0, 0.0);
	for (uint i = 0; i < numLights; i++)
	{
		if (IsDirectional(vLightVec[i]))
		{
			float3 vDir = normalize(vLightVec[i].xyz);
			float3 vNewLight = CalculateDirectionalLight(vDir, output.vNormal, vLightColor[i].xyz);
			vAccColor += vNewLight;
			output.vLightDir = vDir; // only one directional light
		}
	}
	output.vLightClr = vAccColor;

	return output;
}

Texture2D baseTex   : register(t0);
Texture2D normTex   : register(t1);
Texture2D ormTex    : register(t2);
TextureCube envMap  : register(t3);

SamplerState linearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

// Get normal from map or use vertex normal
float3 GetFinalNormal(PS_INPUT input)
{
	float3 N = normalize(input.vNormal);

	if (bHasNormalTexture)
	{
		float3 vNormalMap = normTex.Sample(linearSampler, input.vTexUV).rgb;
		vNormalMap = normalize(vNormalMap * 2.0 - 1.0);

		float3x3 mTBN = float3x3(
			input.vTangent,
			input.vBitangent,
			input.vNormal
		);

		N = normalize(mul(vNormalMap, mTBN));
	}

	return N;
}

float4 PShader(PS_INPUT input) : SV_TARGET
{
	if (bHasBaseTexture)
	{
		// color
		float4 vAlbedoData = baseTex.Sample(linearSampler, input.vTexUV);
		float3 vAlbedo = vAlbedoData.rgb;
		float alpha = vAlbedoData.a;

		// normal and light
		float3 N = GetFinalNormal(input);
		float3 L = normalize(-input.vLightDir);
		float3 V = normalize(vCameraPos.xyz - input.vWorldPos);
		float NdotL = max(dot(N, L), 0.0);
		float NdotL2 = max(dot(N, -L), 0.0);

		float3 vBackLight = vAlbedo * NdotL2 * envCubemapAmount;
		float3 vDirectLight = vAlbedo * input.vLightClr * NdotL;
		float3 vFinalColor = vDirectLight + vBackLight;

		// pbr
		if (bHasORMTexture)
		{
			float3 vORM = ormTex.Sample(linearSampler, input.vTexUV).rgb;
			float metallic = vORM.b;
			float roughness = vORM.g;
			float ao = vORM.r;

			// fresnel
			float3 H = normalize(V + L);
			float3 F0 = lerp(float3(0.04, 0.04, 0.04), vAlbedo, metallic);
			float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

			// normal distribution (GGX)
			float NDF = DistributionGGX(N, H, roughness);
			float NGL = GeometrySmith(N, V, L, roughness);
		
			// specular BRDF
			float3 numerator = NDF * NGL * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
			float3 specular = numerator / denominator;

			// diffuse (energy conservation)
			float3 kD = (1.0 - F) * (1.0 - metallic);
			float3 diffuse = kD * vAlbedo / 3.14159;

			vDirectLight = (diffuse + specular) * input.vLightClr * NdotL;
			vDirectLight *= ao;

			// environment cubemap
			float3 vEnvColor = float3(0.0, 0.0, 0.0);
			if (bHasEnvCubemap)
			{
				float3 R = reflect(-V, N);
				float3 vEnvSpecular = envMap.Sample(linearSampler, R).rgb;

				// Simple diffuse irradiance
				float3 vEnvDiffuse = envMap.Sample(linearSampler, N).rgb;
			
				// Apply Fresnel and metallic to environment
				float3 vEnvSpec = F * vEnvSpecular * (1.0 - roughness);
				float3 vEnvDiff = kD * vEnvDiffuse * ao;

				vEnvColor = (vEnvSpec + vEnvDiff) * metallic;
				float3 vBackEnv = vEnvDiffuse * NdotL2 * envCubemapAmount;
				vBackLight = vBackLight + vBackEnv;
			}
			else
			{
				vBackLight = vBackLight * 1.5;
			}

			// gamma correction
			vFinalColor = AdjustSaturation(vDirectLight, 1.5) + vEnvColor;
			vFinalColor = pow(vFinalColor, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2)) + vBackLight;
		}
		else
		{
			// gamma correction
			vFinalColor = pow(vFinalColor, float3(1.0 / 1.2, 1.0 / 1.2, 1.0 / 1.2));
		}

		return float4(vFinalColor, alpha) * input.iTint * vGlobalTint;
	}

	return input.iTint * vGlobalTint * float4(input.vLightClr, 1.0);
}
