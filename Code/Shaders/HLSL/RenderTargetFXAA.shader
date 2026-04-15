// RenderTargetFXAA.shader

cbuffer VSPSSettingsBuffer : register(b1)
{
	float4 vCameraPos;
	float4 vViewDir;
	float4 vGlobalTint;
	float2 vScreenSize;
};

struct VSInputTx
{
	float3 vPosition : SV_Position;
	float2 vTexUV    : TEXCOORD;
};

struct VSOutputTx
{
	float4 vPosition : SV_Position;
	float2 vTexUV    : TEXCOORD;
};

struct PSInputTx
{
	float4 vPosition : SV_Position;
	float2 vTexUV    : TEXCOORD;
};

VSOutputTx VShader(VSInputTx input)
{
	VSOutputTx output;

	output.vPosition = float4(input.vPosition.xyz, 1.0);
	output.vTexUV = input.vTexUV;

	return output;
}

static const float kSubpixelThreshold = 0.125;
static const float kEdgeThresholdMin = 0.0312;
static const float kEdgeThreshold = 0.125;

Texture2D<float4> SceneColor : register(t0);
sampler PointSampler : register(s2);

float4 PShader(PSInputTx input) : SV_Target0
{
    float2 uv = input.vTexUV;
    float2 ps = float2(1.0 / vScreenSize.x, 1.0 / vScreenSize.y);

    // Sample 9-tap neighborhood
    float4 center = SceneColor.SampleLevel(PointSampler, uv, 0);
    float3 rgbNW = SceneColor.SampleLevel(PointSampler, uv + float2(-1.0, -1.0) * ps, 0).rgb;
    float3 rgbNE = SceneColor.SampleLevel(PointSampler, uv + float2( 1.0, -1.0) * ps, 0).rgb;
    float3 rgbSW = SceneColor.SampleLevel(PointSampler, uv + float2(-1.0,  1.0) * ps, 0).rgb;
    float3 rgbSE = SceneColor.SampleLevel(PointSampler, uv + float2( 1.0,  1.0) * ps, 0).rgb;
    float3 rgbN  = SceneColor.SampleLevel(PointSampler, uv + float2( 0.0, -1.0) * ps, 0).rgb;
    float3 rgbS  = SceneColor.SampleLevel(PointSampler, uv + float2( 0.0,  1.0) * ps, 0).rgb;
    float3 rgbE  = SceneColor.SampleLevel(PointSampler, uv + float2( 1.0,  0.0) * ps, 0).rgb;
    float3 rgbW  = SceneColor.SampleLevel(PointSampler, uv + float2(-1.0,  0.0) * ps, 0).rgb;

    // Convert to luminance (human perception weights)
    float3 luma = float3(0.299, 0.587, 0.114);
    float lNW = dot(rgbNW, luma);
    float lNE = dot(rgbNE, luma);
    float lSW = dot(rgbSW, luma);
    float lSE = dot(rgbSE, luma);
    float lN  = dot(rgbN,  luma);
    float lS  = dot(rgbS,  luma);
    float lE  = dot(rgbE,  luma);
    float lW  = dot(rgbW,  luma);
    float lC  = dot(center.rgb, luma);

    // Edge detection & contrast check
    float lMin = min(min(min(lN, lE), lS), lW);
    float lMax = max(max(max(lN, lE), lS), lW);
    float range = lMax - lMin;

    // Early exit: flat area or contrast too low
    if (range < max(kEdgeThresholdMin, lMax * kEdgeThreshold))
	{
		return float4(center.rgb, 1.0);
    }

    // Compute gradient direction
    float2 dir;
    dir.x = -(lNW + lNE) - (lW + lE) + (lSW + lSE);
    dir.y =  (lNW + lSW) + (lN + lS) - (lNE + lSE);
    dir = normalize(dir);

    // Search along gradient to find edge span & blend factor
    float2 dirPair = dir * ps;

    // Step forward in gradient direction
    float2 uv1 = uv - dirPair;
    float2 uv2 = uv + dirPair;

    float3 rgb1 = SceneColor.SampleLevel(PointSampler, uv1, 0).rgb;
    float3 rgb2 = SceneColor.SampleLevel(PointSampler, uv2, 0).rgb;
    float l1 = dot(rgb1, luma);
    float l2 = dot(rgb2, luma);
    float lDelta1 = abs(l1 - lC);
    float lDelta2 = abs(l2 - lC);

    // Blend factor based on luminance difference along edge
    float blend = saturate( (lDelta1 + lDelta2) / (2.0 * range + 0.0001) );
    blend = saturate( blend - kSubpixelThreshold );
    blend = blend * blend * (3.0 - 2.0 * blend); // Smoothstep

    // Apply directional blur blend
    float3 finalColor = center.rgb * (1.0 - blend) + ((rgb1 + rgb2) * 0.5) * blend;
    return float4(finalColor, 1.0);
}
