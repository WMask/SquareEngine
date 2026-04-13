// Text2d.shader

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
};

struct VSInputTxInst
{
    uint   vVertexID : SV_VertexID;
	float3 vPosition : SV_Position;
	float3 iPosition : INSTANCEPOS;
	float2 iScale    : INSTANCESCALE;
	float4 iColor    : INSTANCECOLOR;
	float2 iTexUV[4] : INSTANCEUV;
};

struct VSOutputTxClr
{
	float4 vPosition : SV_Position;
	float4 vColor    : COLOR;
	float2 vTexUV    : TEXCOORD;
};

struct PSInputTxClr
{
	float4 vPosition : SV_Position;
	float4 vColor    : COLOR;
	float2 vTexUV    : TEXCOORD;
};

VSOutputTxClr VShader(VSInputTxInst input)
{
	VSOutputTxClr output;

	float2 vPos2D = (input.vPosition.xy * input.iScale);
	float4 vWorldPos = float4(
		vPos2D + input.iPosition.xy,
		1.0 - input.iPosition.z, 1.0);
	float4x4 mWVP = mul(mWorld, mul(mView, mProj));

	output.vPosition = mul(vWorldPos, mWVP);
	output.vColor = input.iColor;
	output.vTexUV = input.iTexUV[input.vVertexID];

	return output;
}

Texture2D<float4> AlbedoTexture : register(t0);
sampler SurfaceSampler : register(s0);

float4 PShader(PSInputTxClr input) : SV_TARGET
{
	float4 vTexColor = AlbedoTexture.Sample(SurfaceSampler, input.vTexUV);
	float4 vGlyphMask = float4(1.0, 1.0, 1.0, vTexColor.r);
	return vGlyphMask * input.vColor * vGlobalTint;
}
