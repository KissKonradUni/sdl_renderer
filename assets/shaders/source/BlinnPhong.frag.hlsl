cbuffer CameraData : register(b0, space3)
{
    float4 CameraPosition : packoffset(c0);
    float4 CameraViewDir  : packoffset(c1);
};

cbuffer MaterialData : register(b1, space3)
{
    float4 AlbedoColor : packoffset(c0);
    float  Shininess   : packoffset(c4);
};

#define LIGHT_POS float3(0.0f, 10.0f, 10.0f)

float4 renderingEquation(float4 pos, float4 normal, float2 uv) {
    float3 cameraPos = CameraPosition.xyz;
    float3 cameraDir = CameraViewDir.xyz;

    float3 lightDir = normalize(LIGHT_POS - pos.xyz);
    float3 halfDir = normalize(lightDir + cameraDir);

    float3 albedo = AlbedoColor.xyz;
    float3 diffuse = albedo * max(0.0f, dot(normal.xyz, lightDir));
    float3 specular = albedo * pow(max(0.0f, dot(normal.xyz, halfDir)), Shininess);

    return float4(diffuse + specular, 1.0f);
}

float4 main(float4 Position : SV_Position, float4 Normal : NORMAL, float2 UV: TEXCOORD0) : SV_Target0
{
    return renderingEquation(Position, Normal, UV);
}