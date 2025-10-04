#version 460 core
precision highp float;

in vec2 fragmentUV;

out vec4 outputColor;

uniform sampler2D gDiffuse;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform sampler2D gAORoughnessMetallic;

in vec3 cameraPosition;
in vec3 cameraDirection;
in mat4 cameraMatrix;

uniform sampler2D shadowMap;
uniform mat4 lightViewMatrix;
uniform mat4 lightProjectionMatrix;
uniform vec4 lightDirection;

uniform sampler2D skyboxTexture;

#define PI 3.14159265359
#define HALF_PI 1.57079632679

struct brdfInformation {
    vec3 diffuseColor;
    float roughness;
    float metallic;

    vec3 lightColor;
    vec3 reflectionColor;
} brdfInfo;

vec3 rgbToOklab(vec3 c) {
    // Correct Linear RGB to OKLCh intermediate space
    const mat3 M1 = mat3(
        0.4122214708, 0.2119034982, 0.0883024619,
        0.5363325363, 0.6806995451, 0.2817188376,
        0.0514459929, 0.1073969566, 0.6299787005
    );
    vec3 lms = M1 * c;
    
    // Safe cube root with sign preservation
    lms = sign(lms) * pow(abs(lms), vec3(1.0/3.0));

    // LMS to Oklab
    const mat3 M2 = mat3(
        0.2104542553,  1.9779984951,  0.0259040371,
        0.7936177850, -2.4285922050,  0.7827717662,
       -0.0040720468,  0.4505937099, -0.8086757660
    );
    return M2 * lms;
}

vec3 oklabToRgb(vec3 lab) {
    // Oklab to LMS
    const mat3 M1 = mat3(
        1.0,  1.0,  1.0,
        0.3963377774, -0.1055613458, -0.0894841775,
        0.2158037573, -0.0638541728, -1.2914855480
    );
    vec3 lms = M1 * lab;
    
    // Cube the values with sign preservation
    lms = lms * lms * lms;

    // LMS to Linear RGB
    const mat3 M2 = mat3(
         4.0767416621, -1.2684380046, -0.0041960863,
        -3.3077115913,  2.6097574011, -0.7034186147,
         0.2309699292, -0.3413193965,  1.7076147010
    );
    return M2 * lms;
}

vec2 sampleEquirectangularMap(vec3 dir)
{
    float u = 0.5 + (atan(dir.z, dir.x) / (2.0 * 3.14159265359));
    float v = 0.5 - (asin(dir.y) / 3.14159265359);
    return vec2(u, 1.0 - v);
}

float checkSkylightShadow(vec3 fragPos, vec3 normal)
{
    vec4 lightSpacePos = vec4(fragPos, 1.0) * lightViewMatrix * lightProjectionMatrix;
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // If outside shadow map, return no shadow
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float bias = 0.0025;
    float shadow = 0.0;
    float samples = 0.0;
    float texelSize = 1.0 / textureSize(shadowMap, 0).x;

    // 3x3 PCF kernel
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
            samples += 1.0;
        }
    }
    shadow /= samples;
    return shadow;
}

// GGX
float microfacetDistribution(float cosTheta) {
    float alpha = brdfInfo.roughness * brdfInfo.roughness;
    float denom = (cosTheta * cosTheta) * (alpha - 1.0) + 1.0;
    return (alpha) / (PI * denom * denom);
}

// GGX Smith's lambda
float GGXLambda(float cosTheta, float alpha) {
    float a = cosTheta * alpha;
    float a2 = a * a;
    return (-1.0 + sqrt(1.0 + a2)) / 2.0;
}

// Christophe-Schlick Fresnel approximation
float fresnel(float cosTheta, float F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Smith model
float geometricAttenuation(float NdotV, float NdotL, float roughness) {
    float alpha = roughness * roughness;
    float lambdaV = GGXLambda(NdotV, alpha);
    float lambdaL = GGXLambda(NdotL, alpha);
    return 1.0 / (1.0 + lambdaV + lambdaL);
}

// Oren-Nayar diffuse
vec3 orenNayarDiffuse(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness, vec3 albedo) {
    // Convert [0,1] roughness to radians (max ~90deg)
    float sigma = roughness * HALF_PI;
    float sigma2 = sigma * sigma;

    float A = 1.0 - (sigma2 / (2.0 * (sigma2 + 0.3333)));
    float B = 0.45 * sigma2 / (sigma2 + 0.0966);

    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, viewDir), 0.0);

    float alpha = max(NdotL, NdotV);
    float beta = min(NdotL, NdotV);

    // Compute the angle between view and light projected onto the tangent plane
    float thetaI = acos(NdotL);
    float thetaR = acos(NdotV);

    float phiI = atan(lightDir.y, lightDir.x);
    float phiR = atan(viewDir.y, viewDir.x);

    float deltaPhi = phiI - phiR;
    float cosDeltaPhi = cos(deltaPhi);

    float C = sin(alpha) * tan(beta);

    return albedo * NdotL * (A + B * max(0.0, cosDeltaPhi) * C);
}

