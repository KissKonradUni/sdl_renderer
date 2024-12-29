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
    float4 worldDir = float4(UV.x, UV.y * -1.0f, 1.0f, 1.0f);
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

float cylinderSDF(float3 rayPos, float3 cylinderCenter, float cylinderRadius, float cylinderHeight)
{
    float3 p = rayPos - cylinderCenter;
    float2 d = float2(length(float2(p.x, p.z)) - cylinderRadius, abs(p.y) - cylinderHeight);
    return min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f));
}

float cylinderXSDF(float3 rayPos, float3 cylinderCenter, float cylinderRadius, float cylinderHeight)
{
    float3 p = rayPos - cylinderCenter;
    float2 d = float2(length(float2(p.y, p.z)) - cylinderRadius, abs(p.x) - cylinderHeight);
    return min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f));
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

    float3 repeatPos = rayPos;
    repeatPos.x = fmod(abs(repeatPos.x) + 3.5f, 7.0f) - 3.5f;
    float hole = cylinderSDF(repeatPos, float3(0.0f, 0.0f, 2.0f), 3.0f, 3.0f);
    float holeSphere = sphereSDF(repeatPos, float3(0.0f, 3.0f, 2.0f), 3.0f);
    backwall = max(max(backwall, -hole), -holeSphere);
    combined = min(combined, backwall);

    float roof = rayPos.y * -1.0f + 12.0f;
    float roofEnd = rayPos.z;
    float roofHole = cylinderXSDF(rayPos, float3(0.0f, 12.0f, 2.0f), 2.0f, 1000.0f);
    combined = min(combined, min(max(roof, -roofEnd), roofHole));

    float randomPillar = rectangleSDF(rayPos, float3( -10.0f, 5.0f, -20.0f), float3(1.0f, 10.0f, 1.0f));
    combined = min(combined, randomPillar);

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
        if (dist < smallestDist && dist > 0.1f) { smallestDist = dist; }
        t += dist;
        if (t >= FAR_PLANE) { break; }
    }
    
    result.t = -1.0f;
    result.smallestDist = smallestDist;
    result.steps = MAX_STEPS;
    return result;
}

float softShadow(float3 pos, float3 lightDir, float k)
{
    float result = 1.0f;
    float t = 0.0f;
    
    for(int i = 0; i < MAX_STEPS; i++)
    {
        float3 p = pos + lightDir * t;
        float dist = sceneSDF(p);
        
        if(dist < EPSILON)
            return 0.0f;
            
        if(t > FAR_PLANE)
            break;
            
        // This is the key for soft shadows
        result = min(result, k * dist / t);
        t += dist;
    }
    
    return result;
}

float4 main(float2 UV: TEXCOORD0) : SV_Target0
{
    float3 result = float3(0.0f, 0.0f, 0.0f);

    float3 lightDir = normalize(float3(-1.0f, 1.0f, -0.5f));

    float3 rayOrigin = CameraPosition;
    float3 rayDirection = normalize(calculateRayDirection(UV));

    float3 halfwayDir = normalize(rayDirection + lightDir);

    hitResult hit = rayMarch(rayOrigin, rayDirection);
    float t = hit.t;
    float3 pos = rayOrigin + rayDirection * t;
    float3 normal = computeNormal(pos);

    float shadowHit = softShadow(pos + normal * EPSILON * 5.0f, lightDir, 64.0f);

    if (t > 0.0f) { 
        float diffuse  = max(0.0f, dot(normal, lightDir));
        float specular = pow(max(0.0f, dot(normal, halfwayDir)), 32.0f);
        
        diffuse  *= shadowHit;
        specular *= shadowHit;

        float value = diffuse + specular;
        result = float3(value, value, value);
    }

    // Fog
    float rayLength = length(rayDirection * t);
    float fogAmount = 1.0f - exp(-rayLength * 0.08f);
    result = lerp(result, float3(0.0f, 0.0f, 0.0f), fogAmount);
    
    // Debug override
    // hit debug
    // float3 col = result;
    // col *= shadowHit.smallestDist;
    // result = col;

    return float4(result, 1.0f);
}