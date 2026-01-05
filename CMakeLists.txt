cmake_minimum_required(VERSION 3.20)
project(UrbanShooter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# SDL2 подтягиваем прямо из репы (без установки по системе)
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.30.0
)
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2)
if (MINGW)
  # MinGW's xinput.h already defines XINPUT_CAPABILITIES_EX; avoid a duplicate typedef in SDL.
  set(_sdl_xinput_h "${SDL2_SOURCE_DIR}/src/core/windows/SDL_xinput.h")
  if (EXISTS "${_sdl_xinput_h}")
    file(READ "${_sdl_xinput_h}" _sdl_xinput_h_content)
    string(FIND "${_sdl_xinput_h_content}" "\r\n" _sdl_crlf_pos)
    if (_sdl_crlf_pos EQUAL -1)
      set(_sdl_nl "\n")
    else()
      set(_sdl_nl "\r\n")
    endif()
    if (NOT _sdl_xinput_h_content MATCHES "#if !defined\\(__MINGW32__\\)")
      string(REPLACE
        "/* This struct is not defined in XInput headers. */"
        "/* This struct is not defined in XInput headers. */${_sdl_nl}#if !defined(__MINGW32__)"
        _sdl_xinput_h_content
        "${_sdl_xinput_h_content}")
      string(REPLACE
        "} XINPUT_CAPABILITIES_EX, *PXINPUT_CAPABILITIES_EX;"
        "} XINPUT_CAPABILITIES_EX, *PXINPUT_CAPABILITIES_EX;${_sdl_nl}#endif /* !__MINGW32__ */"
        _sdl_xinput_h_content
        "${_sdl_xinput_h_content}")
    endif()
    if (_sdl_xinput_h_content MATCHES "#endif /\\* !__MINGW32__ \\*/" AND
        NOT _sdl_xinput_h_content MATCHES "#define ProductVersion VersionNumber")
      string(REPLACE
        "#endif /* !__MINGW32__ */"
        "#endif /* !__MINGW32__ */${_sdl_nl}#if defined(__MINGW32__)${_sdl_nl}#define ProductVersion VersionNumber${_sdl_nl}#endif"
        _sdl_xinput_h_content
        "${_sdl_xinput_h_content}")
    endif()
    file(WRITE "${_sdl_xinput_h}" "${_sdl_xinput_h_content}")
  endif()
endif()

add_executable(UrbanShooter
  src/main.cpp
  src/core/Application.cpp
  src/core/Config.cpp
  src/core/Log.cpp
  src/core/Utils.cpp
  src/game/Chat.cpp
  src/game/Console.cpp
  src/game/Game.cpp
  src/input/Input.cpp
  src/physics/Collision.cpp
  src/world/World.cpp
  src/game/Player.cpp
  src/render/Renderer.cpp
  src/render/UI.cpp
  src/render/Texture.cpp
  src/render/PostProcess.cpp
  src/render/WorldRenderer.cpp
  src/render/GltfModel.cpp
)

# SDL2 targets обычно такие:
target_link_libraries(UrbanShooter PRIVATE SDL2::SDL2main SDL2::SDL2)
target_include_directories(UrbanShooter PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# OpenGL на Windows (для самого старта хватит)
if (WIN32)
  target_link_libraries(UrbanShooter PRIVATE opengl32)
endif()

# Чтобы SDL2.dll оказалась рядом с .exe (иначе будет “dll not found”)
add_custom_command(TARGET UrbanShooter POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2::SDL2>
    $<TARGET_FILE_DIR:UrbanShooter>
)
