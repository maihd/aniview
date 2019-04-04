local ANIVIEW_DIR = path.getabsolute("..")
local ANIVIEW_BUILD_DIR = path.join(ANIVIEW_DIR, "build")
local ANIVIEW_3RDPARTY_DIR = path.join(ANIVIEW_DIR, "3rdparty")

solution "aniview"
do
    language "c++"
    location (ANIVIEW_BUILD_DIR)

    platforms { "x32", "x64", "native" }
    configurations { "debug", "release" }

    configuration { "debug" }
    do
       flags {
          "Symbols"
       }
    end

    startproject "aniview"

    configuration {}
end

-- load toolchain
dofile (path.join(ANIVIEW_DIR, "3rdparty/riku/scripts/toolchain.genie.lua"))
toolchain (ANIVIEW_BUILD_DIR, ANIVIEW_3RDPARTY_DIR)

-- load riku project
dofile (path.join(ANIVIEW_DIR, "3rdparty/riku/scripts/riku.genie.lua"))

-- load sora project
dofile (path.join(ANIVIEW_DIR, "3rdparty/sora/scripts/sora.genie.lua"))

-- main project
project "aniview"
do
    kind "consoleapp"

    files {
        path.join(ANIVIEW_DIR, "src/*.h"),
        path.join(ANIVIEW_DIR, "src/*.c"),
        path.join(ANIVIEW_DIR, "src/*.cc"),
        path.join(ANIVIEW_DIR, "src/*.cpp"),
        path.join(ANIVIEW_DIR, "src/**/*.h"),
        path.join(ANIVIEW_DIR, "src/**/*.c"),
        path.join(ANIVIEW_DIR, "src/**/*.cc"),
        path.join(ANIVIEW_DIR, "src/**/*.cpp"),
    }

    links {
        "riku",
        "sora",
        "gdi32",
        "user32",
        "kernel32",
        "opengl32",
    }

    includedirs {
        path.join(ANIVIEW_DIR, "include"),
        path.join(ANIVIEW_DIR, "3rdparty/riku/include"),
        path.join(ANIVIEW_DIR, "3rdparty/sora/include"),
    }
end