cmake_minimum_required(VERSION 3.15)
project(demo CXX)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
  add_compile_options(/wd4273)
  foreach(flag_var CMAKE_SHARED_LINKER_FLAGS CMAKE_STATIC_LINKER_FLAGS
                   CMAKE_EXE_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS)
    string(APPEND ${flag_var} " /ignore:4049 /ignore:4217 /ignore:4099")
  endforeach(flag_var)
endif()

add_library(meta SHARED meta.cpp)
target_compile_definitions(meta PRIVATE BUILD_MAIN_LIB)

set(FOR_CUDA FALSE)

if (FOR_CUDA)
  # b.dll simulates libtorch_cuda + libtorch_cpu (meta.cpp)
  add_library(b SHARED a.cpp b.cpp)
  target_include_directories(b PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(b PUBLIC meta)
else ()
  # a.cpp -> a.lib (simulates libtorch_xpu_ops.a)
  # No BUILDING_B_LIB: MY_API = dllimport always, this is the issue site
  add_library(a STATIC a.cpp)
  target_include_directories(a PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(a PUBLIC meta)

  # b.dll simulates libtorch_xpu + libtorch_cpu (meta.cpp)
  add_library(b SHARED b.cpp)
  target_include_directories(b PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(b PUBLIC meta)
  # simulates whole archive link in PyTorch, suppress LNK2019
  target_link_libraries(b PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,a>)
endif ()

# c.exe simulates user code (libtorch_python)
add_executable(c c.cpp)
target_link_libraries(c PRIVATE meta)
target_link_libraries(c PRIVATE b)
