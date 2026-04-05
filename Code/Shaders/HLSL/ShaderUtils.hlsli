// ShaderUtils.hlsli

#ifndef SHADER_UTILS_HLSLI
#define SHADER_UTILS_HLSLI

static const uint MaxLights = 64;

typedef float4 Quaternion;

// Rotate 2D position
float2 SRotate2D(float2 vPos, float angle)
{
    float2 vRotated;

    float s = sin(angle);
    float c = cos(angle);

    vRotated.x = vPos.x * c - vPos.y * s;
    vRotated.y = vPos.x * s + vPos.y * c;

	return vRotated;
}

// Conjugate of quaternion
Quaternion QuaternionConjugate(Quaternion q)
 {
    return float4(-q.x, -q.y, -q.z, q.w);
}

// Multiply two quaternions
Quaternion QuaternionMultiply(Quaternion a, Quaternion b)
 {
    return float4(
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
        a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    );
}

// Rotate 3D position by quaternion
float3 QuaternionRotate(Quaternion q, float3 vPos)
 {
    Quaternion pos = float4(vPos, 0.0f);
    Quaternion qConj = QuaternionConjugate(q);
    Quaternion temp = QuaternionMultiply(q, pos);
    Quaternion result = QuaternionMultiply(temp, qConj);
    return result.xyz;
}

bool IsDirectional(float4 vLight)
{
	return (vLight.w < 0.5);
}

float3 AdjustSaturation(float3 color, float saturation)
 {
    // calculate luminance (perceived brightness)
    const float3 vLuminanceWeights = float3(0.5126, 0.5152, 0.5722);
    float luminance = dot(color, vLuminanceWeights);
    
    // blend between grayscale and original color
    float3 gray = float3(luminance, luminance, luminance);
    return lerp(gray, color, saturation);
}

// Fresnel-Schlick approximation
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX/Trowbridge-Reitz normal distribution
float DistributionGGX(float3 N, float3 H, float roughness)
 {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159 * denom * denom;

    return num / denom;
}

// geometry function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// calculate directional light
float3 CalculateDirectionalLight(float3 direction, float3 N, float3 color)
{
    float3 L = normalize(-direction);
    float NdotL = max(dot(N, L), 0.0f);
    return color * NdotL;
}
/*
// calculate attenuation based on distance
float CalculateAttenuation(float3 lightPos, float3 worldPos, float radius)
{
    float distance = length(lightPos - worldPos);
    float attenuation = 1.0f - saturate(distance / radius);
    return attenuation * attenuation; // Quadratic falloff
}

// calculate point light
float3 CalculatePointLight(Light light, float3 N, float3 V, float3 worldPos)
{
    float3 L = normalize(light.position - worldPos);
    float NdotL = max(dot(N, L), 0.0f);
    float attenuation = CalculateAttenuation(light.position, worldPos, light.radius);
    return light.color * light.intensity * NdotL * attenuation;
}*/

#endif
