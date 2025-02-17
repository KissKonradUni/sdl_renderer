# SDL3 Vulkan Renderer

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
- SDL3: [zlib license](https://www.libsdl.org/license.php)
- Assimp: [BSD 3-Clause license](https://github.com/assimp/assimp/blob/master/LICENSE)
- Glad: [Public Domain, WTFPL or CC0](https://github.com/Dav1dde/glad)
- Dear Imgui: [MIT license](https://github.com/ocornut/imgui?tab=MIT-1-ov-file)

## Compiling
```sh
make
```
## Running
```sh
make run
```

## Debugging
Visual Studio Code:
- Install the **Clangd** extension
- Install the **C/C++** extension (disable the **IntelliSense** engine, clangd already provides it)
- Add the **CodeLLDB** extension for debugging
- Run the **(lldb) Build and debug** task (F5)