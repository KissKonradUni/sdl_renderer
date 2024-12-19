:: Requires shaderc

@echo off

for %%f in (*.vert.hlsl) do (
    if exist "%%f" (
        glslc -fshader-stage=vert -o "../compiled/SPIRV/%%~nf.spv" "%%f"
    )
)

for %%f in (*.frag.hlsl) do (
    if exist "%%f" (
        glslc -fshader-stage=frag -o "../compiled/SPIRV/%%~nf.spv" "%%f"
    )
)

for %%f in (*.comp.hlsl) do (
    if exist "%%f" (
        glslc -fshader-stage=comp -o "../compiled/SPIRV/%%~nf.spv" "%%f"
    )
)