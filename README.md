# SDL3 Vulkan Renderer

# Building
## Dependencies
### Linux
- SDL3
- Assimp
- Clang++
- Make

### Windows
- SDL3 binaries ([GitHub](https://github.com/mmozeiko/build-sdl3/)) - Add to **SDL3** env variable
- Shaderc binaries ([GitHub](github.com/google/shaderc)) - Add to **PATH**
- MSVC with Clang (LLVM) support - ([Microsoft](https://visualstudio.microsoft.com/downloads/?q=build+tools#build-tools-for-visual-studio-2022)) - Be sure to **tick** the **Clang compiler** option
- Clang++ and LLVM binaries ([LLVM](https://releases.llvm.org/download.html)) - Add to **PATH**
- Make (from [Chocolatey](https://chocolatey.org/)) - Any mingw build of *make* should work

> The build pipeline is much simpler on Linux, as you can see.

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