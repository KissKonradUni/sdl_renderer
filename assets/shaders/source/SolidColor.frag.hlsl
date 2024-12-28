cbuffer UniformBlock : register(b0, space3)
{
    float4x4 CameraTranlation : packoffset(c0);
    float4x4 CameraRotation   : packoffset(c4);
    float4x4 CameraProjection : packoffset(c8);
    float4   CameraPosition   : packoffset(c12);
};

#define MAX_STEPS 1000
#define EPSILON 0.001f
#define FAR_PLANE 1000.0f

float3 calculateRayDirection(float2 UV)
{
    float4 worldDir = float4(UV.x * -1.0f, UV.y * -1.0f, 1.0f, 1.0f);
    worldDir = mul(mul(CameraRotation, CameraProjection), worldDir);
    worldDir *= (1 / worldDir.w);
    float3 result = normalize(worldDir.xyz);
    return worldDir.xyz;
}

float rectangleSDF(float3 rayPos, float3 rectCenter, float3 rectSize)
{
    float3 d = abs(rayPos - rectCenter) - rectSize;
    return min(max(d.x, max(d.y, d.z)), 0.0f) + length(max(d, 0.0f));
}

float sphereSDF(float3 rayPos, float3 sphereCenter, float sphereRadius)
{
    return length(rayPos - sphereCenter) - sphereRadius;
}

float sceneSDF(float3 rayPos)
{
    // Curve ray pos upwards
    // float dist = length(rayPos.xz) - 1.0f;
    // float curve = rayPos.y + 0.00125f * pow(dist, 2.0f);
    // rayPos.y = curve;

    float combined = FAR_PLANE;

    float sphere_01 = sphereSDF(rayPos, float3( 0.0f, 0.00f, 3.0f), 1.0f);
    combined = min(combined, sphere_01);
    float sphere_02 = sphereSDF(rayPos, float3( 0.0f, 1.25f, 3.0f), 0.75f);
    combined = min(combined, sphere_02);
    float sphere_03 = sphereSDF(rayPos, float3( 0.0f, 2.25f, 3.0f), 0.50f);
    combined = min(combined, sphere_03);

    float sphere_04 = sphereSDF(rayPos, float3( 0.2f, 2.35f, 2.55f), 0.05f);
    combined = min(combined, sphere_04);
    float sphere_05 = sphereSDF(rayPos, float3(-0.2f, 2.35f, 2.55f), 0.05f);
    combined = min(combined, sphere_05);

    float plane = rayPos.y + 1.0f;
    combined = min(combined, plane);

    float backwall = rayPos.z * -1.0f + 2.0f;
    float hole = rectangleSDF(rayPos, float3(0.0f, 1.0f, 3.0f), float3(3.0f, 3.0f, 2.0f));
    combined = min(combined, max(backwall, -hole));

    return combined;
}

float3 computeNormal(float3 p) {
    float epsilon = 0.001;
    return normalize(float3(
        sceneSDF(p + float3(epsilon, 0, 0)) - sceneSDF(p - float3(epsilon, 0, 0)),
        sceneSDF(p + float3(0, epsilon, 0)) - sceneSDF(p - float3(0, epsilon, 0)),
        sceneSDF(p + float3(0, 0, epsilon)) - sceneSDF(p - float3(0, 0, epsilon))
    ));
}

struct hitResult {
    float t;
    float smallestDist;
    int steps;
};

hitResult rayMarch(float3 rayOrigin, float3 rayDirection)
{
    hitResult result;

    float t = 0.0f;
    float smallestDist = 100.0f;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        float3 pos = rayOrigin + rayDirection * t;
        float dist = sceneSDF(pos);
        if (dist < EPSILON) { 
            result.t = t;
            result.smallestDist = smallestDist;
            result.steps = i;
            return result;
        }
        if (dist < smallestDist) { smallestDist = dist; }
        t += dist;
        if (t >= FAR_PLANE) { break; }
    }
    
    result.t = -1.0f;
    result.smallestDist = smallestDist;
    result.steps = MAX_STEPS;
    return result;
}

float4 main(float2 UV: TEXCOORD0) : SV_Target0
{
    float3 result = float3(0.0f, 0.0f, 0.0f);

    float3 lightDir = normalize(float3(-1.0f, 1.0f, -0.5f));

    float3 rayOrigin = CameraPosition;
    float3 rayDirection = normalize(calculateRayDirection(UV));

    hitResult hit = rayMarch(rayOrigin, rayDirection);
    float t = hit.t;
    float3 pos = rayOrigin + rayDirection * t;
    float3 normal = computeNormal(pos);

    hitResult shadowHit = rayMarch(pos + normal * EPSILON, lightDir);

    if (t > 0.0f) { 
        float diffuse = max(0.0f, dot(normal, lightDir));
        
        if (shadowHit.t > 0.0f) {
            diffuse *= 0.01f;
        }

        result = float4(diffuse, diffuse, diffuse, 1.0f);
    }

    // Fog
    float rayLength = length(rayDirection * t);
    float fogAmount = 1.0f - exp(-rayLength * 0.08f);
    result = lerp(result, float3(0.0f, 0.0f, 0.0f), fogAmount);
    
    // Debug override
    // hit debug
    //float col = result;
    //if (hit.steps == MAX_STEPS) {
    //    col = shadowHit.smallestDist;
    //}
    //result = float3(col, col, col);

    return float4(result, 1.0f);
}