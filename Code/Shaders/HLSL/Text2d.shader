// Text2d.shader

#include "ShaderUtils.hlsli"

cbuffer VS_PROJ_BUFFER : register(b0)
{
	row_major float4x4 mProj;
};

cbuffer VS_VIEW_BUFFER : register(b1)
{
	row_major float4x4 mView;
};

cbuffer VS_TRANS_BUFFER : register(b2)
{
	row_major float4x4 mTrans;
};

cbuffer VS_SETTINGS_BUFFER : register(b6)
{
	float4 vGlobalTint;
};

struct VS_INPUT
{
    uint   vVertexID : SV_VertexID;
	float3 vPosition : POSITION;
	float3 iPosition : INSTANCEPOS;
	float2 iScale    : INSTANCESCALE;
	float4 iColor    : INSTANCECOLOR;
	float2 iTexUV[4] : INSTANCEUV;
};

struct VS_OUT
{
	float4 vPosition : SV_POSITION;
	float4 vColor    : COLOR0;
	float2 vTexUV    : TEXCOORD;
};

VS_OUT VShader(VS_INPUT input)
{
	VS_OUT output;

	float2 vPos2D = (input.vPosition.xy * input.iScale);
	float4 vWorldPos = float4(
		vPos2D + input.iPosition.xy,
		1.0 - input.iPosition.z,
		1.0);
	float4x4 mWVP = mul(mTrans, mul(mView, mProj));

	output.vPosition = mul(vWorldPos, mWVP);
	output.vColor = input.iColor;
	output.vTexUV = input.iTexUV[input.vVertexID];

	return output;
}

Texture2D tex2D;

SamplerState linearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PShader(float4 vPosition : SV_POSITION, float4 vColor : COLOR0, float2 vCoord : TEXCOORD) : SV_TARGET
{
	float4 texColor = tex2D.Sample(linearSampler, vCoord);
	float4 color = float4(1.0, 1.0, 1.0, texColor.r);
	return color * vColor * vGlobalTint;
}
