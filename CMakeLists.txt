cmake_minimum_required(VERSION 3.20)

# ============================================================
# User-configurable section（ここだけ触ればOK）
# ============================================================
set(YW_EXECUTABLE_NAME ywlib_app)
option(YW_GENERATE_UMBRELLA "Generate umbrella headers (ywlib/ywlib, ywlib/ywxlib)" ON)

project(ywlib LANGUAGES CXX)

# Always export compile_commands (useful for VSCode)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# Executable (your current build target)
# ============================================================
add_executable(${YW_EXECUTABLE_NAME}
  main.cpp
)

target_compile_features(${YW_EXECUTABLE_NAME} PRIVATE cxx_std_23)

# <ywlib> と <ywxlib> を拾うために "ywlib/" を include パスに追加
target_include_directories(${YW_EXECUTABLE_NAME} PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/ywlib"
)

if (WIN32)
  target_compile_definitions(${YW_EXECUTABLE_NAME}
    PRIVATE NOMINMAX UNICODE _UNICODE
  )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND WIN32)
  target_link_libraries(ywlib_app PRIVATE stdc++exp)
endif()

# ============================================================
# Umbrella generation
#   - ywlib/ywlib : includes yw/*.h, *.hpp
#   - ywlib/ywxlib: includes ywx/*.h, *.hpp (Windows/DX) and starts with:
#       #pragma once
#       #include <ywlib>
# ============================================================
if (YW_GENERATE_UMBRELLA)
  find_package(Python3 COMPONENTS Interpreter REQUIRED)

  set(YW_GEN_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_umbrella.py")

  # ----- ywlib/ywlib -----
  set(YW_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/ywlib/yw")
  set(YW_UMBRELLA "${CMAKE_CURRENT_SOURCE_DIR}/ywlib/ywlib")

  file(GLOB YW_HEADERS CONFIGURE_DEPENDS
    "${YW_DIR}/*.h"
    "${YW_DIR}/*.hpp"
  )

  add_custom_command(
    OUTPUT "${YW_UMBRELLA}"
    COMMAND "${Python3_EXECUTABLE}" "${YW_GEN_SCRIPT}"
            --in-dir "${YW_DIR}"
            --out "${YW_UMBRELLA}"
            --prefix "yw"
    DEPENDS ${YW_HEADERS} "${YW_GEN_SCRIPT}"
    VERBATIM
  )

  add_custom_target(ywlib_umbrella_yw DEPENDS "${YW_UMBRELLA}")

  # ----- ywlib/ywxlib (Windows/DX) -----
  # 生成自体はクロスでも可能ですが、"Windows+DirectX用" という意図に合わせて WIN32 でのみ有効にしています。
  if (WIN32)
    set(YWX_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/ywlib/ywx")
    set(YWX_UMBRELLA "${CMAKE_CURRENT_SOURCE_DIR}/ywlib/ywxlib")

    file(GLOB YWX_HEADERS CONFIGURE_DEPENDS
      "${YWX_DIR}/*.h"
      "${YWX_DIR}/*.hpp"
    )

    add_custom_command(
      OUTPUT "${YWX_UMBRELLA}"
      COMMAND "${Python3_EXECUTABLE}" "${YW_GEN_SCRIPT}"
              --in-dir "${YWX_DIR}"
              --out "${YWX_UMBRELLA}"
              --prefix "ywx"
              --preamble "#include <ywlib>"
      DEPENDS ${YWX_HEADERS} "${YW_GEN_SCRIPT}"
      VERBATIM
    )

    add_custom_target(ywlib_umbrella_ywx DEPENDS "${YWX_UMBRELLA}")

    add_custom_target(ywlib_umbrellas ALL)
    add_dependencies(ywlib_umbrellas ywlib_umbrella_yw ywlib_umbrella_ywx)

    # Ensure generated headers exist before compiling main.cpp
    add_dependencies(${YW_EXECUTABLE_NAME} ywlib_umbrellas)
  else()
    add_custom_target(ywlib_umbrellas ALL)
    add_dependencies(ywlib_umbrellas ywlib_umbrella_yw)
    add_dependencies(${YW_EXECUTABLE_NAME} ywlib_umbrellas)
  endif()
endif()
