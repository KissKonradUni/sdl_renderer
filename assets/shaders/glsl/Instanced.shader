{
    "name": "Instanced PBR Shader",
    "vert": "./assets/shaders/glsl/Instanced.vert.glsl",
    "frag": "./assets/shaders/glsl/Instanced.frag.glsl",
    "layout": {
        "vertex": [
            { "name": "position", "type": "vec3" },
            { "name": "normal", "type": "vec3" },
            { "name": "tangent", "type": "vec3" },
            { "name": "uv", "type": "vec2" }
        ],
        "fragment": [
            { "name": "outputColor", "type": "vec4" }
        ]
    },
    "textures": [
        "textureDiffuse",
        "textureNormal",
        "textureAORoughnessMetallic"
    ],
    "requires": [
        "cameraBuffer",
        "modelMatrix"
    ]
}