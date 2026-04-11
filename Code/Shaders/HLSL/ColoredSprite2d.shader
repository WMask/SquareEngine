// ColoredSprite2d.shader

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
	float4 vGlobalTint;
	float4 vCameraPos;
	float4 vViewDir;
};

struct VSInputTxInst
{
    uint   vVertexID : SV_VertexID;
	float3 vPosition : SV_Position;
	float3 iPosition : INSTANCEPOS;
	float  iRotation : INSTANCEROT;
	float2 iScale    : INSTANCESCALE;
	float4 iColor[4] : INSTANCECOLOR;
};

struct VSOutputClr
{
	float4 vPosition : SV_Position;
	float4 vColor    : COLOR;
};

struct PSInputClr
{
	float4 vPosition : SV_Position;
	float4 vColor    : COLOR;
};

VSOutputClr VShader(VSInputTxInst input)
{
	VSOutputClr output;

	float2 vPos2D = (input.vPosition.xy * input.iScale);
	float2 vRotatedPos2D = SRotate2D(vPos2D, input.iRotation);

	float4 vWorldPos = float4(
		vRotatedPos2D + input.iPosition.xy,
		1.0 - input.iPosition.z,
		1.0);
	float4x4 mWVP = mul(mWorld, mul(mView, mProj));

	output.vPosition = mul(vWorldPos, mWVP);
	output.vColor = input.iColor[input.vVertexID];

	return output;
}

float4 PShader(PSInputClr input) : SV_Target0
{
	return input.vColor * vGlobalTint;
}
