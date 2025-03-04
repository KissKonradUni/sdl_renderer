workspace "SDL3App"
    configurations { "Debug", "Release" }
    architecture "x86_64"

project "SDL3App"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.buildcfg}"

    files { "src/**.cpp" }
    includedirs { "include", "../imgui", "../imgui/backends" }

    filter { "system:windows" }
        defines { "WINDOWS" }
        links { "opengl32" }
        postbuildcommands {
            "{COPY} ./win_dlls/* bin/%{cfg.buildcfg}/"
        }

    filter { "system:not windows" }
        links { "GL" }

    filter {}

    -- SDL3 and Assimp
    buildoptions { "`pkg-config sdl3 --cflags`", "`pkg-config assimp --cflags`" }
    linkoptions { "`pkg-config sdl3 --libs`", "`pkg-config assimp --libs`" }

    -- ImGui
    files {
        "../imgui/imgui.cpp",
        "../imgui/imgui_demo.cpp",
        "../imgui/imgui_draw.cpp",
        "../imgui/imgui_tables.cpp",
        "../imgui/imgui_widgets.cpp",
        "../imgui/backends/imgui_impl_sdl3.cpp",
        "../imgui/backends/imgui_impl_opengl3.cpp"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        sanitize { "Address" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Speed"
        flags { "LinkTimeOptimization" }
        buildoptions { "-march=native", "-flto", "-fomit-frame-pointer" }