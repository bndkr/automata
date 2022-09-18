cmake_minimum_required(VERSION 3.18)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
message(${CMAKE_MODULE_PATH})
set(project_name Automata)
project(${project_name})
set(imgui_path ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui CACHE PATH "Path to the dear ImGui header files")
include_directories(${imgui_path})
find_package(D3D11 MODULE) # sets D3D11_lib
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
set(main_srcs
  src/main.cpp
)
source_group(imgui FILES ${imgui_srcs})
source_group("" FILES ${main_srcs})
add_executable(${project_name} ${imgui_srcs} ${main_srcs})
target_link_libraries(${project_name} ${D3D11_lib})