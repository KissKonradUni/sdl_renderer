cbuffer UniformBlock : register(b0, space3)
{
    float4x4 CameraTranlation : packoffset(c0);
    float4x4 CameraRotation   : packoffset(c4);
    float4x4 CameraProjection : packoffset(c8);
    float4   CameraPosition   : packoffset(c12);
};

#define MAX_STEPS 300
#define EPSILON 0.001f
#define FAR_PLANE 1000.0f
#define SKY_COLOR float4(0.0f, 0.70f, 1.0f, 1.0f)

float3 calculateRayDirection(float2 UV)
{
    float4 worldDir = float4(UV.x, UV.y * -1.0f, 1.0f, 1.0f);
    worldDir = mul(mul(CameraRotation, CameraProjection), worldDir);
    worldDir *= (1 / worldDir.w);
    float3 result = normalize(worldDir.xyz);
    return worldDir.xyz;
}

struct sdfResult {
    float d;
    float4 color;
};

sdfResult newSdfResult(float d, float4 color)
{
    sdfResult result;
    result.d = d;
    result.color = color;
    return result;
}

sdfResult minSdf(sdfResult a, sdfResult b)
{
    if (a.d < b.d) { return a; }
    return b;
}

sdfResult maxSdf(sdfResult a, sdfResult b)
{
    if (a.d > b.d) { return a; }
    return b;
}

sdfResult negSdf(sdfResult a)
{
    a.d *= -1.0f;
    return a;
}

sdfResult rectangleSDF(float3 rayPos, float3 rectCenter, float3 rectSize, float4 color)
{
    float3 d = abs(rayPos - rectCenter) - rectSize;
    float dist = min(max(d.x, max(d.y, d.z)), 0.0f) + length(max(d, 0.0f));
    return newSdfResult(dist, color);
}

sdfResult cylinderSDF(float3 rayPos, float3 cylinderCenter, float cylinderRadius, float cylinderHeight, float4 color)
{
    float3 p = rayPos - cylinderCenter;
    float2 d = float2(length(float2(p.x, p.z)) - cylinderRadius, abs(p.y) - cylinderHeight);
    float dist = min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f));
    return newSdfResult(dist, color);
}

sdfResult cylinderXSDF(float3 rayPos, float3 cylinderCenter, float cylinderRadius, float cylinderHeight, float4 color)
{
    float3 p = rayPos - cylinderCenter;
    float2 d = float2(length(float2(p.y, p.z)) - cylinderRadius, abs(p.x) - cylinderHeight);
    float dist = min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f));
    return newSdfResult(dist, color);
}

sdfResult sphereSDF(float3 rayPos, float3 sphereCenter, float sphereRadius, float4 color)
{
    float dist = length(rayPos - sphereCenter) - sphereRadius;
    return newSdfResult(dist, color);
}

sdfResult sceneSDF(float3 rayPos)
{
    // Curve ray pos upwards
    // float dist = length(rayPos.xz) - 1.0f;
    // float curve = rayPos.y + 0.00125f * pow(dist, 2.0f);
    // rayPos.y = curve;

    sdfResult combined = newSdfResult(FAR_PLANE, SKY_COLOR);

    // White
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    sdfResult sphere_01 = sphereSDF(rayPos, float3( 0.0f, 0.00f, 3.0f), 1.0f, color);
    if (sphere_01.d < combined.d) { combined = sphere_01; }
    sdfResult sphere_02 = sphereSDF(rayPos, float3( 0.0f, 1.25f, 3.0f), 0.75f, color);
    if (sphere_02.d < combined.d) { combined = sphere_02; }
    sdfResult sphere_03 = sphereSDF(rayPos, float3( 0.0f, 2.25f, 3.0f), 0.50f, color);
    if (sphere_03.d < combined.d) { combined = sphere_03; }

    // Gray
    color = float4(0.2f, 0.2f, 0.2f, 1.0f);

    sdfResult sphere_04 = sphereSDF(rayPos, float3( 0.2f, 2.35f, 2.55f), 0.05f, color);
    if (sphere_04.d < combined.d) { combined = sphere_04; }
    sdfResult sphere_05 = sphereSDF(rayPos, float3(-0.2f, 2.35f, 2.55f), 0.05f, color);
    if (sphere_05.d < combined.d) { combined = sphere_05; }

    float plane = rayPos.y + 1.0f;
    color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    if (plane < combined.d) { combined = newSdfResult(plane, color); }

    float backwallDist = rayPos.z * -1.0f + 2.0f;
    color = float4(1.0f, 1.0f, 0.0f, 1.0f);
    sdfResult backwall = newSdfResult(backwallDist, color);

    float3 repeatPos = rayPos;
    repeatPos.x = fmod(abs(repeatPos.x) + 3.5f, 7.0f) - 3.5f;
    sdfResult hole = cylinderSDF(repeatPos, float3(0.0f, 0.0f, 2.0f), 3.0f, 3.0f, color);
    sdfResult holeSphere = sphereSDF(repeatPos, float3(0.0f, 3.0f, 2.0f), 3.0f, color);
    backwall = maxSdf(maxSdf(backwall, negSdf(hole)), negSdf(holeSphere));
    combined = minSdf(combined, backwall);

    sdfResult roof = newSdfResult(rayPos.y * -1.0f + 12.0f, color);
    sdfResult roofEnd = newSdfResult(rayPos.z, color);
    sdfResult roofHole = cylinderXSDF(rayPos, float3(0.0f, 12.0f, 2.0f), 2.0f, 1000.0f, color);
    combined = minSdf(combined, minSdf(maxSdf(roof, negSdf(roofEnd)), roofHole));

    color = float4(0.0f, 0.0f, 1.0f, 1.0f);
    sdfResult randomPillar = rectangleSDF(rayPos, float3( -10.0f, 5.0f, -20.0f), float3(1.0f, 10.0f, 1.0f), color);
    combined = minSdf(combined, randomPillar);

    return combined;
}

