
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
	float3 vPosition : POSITION;
	uint   vIndex    : INSTANCEID;
	float3 iPosition : INSTANCEPOS;
	float  iRotation : INSTANCEROT;
	float2 iScale    : INSTANCESCALE;
	float4 iColor[4] : INSTANCECOLOR;
};

struct VS_OUT
{
	float4 vPosition : SV_POSITION;
	float4 vColor    : COLOR0;
	float4 vTint     : COLOR1;
};

float2 SRotate2D(float2 vPos, float angle)
{
    float2 vRotated;

    float s = sin(angle);
    float c = cos(angle);

    vRotated.x = vPos.x * c - vPos.y * s;
    vRotated.y = vPos.x * s + vPos.y * c;

	return vRotated;
}

VS_OUT VShader(VS_INPUT input)
{
	VS_OUT output;

	float2 vPos2D = (input.vPosition.xy * input.iScale);
	float2 vRotatedPos2D = SRotate2D(vPos2D, input.iRotation);

	float4 vWorldPos = float4(vRotatedPos2D + input.iPosition.xy, input.vPosition.z, 1.0);
	float4x4 mWVP = mul(mTrans, mul(mView, mProj));

	output.vPosition = mul(vWorldPos, mWVP);
	output.vColor = input.iColor[input.vIndex];
	output.vTint = vGlobalTint;

	return output;
}

float4 PShader(float4 vPosition : SV_POSITION, float4 vColor : COLOR0, float4 vTint : COLOR1) : SV_TARGET
{
	return vColor * vTint;
}