vec3 diffuseBrdf(vec3 normal, vec3 viewDir, vec3 lightDir) {
    // Adjust albedo for metallic surfaces
    // (More of an artistic choice than physically accurate)
    vec3 albedo = mix(
        brdfInfo.diffuseColor,
        mix(brdfInfo.lightColor, brdfInfo.reflectionColor, brdfInfo.metallic * 0.8 + 0.2),
        brdfInfo.metallic * 0.6 + 0.2
    );
    return orenNayarDiffuse(normal, viewDir, lightDir, brdfInfo.roughness, albedo);
}

// Microfacet theory based specular
vec3 specularBrdf(vec3 incomingDir, vec3 outgoingDir, vec3 normal) {
    float cosTheta = max(dot(normal, (incomingDir + outgoingDir) * 0.5), 0.0);
    
    float D = microfacetDistribution(cosTheta);
    // Assuming non-metallic surface with F0 = 0.04
    float F = fresnel(cosTheta, 0.04);
    // Assuming we use GGX
    float NdotV = max(dot(normal, outgoingDir), 0.0);
    float NdotL = max(dot(normal, incomingDir), 0.0);
    float G = geometricAttenuation(NdotV, NdotL, brdfInfo.roughness);

    // Industry standard denominator
    float denominator = max(4.0 * NdotV * NdotL, 0.001);

    vec3 materialBaseColor = mix(brdfInfo.diffuseColor, brdfInfo.reflectionColor, brdfInfo.metallic * 0.5);

    return vec3((D * F * G) / denominator) * materialBaseColor;
}

vec3 brdf(vec3 incomingDir, vec3 outgoingDir, vec3 normal) {
    return diffuseBrdf(normal, outgoingDir, incomingDir) + specularBrdf(incomingDir, outgoingDir, normal);
}

vec3 applyToneMapping(vec3 color) {
    // Convert to Oklab
    vec3 lab = rgbToOklab(color);
    
    // Convert to Oklch
    float C = sqrt(lab.y * lab.y + lab.z * lab.z);
    float h = atan(lab.z, lab.y);

    // Apply tone mapping to L channel
    lab.x = 1.0 - exp(-lab.x * 2.0);
    // Increase saturation and compress chroma
    C = pow(C, 0.9) * 1.1;
    // Wrap hue to [0, 2PI]
    h = mod(h, 2.0 * PI);

    // Convert back to Oklab
    lab.y = C * cos(h);
    lab.z = C * sin(h);

    // Convert back to RGB
    vec3 result = oklabToRgb(lab);
    return clamp(result, 0.0, 1.0);
}

void main()
{
    float gamma = 2.2;

    vec3 normal = normalize(texture(gNormal, fragmentUV).rgb * 2.0 - 1.0);
    vec3 position = texture(gPosition, fragmentUV).rgb;
    vec4 aoRoughnessMetallic = texture(gAORoughnessMetallic, fragmentUV);

    brdfInfo.diffuseColor = pow(texture(gDiffuse, fragmentUV).rgb, vec3(gamma));
    brdfInfo.roughness    = 1.0 - aoRoughnessMetallic.g;
    brdfInfo.metallic     = aoRoughnessMetallic.b;
    float ao              = aoRoughnessMetallic.r;

    // If the fragment has no geometry, show the skybox
    if (length(position) <= 0.001)
    {
        outputColor = vec4(brdfInfo.diffuseColor, 1.0);
        outputColor.rgb = applyToneMapping(outputColor.rgb);
        outputColor.rgb = clamp(outputColor.rgb, 0.0, 1.0);
        outputColor.rgb = pow(outputColor.rgb, vec3(1.0 / gamma));
        return;
    }

    // Base lighting calculations
    vec3 viewDir = normalize(cameraPosition - position);
    vec3 lightDir = normalize(lightDirection.xyz * vec3(-1.0, 1.0, -1.0));
    float cosPhi = max(dot(normal, lightDir), 0.0);

    // Diffuse lighting from skybox
    vec2 diffuseSkyUV = sampleEquirectangularMap(normal);
    brdfInfo.lightColor = pow(texture(skyboxTexture, diffuseSkyUV).rgb, vec3(gamma));

    // Specular lighting from skybox
    vec2 specularSkyUV = sampleEquirectangularMap(reflect(-viewDir, normal));
    brdfInfo.reflectionColor = pow(texture(skyboxTexture, specularSkyUV).rgb, vec3(gamma));

    // Shadow calculation
    float shadow = checkSkylightShadow(position, normal);

    // Combine results
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    outputColor = vec4(
        lightColor *
        brdf(lightDir, viewDir, normal) *
        cosPhi *
        ao *
        (1.0 - shadow * 0.95 + 0.05)
    , 1.0);

    // Gamma correction
    outputColor.rgb = applyToneMapping(outputColor.rgb);

    outputColor.rgb = clamp(outputColor.rgb, 0.0, 1.0);
    outputColor.rgb = pow(outputColor.rgb, vec3(1.0 / gamma));
}
