cmake_minimum_required(VERSION 3.21)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
set(project_name Automata)
project(${project_name})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
find_package(D3D11 MODULE) # sets D3D11_lib
###################################
set(main_srcs
  src/main.cpp
)
set(srcs ${main_srcs})
source_group("src" FILES ${main_srcs})
###################################
set(imgui_srcs
  src/imgui/imconfig.h
  src/imgui/imgui_demo.cpp
  src/imgui/imgui_draw.cpp
  src/imgui/imgui_internal.h
  src/imgui/imgui_tables.cpp
  src/imgui/imgui_widgets.cpp
  src/imgui/imgui.cpp
  src/imgui/imgui.h
  src/imgui/imgui_impl_win32.cpp
  src/imgui/imgui_impl_win32.h
  src/imgui/imgui_impl_dx11.cpp
  src/imgui/imgui_impl_dx11.h
)
list(APPEND srcs ${imgui_srcs})
source_group("imgui" FILES ${imgui_srcs})
####################################
set(font_srcs
  fonts/segoeui.ttf
  )
####################################
set(automata_srcs
  src/automata/Grid.cpp
  src/automata/Grid.hpp
  src/automata/Conways.cpp
  src/automata/Conways.hpp
  src/automata/Elementary.cpp
  src/automata/Elementary.hpp
  src/automata/Fractal.cpp
  src/automata/Fractal.hpp
  src/automata/Gradient.cpp
  src/automata/Gradient.hpp
  src/automata/Julia.cpp
  src/automata/Julia.hpp
  src/automata/Mandelbrot.cpp
  src/automata/Mandelbrot.hpp
  src/automata/Palette.cpp
  src/automata/Palette.hpp
)
list(APPEND srcs ${automata_srcs})
source_group("automata" FILES ${automata_srcs})
####################################
set(utils_srcs
  src/utils/LoadTextureFromData.cpp
  src/utils/LoadTextureFromData.hpp
)
list(APPEND srcs ${utils_srcs})
source_group("utils" FILES ${utils_srcs})
####################################
add_executable(${project_name} ${srcs})
install(TARGETS ${project_name} DESTINATION ${CMAKE_BINARY_DIR}/bin)
install(FILES ${font_srcs} DESTINATION ${CMAKE_BINARY_DIR}/bin/fonts)
target_link_libraries(${project_name} ${D3D11_lib})