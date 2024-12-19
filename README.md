# SDL3 Vulkan Renderer

# Building
## Dependencies
### Linux
- SDL3 (from AUR)
- Shaderc
- Clang++
- Make

### Windows
- SDL3 binaries ([GitHub](https://github.com/mmozeiko/build-sdl3/))
- Shaderc binaries ([GitHub](github.com/google/shaderc)) - Add to PATH
- MSVC with Clang (LLVM) support - ([Microsoft](https://visualstudio.microsoft.com/downloads/?q=build+tools#build-tools-for-visual-studio-2022)) - Be sure to tick the Clang compiler option
- Make (from [Chocolatey](https://chocolatey.org/))
- Clang++ and LLVM binaries ([LLVM](https://releases.llvm.org/download.html)) - Add to PATH

## Compiling
```sh
make shaders
make
```
## Running
```sh
make run
```