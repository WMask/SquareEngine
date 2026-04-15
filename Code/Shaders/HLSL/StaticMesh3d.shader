// StaticMesh3d.shader
// Based on DirectXTK shaders
// http://go.microsoft.com/fwlink/?LinkId=248929

#include "ShaderUtils.hlsli"

cbuffer VSMatrixBuffer : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
	float3x3 mNormal;
};

cbuffer VSPSSettingsBuffer : register(b1)
{
	float4 vCameraPos;
	float4 vViewDir;
	float4 vGlobalTint;
	float2 vScreenSize;
	float2 padding;
	float4 vBackLight;
	float4 vPbrGammaCorrection;
};

cbuffer VSPSMaterialBuffer : register(b2)
{
	uint bHasBaseTexture;
	uint bHasNormalTexture;
	uint bHasRMATexture;
	uint bHasEmissiveTexture;
};

cbuffer PSCubemapsBuffer : register(b3)
{
	uint  bHasDiffuseCubemap;  // diffuse ambient light (radiance)
	uint  bHasSpecularCubemap; // specular reflection (irradiance)
	float diffuseAmount;
	float specularAmount;
	uint  maxCubemapMipLevels;
};

cbuffer PSLightsBuffer : register(b4)
{
	float4 vLightVec[kMaxLights];   // xyz - direction for directional, position for point
	float4 vLightColor[kMaxLights]; // rgb - color, a - distance if point light type
	uint numLights;
};

struct VSOutputNmTx
{
    float3 vPositionWS : POSITION;
    float3 vNormalWS   : NORMAL0;
    float3 vTangent    : NORMAL1;
    float3 vBitangent  : NORMAL2;
    float2 vTexCoord   : TEXCOORD;
    float4 vPositionPS : SV_Position;
};

struct PSInputNmTx
{
    float3 vPositionWS : POSITION;
    float3 vNormalWS   : NORMAL0;
    float3 vTangent    : NORMAL1;
    float3 vBitangent  : NORMAL2;
    float2 vTexCoord   : TEXCOORD;
};

struct VSInputNmTxInst
{
    float4 vPosition : SV_Position;
    float3 vNormal   : NORMAL;
    float3 vTangent  : TANGENT;
    float2 vTexCoord : TEXCOORD;
	float3 iPosition : INSTANCEPOS;
	float4 iRotation : INSTANCEROT;
	float3 iScale    : INSTANCESCALE;
	float3 iTint     : INSTANCECOLOR;
};


VSOutputNmTx VShader(VSInputNmTxInst input)
{
    VSOutputNmTx output;

	float3 vScaledPos3 = (input.vPosition.xyz * input.iScale);
	float3 vRotatedPos3 = QuaternionRotate(input.iRotation, vScaledPos3);
	float4 vWorldPos4 = float4(vRotatedPos3 + input.iPosition, 1.0);

	float4x4 mWVP = mul(mWorld, mul(mView, mProj));

    output.vPositionPS = mul(vWorldPos4, mWVP);
    output.vTexCoord   = input.vTexCoord;

	// Build TBN in world space
	float3x3 mWorld3x3 = (float3x3)mWorld;
	float3 N = normalize(mul(input.vNormal, mWorld3x3));
	float3 T = normalize(mul(input.vTangent, mWorld3x3));

	// Gram-Schmidt orthogonalization
	T = normalize(T - N * dot(T, N));
	float3 B = cross(N, T);

    output.vPositionWS = mul(vWorldPos4.xyz, mWorld3x3).xyz;
	output.vNormalWS  = N;
	output.vTangent   = T;
	output.vBitangent = B;

    return output;
}


Texture2D<float4> AlbedoTexture   : register(t0);
Texture2D<float3> NormalTexture   : register(t1);
Texture2D<float3> RMATexture      : register(t2);
Texture2D<float3> EmissiveTexture : register(t3);

// Specular reflection cubemap
TextureCube<float3> SpecularTexture : register(t4);

sampler SurfaceSampler : register(s0);
sampler CubemapSampler : register(s1);


