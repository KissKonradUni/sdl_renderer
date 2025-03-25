# 🔥 Cinder - an SDL3 OpenGL rendering engine

[![Makefile](https://img.shields.io/badge/Makefile-red?style=for-the-badge)](./makefile) ![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![Version](https://img.shields.io/badge/version-0.1.0-blue?style=for-the-badge) [![view - Documentation](https://img.shields.io/badge/view-Documentation-blue?style=for-the-badge)](./docs/Cinder.md "Go to project documentation") [![view - License](https://img.shields.io/badge/view-License-green?style=for-the-badge)](./LICENSE.md "Go to project license")

> Created as a thesis for my Bachelor's degree in Computer Science.

> <ins>**Created by:**</ins>
👤 Konrád Soma Kiss
📚 CNY8MP

## 📕 Description

This project is a "modern" OpenGL rendering engine built using SDL3, designed to provide a flexible and efficient framework for graphics applications.

The engine features ImGui integration for fast development, 3D model loading via Assimp, and a structured architecture for managing assets. It's cross-platform compatible for both Linux and Windows. The renderer provides a solid foundation for developing graphical applications, visualizations, and maybe games.

The main goal of the project (as a thesis) is to showcase techniques that don't require the newest hardware, AI capabilities or complex graphics APIs to achieve a visually appealing result.

## ⚙️ Dependencies

### 🐧 Linux

- From the package manager (Arch):
  - Clang++
  - Make
  - SDL3
  - Assimp
- From GitHub:
  - Dear Imgui [source](https://github.com/ocornut/imgui) `// Use the docking branch. Specify the path in the Makefile`

### 🪟 Windows

- MSYS2 ([https://www.msys2.org/](https://www.msys2.org/))
- UCRT64 packages of the following:
  - Clang++ (`mingw-w64-ucrt-x86_64-clang`)
  - Make (`mingw-w64-ucrt-x86_64-make` and regular `make`)
  - SDL3 (`mingw-w64-ucrt-x86_64-SDL3`)
  - Assimp (`mingw-w64-ucrt-x86_64-assimp`)
- From GitHub:
  - Dear Imgui [source](https://github.com/ocornut/imgui) `// Use the docking branch. Specify the path in the Makefile`

> On windows the compilation should be done from the MSYS2 UCRT64 terminal. Also, the .dll files of the dependencies should be provided in the `win_dlls` folder in order to make the executable shippable. The `win_dlls/dll_list.txt` contains the necessary .dll files' names.

## 📦 Cloning

```sh
# Clone the repository
git clone https://github.com/KissKonradUni/sdl_renderer --depth 1 # Recommended, the repository is quite large
# Clone imgui next to the project
git clone https://github.com/ocornut/imgui -b docking --depth 1
```

## 🛠️ Compiling

```sh
# Enter the project directory
cd sdl_renderer
# Compile 3rd party libraries
make -C lib
# Compile the project
make
```

## 🏃 Running

```sh
# Run the project
make run
```

or

``` sh
# The working directory has to be the folder containing the assets folder
./bin/sdl3_app
```

## 🐞 Debugging

> Visual Studio Code

- Install the **Clangd** extension
- Install the **C/C++** extension (disable the **IntelliSense** engine, clangd already provides it)
- Add the **CodeLLDB** extension for debugging
- Run the **(lldb) Build and debug** task (F5)

## 📜 Licenses

> Of the project

[GNU General Public License v3.0](./LICENSE.md)

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
