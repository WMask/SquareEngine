
cbuffer VS_PROJ_BUFFER : register(b0)
{
	float4x4 mProj;
};

cbuffer VS_VIEW_BUFFER : register(b1)
{
	float4x4 mView;
};

cbuffer VS_TRANS_BUFFER : register(b2)
{
	float4x4 mTrans;
};

cbuffer VS_SETTINGS_BUFFER : register(b6)
{
	float4 vGlobalTint;
};

struct VS_INPUT
{
	float3 vPosition : POSITION;
	uint   vIndex    : INSTANCEID;
	float3 iPosition : INSTANCEPOS;
	float4 iColor[4] : COLOR;
};

struct VS_OUT
{
	float4 vPosition : SV_POSITION;
	float4 vColor : COLOR0;
	float4 vTint : COLOR1;
};

VS_OUT VShader(VS_INPUT input)
{
	VS_OUT output;

	float4x4 mWVP = mul(mTrans, mul(mView, mProj));
	float4 vWorldPos = float4(input.vPosition + input.iPosition, 1.0);

	output.vPosition = mul(vWorldPos, mWVP);
	output.vColor = input.iColor[input.vIndex];
	output.vTint = vGlobalTint;

	return output;
}

float4 PShader(float4 vPosition : SV_POSITION, float4 vColor : COLOR0, float4 vTint : COLOR1) : SV_TARGET
{
	return vColor * vTint;
}
