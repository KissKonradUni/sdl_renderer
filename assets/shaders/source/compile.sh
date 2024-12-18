# Requires shadercross CLI installed from SDL_shadercross
for filename in *.vert.hlsl; do
    if [ -f "$filename" ]; then
        glslc -fshader-stage=vert -o "../compiled/SPIRV/${filename/.hlsl/.spv}" "$filename"
    fi
done

for filename in *.frag.hlsl; do
    if [ -f "$filename" ]; then
        glslc -fshader-stage=frag -o "../compiled/SPIRV/${filename/.hlsl/.spv}" "$filename"
    fi
done

for filename in *.comp.hlsl; do
    if [ -f "$filename" ]; then
        glslc -fshader-stage=comp -o "../compiled/SPIRV/${filename/.hlsl/.spv}" "$filename"
    fi
done