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
	float4 vGlobalTint;
	float4 vCameraPos;
	float4 vViewDir;
	uint   bHasDiffuseCubemap;  // diffuse ambient light (radiance)
	uint   bHasSpecularCubemap; // specular reflection (irradiance)
	float  diffuseAmount;
	float  specularAmount;
};

cbuffer VSPSMaterialBuffer : register(b2)
{
	uint bHasBaseTexture;
	uint bHasNormalTexture;
	uint bHasRMATexture;
	uint bHasEmissiveTexture;
};

cbuffer PSLightsBuffer : register(b3)
{
	float4 vLightVec[MaxLights];   // xyz - direction for directional, position for point, w - light type: <0.5 - directional, >0.5 - point
	float4 vLightColor[MaxLights]; // rgb - color, a - distance if point light type
	uint numLights;
};

struct VSOutputNmTx
{
    float3 vPositionWS : POSITION;
    float3 vNormalWS   : NORMAL;
    float2 vTexCoord   : TEXCOORD;
    float4 vPositionPS : SV_Position;
};

struct PSInputNmTx
{
    float3 vPositionWS : POSITION;
    float3 vNormalWS   : NORMAL;
    float2 vTexCoord   : TEXCOORD;
};

struct VSInputNmTxInst
{
    float4 vPosition : SV_Position;
    float3 vNormal   : NORMAL;
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
    output.vPositionWS = mul(vWorldPos4, mWorld).xyz;
    output.vNormalWS = normalize(mul(input.vNormal, mNormal));
    output.vTexCoord = input.vTexCoord;

    return output;
}


Texture2D<float4> AlbedoTexture   : register(t0);
Texture2D<float3> NormalTexture   : register(t1);
Texture2D<float3> RMATexture      : register(t2);
Texture2D<float3> EmissiveTexture : register(t3);

TextureCube<float3> RadianceTexture   : register(t4); // diffuse ambient light (radiance)
TextureCube<float3> IrradianceTexture : register(t5); // specular reflection (irradiance)

sampler SurfaceSampler : register(s0);
sampler IBLSampler     : register(s1);


// Apply Disney-style physically based rendering to a surface with:
float3 LightSurface(
    in float3 V, in float3 N, in float3 albedo,
	in float roughness, in float metallic, in float ambientOcclusion)
{
    // Specular coefficiant - fixed reflectance value for non-metals
    static const float kSpecularCoefficient = 0.04;

    const float NdotV = saturate(dot(N, V));

    // Burley roughness bias
    const float alpha = roughness * roughness;

    // Blend base colors
    const float3 vDiff = lerp(albedo, float3(0, 0, 0), metallic) * ambientOcclusion;
    const float3 vSpec = lerp(kSpecularCoefficient, albedo, metallic) * ambientOcclusion;

    // Output color
    float3 vAccColor = 0;

    // Accumulate light values
    for (uint i = 0; i < numLights; i++)
    {
        // light vector (to light)
        const float3 L = normalize(-vLightVec[i].xyz);

        // Half vector
        const float3 H = normalize(L + V);

        // products
        const float NdotL = saturate(dot(N, L));
        const float LdotH = saturate(dot(L, H));
        const float NdotH = saturate(dot(N, H));

        // Diffuse & specular factors
        float diffuseFactor = DiffuseBurley(NdotL, NdotV, LdotH, roughness);
        float3 vSpecular = SpecularBRDF(alpha, vSpec, NdotV, NdotL, LdotH, NdotH);

        // Directional light
        vAccColor += NdotL * vLightColor[i].rgb * (((vDiff * diffuseFactor) + vSpecular));
    }

	// Add diffuse irradiance
	float3 vDiffuseEnv = IrradianceTexture.Sample(IBLSampler, N);
	vAccColor += vDiff * vDiffuseEnv;

	// Get specular radiance
	float3 vDir = reflect(-V, N);
	float3 vSpecularEnv = RadianceTexture.Sample(IBLSampler, vDir);
	float3 vSpecular = vSpec * vSpecularEnv;

	// Lerp IBL and normal lighting
	const float3 L = normalize(-vLightVec[0].xyz);
	const float NdotL = saturate(dot(N, L));
	vAccColor += lerp(albedo * NdotL, vSpecular, 0.5);

	return vAccColor;
}


float4 PShader(PSInputNmTx input) : SV_Target0
{
    const float3 V = normalize(vCameraPos.xyz - input.vPositionWS);
    const float3 L = normalize(-vLightVec[0].xyz);

	// Get normal
	float3 N = input.vNormalWS;
	if (bHasNormalTexture)
	{
		// Before lighting, peturb the surface's normal by the one given in normal map.
		float3 vLocalNormal = TwoChannelNormalX2(NormalTexture.Sample(SurfaceSampler, input.vTexCoord).xy);
		N = PeturbNormal(vLocalNormal, input.vPositionWS, input.vNormalWS, input.vTexCoord);
	}

    // Get albedo
    float4 albedo = AlbedoTexture.Sample(SurfaceSampler, input.vTexCoord);

    // Get roughness, metalness, and ambient occlusion
    float3 vRMA = RMATexture.Sample(SurfaceSampler, input.vTexCoord);

    // Shade surface
    float3 color = LightSurface(V, N, albedo.rgb, vRMA.g, vRMA.r, vRMA.b);

	if (bHasEmissiveTexture)
	{
		// Add emissive
		color += EmissiveTexture.Sample(SurfaceSampler, input.vTexCoord).rgb;
	}

    return float4(color, albedo.w);
}
