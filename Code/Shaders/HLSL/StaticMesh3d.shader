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
};

cbuffer MESH_BUFFER : register(b2)
{
	uint  bHasBaseTexture;
	uint  bHasNormalTexture;
	uint  bHasPbrTexture;
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
	float2 vTexUV     : TEXCOORD;
	float3 vNormal    : NORMAL0;
    float3 vTangent   : TANGENT;
    float3 vBitangent : BITANGENT;
    float3 vLightDir  : NORMAL1;
	float4 vLightClr  : COLOR0;
	float4 iTint      : COLOR1;
};

PS_INPUT VShader(VS_INPUT input)
{
	PS_INPUT output;

	float3 vPos3 = (input.vPosition * input.iScale);
	float3 vRotatedPos3 = QuaternionRotate(input.iRotation, vPos3);
	float4 vWorldPos4 = float4(vRotatedPos3 + input.iPosition, 1.0);

	float3x3 mWorld = (float3x3)mTrans;
	float4x4 mWVP = mul(mTrans, mul(mView, mProj));

	output.vPosition = mul(vWorldPos4, mWVP);
	output.vNormal = normalize(mul(input.vNormal, mWorld));
	output.vTexUV = input.vTexUV;
	output.iTint = float4(input.iTint, 1.0);

	// compute tangents
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
			float3 vNewLight = CalculateDirectionalLight(vDir, vLightColor[i].xyz, output.vNormal, output.vPosition.xyz);
			vAccColor += vNewLight;
			output.vLightDir = -vDir; // only one directional light
		}
	}
	output.vLightClr = float4(vAccColor, 1.0);

	return output;
}

Texture2D baseTex2D : register(t0);
Texture2D normTex2D : register(t1);
Texture2D pbrTex2D  : register(t2);

SamplerState linearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PShader(PS_INPUT input) : SV_TARGET
{
	if (bHasBaseTexture)
	{
		float4 vColor = baseTex2D.Sample(linearSampler, input.vTexUV);
		float3 N = input.vNormal;

		if (bHasNormalTexture)
		{
			float3 vNormalMap = normTex2D.Sample(linearSampler, input.vTexUV).rgb;
			vNormalMap = normalize(vNormalMap * 2.0 - 1.0);

			float3x3 mTBN = float3x3(
				input.vTangent,
				input.vBitangent,
				input.vNormal
			);

			N = normalize(mul(vNormalMap, mTBN));
		}

		float NdotL = max(dot(N, input.vLightDir), 0.0);
		float normalScale = min(NdotL, 1.0);

		float4 vFinalColor = float4(vColor.rgb * normalScale, vColor.a);
		return vFinalColor * (input.iTint * vGlobalTint) * input.vLightClr;
	}

	return (input.iTint * vGlobalTint) * input.vLightClr;
}
