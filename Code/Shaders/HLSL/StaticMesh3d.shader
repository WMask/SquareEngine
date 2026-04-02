// StaticMesh3d.shader

#include "ShaderUtils.hlsli"

cbuffer VS_WVP_BUFFER : register(b0)
{
	row_major float4x4 mTrans;
	row_major float4x4 mView;
	row_major float4x4 mProj;
};

cbuffer VS_SETTINGS_BUFFER : register(b1)
{
	float4 vGlobalTint;
};

cbuffer VS_LIGHTS_BUFFER : register(b2)
{
	float4 vLightVec[MaxLights];   // w - light type: <0.5 - directional, >0.5 - point
	float4 vLightColor[MaxLights]; // w - distance if point light type
	uint numLights;
};

struct VS_INPUT
{
	float3 vPosition : POSITION;
	float3 vNormal   : NORMAL;
	float2 vTexUV    : TEXCOORD;
	float3 iPosition : INSTANCEPOS;
	float4 iRotation : INSTANCEROT;
	float3 iScale    : INSTANCESCALE;
	float3 iTint     : INSTANCECOLOR;
};

struct VS_OUT
{
	float4 vPosition : SV_POSITION;
	float3 vNormal   : NORMAL;
	float2 vTexUV    : TEXCOORD;
	float4 vTint     : COLOR;
};

VS_OUT VShader(VS_INPUT input)
{
	VS_OUT output;

	float3 vPos3 = (input.vPosition * input.iScale);
	float3 vRotatedPos3 = QuaternionRotate(input.iRotation, vPos3);
	float4 vWorldPos4 = float4(vRotatedPos3 + input.iPosition, 1.0);
	float4 vNormal4 = float4(input.vNormal, 1.0);

	float4x4 mWVP = mul(mTrans, mul(mView, mProj));

	output.vPosition = mul(vWorldPos4, mWVP);
	output.vNormal = normalize(mul(vNormal4, mTrans).xyz);
	output.vTexUV = input.vTexUV;

	float3 vLight = float3(0.0, 0.0, 0.0);
	for (uint i = 0; i < numLights; i++)
	{
		if (IsDirectional(vLightVec[i]))
		{
			float3 vDir = normalize(vLightVec[i].xyz);
			float3 vNewLight = CalculateDirectionalLight(vDir, vLightColor[i].xyz, output.vNormal, output.vPosition.xyz);
			vLight += vNewLight;
		}
	}

	output.vTint = float4(input.iTint * vLight, 1.0);

	return output;
}

Texture2D tex2D;

SamplerState linearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PShader(float4 vPosition : SV_POSITION, float3 vNormal : NORMAL, float2 vTexUV : TEXCOORD, float4 vTint : COLOR) : SV_TARGET
{
	float4 vTexColor = tex2D.Sample(linearSampler, vTexUV);
	return vTexColor * vTint * vGlobalTint;
}
