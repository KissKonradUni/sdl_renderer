# SDL3 OpenGL Renderer

# Building
## Dependencies
### Linux
- From the package manager (Arch):
    - Clang++
    - Make
    - SDL3
    - Assimp
- From GitHub:
    - Dear Imgui [source](https://github.com/ocornut/imgui) `// Use the docking branch. Specify the path in the Makefile`

### Windows
- MSYS2 ([https://www.msys2.org/](https://www.msys2.org/))
- UCRT64 packages of the following:
    - Clang++ (`mingw-w64-ucrt-x86_64-clang`)
    - Make (`mingw-w64-ucrt-x86_64-make` and regular `make`)
    - SDL3 (`mingw-w64-ucrt-x86_64-SDL3`)
    - Assimp (`mingw-w64-ucrt-x86_64-assimp`)
- From GitHub:
    - Dear Imgui [source](https://github.com/ocornut/imgui) `// Use the docking branch. Specify the path in the Makefile`

> On windows the compilation should be done from the MSYS2 UCRT64 terminal.
> Also, the .dll files of the dependencies should be provided in the `win_dlls` folder in order to make the executable shippable.
> The `win_dlls/dll_list.txt` contains the necessary .dll files' names.

## Licenses
> Of used 3rd party extras
- SDL3: [ZLib license](https://www.libsdl.org/license.php)
- Assimp: [BSD 3-Clause license](https://github.com/assimp/assimp/blob/master/LICENSE)
- Glad: [Public Domain, WTFPL or CC0](https://github.com/Dav1dde/glad)
- Dear Imgui: [MIT license](https://github.com/ocornut/imgui?tab=MIT-1-ov-file)
- stb_image: [Public Domain](https://github.com/nothings/stb/tree/master?tab=License-1-ov-file)
- FiraCode font: [SIL Open Font License](https://github.com/tonsky/FiraCode?tab=OFL-1.1-1-ov-file)
- Nholmann JSON: [MIT license](https://github.com/nlohmann/json?tab=MIT-1-ov-file)
- IconFontCPPHeaders: [ZLib license](https://github.com/juliettef/IconFontCppHeaders?tab=Zlib-1-ov-file)
- Material Symbols Font: [Apache 2.0](https://developers.google.com/fonts/docs/material_symbols#licensing)

## Compiling
```sh
# Compile 3rd party libraries
make -C lib
# Compile the project
make
```
## Running
```sh
# Run the project
make run
```

## Debugging
Visual Studio Code:
- Install the **Clangd** extension
- Install the **C/C++** extension (disable the **IntelliSense** engine, clangd already provides it)
- Add the **CodeLLDB** extension for debugging
- Run the **(lldb) Build and debug** task (F5)