float3 computeNormal(float3 p) {
    float epsilon = 0.001;
    return normalize(float3(
        sceneSDF(p + float3(epsilon, 0, 0)).d - sceneSDF(p - float3(epsilon, 0, 0)).d,
        sceneSDF(p + float3(0, epsilon, 0)).d - sceneSDF(p - float3(0, epsilon, 0)).d,
        sceneSDF(p + float3(0, 0, epsilon)).d - sceneSDF(p - float3(0, 0, epsilon)).d
    ));
}

struct hitResult {
    float t;
    int steps;
    float4 color;
};

hitResult rayMarch(float3 rayOrigin, float3 rayDirection)
{
    hitResult result;

    float t = 0.0f;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        float3 pos = rayOrigin + rayDirection * t;
        sdfResult res = sceneSDF(pos);
        float dist = res.d;
        if (dist < EPSILON) { 
            result.t = t;
            result.steps = i;
            result.color = res.color;
            return result;
        }
        t += dist;
        if (t >= FAR_PLANE) { break; }
    }
    
    result.t = -1.0f;
    result.steps = MAX_STEPS;
    result.color = SKY_COLOR;
    return result;
}

float softShadow(float3 pos, float3 lightDir, float k)
{
    float result = 1.0f;
    float t = 0.0f;
    
    for(int i = 0; i < MAX_STEPS; i++)
    {
        float3 p = pos + lightDir * t;
        float dist = sceneSDF(p).d;
        
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
    float4 result = SKY_COLOR;

    float3 lightDir = normalize(float3(-1.0f, 1.0f, -0.5f));

    float3 rayOrigin = CameraPosition.xyz;
    float3 rayDirection = normalize(calculateRayDirection(UV));

    float3 halfwayDir = normalize(rayDirection + lightDir);

    hitResult hit = rayMarch(rayOrigin, rayDirection);
    float t = hit.t;
    float3 pos = rayOrigin + rayDirection * t;
    
    bool skyHit = false;
    float3 normal;
    if (t > 0.0f) { normal = computeNormal(pos); }
    else { skyHit = true; }

    float shadowHit = softShadow(pos + normal * EPSILON * 5.0f, lightDir, 64.0f);

    if (t > 0.0f) { 
        float diffuse  = max(0.0f, dot(normal, lightDir));
        float specular = pow(max(0.0f, dot(normal, halfwayDir)), 32.0f);
        float4 color = hit.color;
        
        diffuse  *= shadowHit;
        specular *= shadowHit;

        float value = lerp(0.25f, 1.0f, diffuse + specular);
        if (skyHit) { value = 1.0f; }
        result = float4(value, value, value, value) * color;
    }

    // Fog
    float rayLength = length(rayDirection * t);
    float fogAmount = clamp(1.0f - exp(-rayLength * 0.03f) * 1.5f, 0.0f, 1.0f);
    result = float4(lerp(result.xyz, SKY_COLOR, fogAmount), 1.0f);

    return float4(result);
}