float3 GetLightDirection()
{
	if (numLights > 0)
	{
		return vLightVec[0].xyz;
	}

	return normalize(float3(-1.0, -1.0, -1.0));
}

float3 GetLightColor()
{
	if (numLights > 0)
	{
		return vLightColor[0].rgb;
	}

	return normalize(float3(1.0, 1.0, 1.0));
}


float4 PShader(PSInputNmTx input) : SV_Target0
{
	float4 vAlbedo = AlbedoTexture.Sample(SurfaceSampler, input.vTexCoord);
	float roughness = 1.0;
	float metallic = 0.0;
	float ao = 1.0;

	// Basic normal
	const float3 vRadiance = GetLightColor();
	const float3 L = -GetLightDirection();
	float3 N = input.vNormalWS;
	float NdotL = max(dot(N, L), 0.0);
	float NdotL2 = max(dot(N, -L), 0.0);

	// Normal map
	if (bHasNormalTexture)
	{
		// Reconstruct world normal from normal map
		float3 vNormalMap = NormalTexture.Sample(SurfaceSampler, input.vTexCoord).rgb * 2.0 - 1.0;
		N = normalize(
			(vNormalMap.x * input.vTangent) + 
			(vNormalMap.y * input.vBitangent) + 
			(vNormalMap.z * input.vNormalWS)
		);

		NdotL = max(dot(N, L), 0.0);
		NdotL2 = max(dot(N, -L), 0.0);
	}

	float3 vFinalColor = vAlbedo.rgb * vRadiance * ao * NdotL;

	if (bHasRMATexture)
	{
		const float3 vRMA = RMATexture.Sample(SurfaceSampler, input.vTexCoord);
		roughness = max(vRMA.r, 0.04);
		metallic = vRMA.g;
		ao = vRMA.b;

		float3 V = normalize(vCameraPos.xyz - input.vPositionWS);
		const float NdotV = max(dot(N, V), 0.0);
		float3 H = normalize(V + L);

		// Cook-Torrance BRDF
		float3 F0 = lerp(float3(0.04, 0.04, 0.04), vAlbedo.rgb, metallic);
		float3 F  = FresnelSchlick(max(dot(H, V), 0.0), F0);

		float NDF = DistributionGGX(N, H, roughness);
		float G   = GeometrySmith(N, V, L, roughness);

		float3 numerator = NDF * G * F;
		float  denominator = 4.0 * NdotV * NdotL;
		float3 vSpecular = numerator / max(denominator, 0.001);

		float3 kS = F;
		float3 kD = (float3(1.0, 1.0, 1.0) - kS) * (1.0 - metallic);
		float3 vDiffuse = kD * vAlbedo.rgb / PI;

		// Environment cubemap specular reflection
		float3 vDir = reflect(-V, N);
		float mip = roughness * maxCubemapMipLevels;
		float3 vEnvColor = SpecularTexture.SampleLevel(CubemapSampler, vDir, mip).rgb;
		float3 vFresnelAmount = lerp(float3(1.0, 1.0, 1.0), NdotL, 0.5); // Reduce env amount on dark side
		float3 vEnvAmount = FresnelSchlick(max(dot(N, V), 0.0), F0) * vFresnelAmount;
		float3 vIndirectSpecular = vEnvColor * vEnvAmount * specularAmount * kIndirectSpecularScale;
		vIndirectSpecular *= metallic; // No specular reflection on non-metals

		// Apply directional light
		vFinalColor = (vDiffuse + vSpecular) * vRadiance * ao * NdotL + vIndirectSpecular;

		// Gamma correction
		vFinalColor = vFinalColor * vPbrGammaCorrection.rgb * kGammaCorrectionScale;
	}

	// Add back light
	float3 vBackColor = vAlbedo.rgb * vBackLight * NdotL2 * (1.0 - metallic);
	vFinalColor += vBackColor;

	// Add emissive
	if (bHasEmissiveTexture)
	{
		vFinalColor += EmissiveTexture.Sample(SurfaceSampler, input.vTexCoord).rgb;
	}

    return float4(vFinalColor, vAlbedo.w);
}
