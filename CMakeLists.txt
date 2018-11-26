cmake_minimum_required( VERSION 3.6 )

project( parser )

enable_language( CXX )

find_package( Qt5 COMPONENTS Core REQUIRED )

set( CMAKE_SYSTEM_NAME "Linux" CACHE INTERNAL "CMake system name" )
set( CMAKE_SYSTEM_PROCESSOR "amd64" CACHE INTERNAL "CMake processor" )
set( CMAKE_VERBOSE_MAKEFILE OFF CACHE INTERNAL "CMake verbose makefile" )

set( CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE INTERNAL "CMake cxx compiler" )
set( CMAKE_OBJCOPY_UTIL "/usr/bin/objcopy" CACHE INTERNAL "CMake objcopy util" )
set( CMAKE_OBJDUMP_UTIL "/usr/bin/objdump" CACHE INTERNAL "CMake objdump util" )
set( CMAKE_SIZE_UTIL "/usr/bin/size" CACHE INTERNAL "CMake size util" )
set( CMAKE_GDB_BACKEND "/usr/bin/gdb-multiarch" CACHE INTERNAL "CMake gdb backend" )
set( CMAKE_GDB_FRONTEND "/usr/bin/cgdb" CACHE INTERNAL "CMake gdb frontend" )

set( CMAKE_CXX_FLAGS "-g -pipe -std=gnu++11 -stdlib=libc++" CACHE INTERNAL "cxx compiler flags" )
set( CMAKE_EXE_LINKER_FLAGS "-g -pipe -stdlib=libc++" CACHE INTERNAL "exe link flags" )

set( PROJECT_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc" CACHE INTERNAL "Project include path" )
file( GLOB PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.s" CACHE INTERNAL "Project sources" )

set( QT_HEADERS_PATH "/usr/include/x86_64-linux-gnu/qt5" )
set( QT_CORE_HEADERS_PATH "/usr/include/x86_64-linux-gnu/qt5/QtCore" )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang_complete.in ${CMAKE_CURRENT_SOURCE_DIR}/.clang_complete )
include_directories( ${PROJECT_INC_DIR} ${QT_HEADERS_PATH} ${QT_CORE_HEADERS_PATH} )

add_executable( ${CMAKE_PROJECT_NAME}.elf ${PROJECT_SOURCES} )
qt5_use_modules( ${CMAKE_PROJECT_NAME}.elf Core )

add_custom_command( TARGET ${CMAKE_PROJECT_NAME}.elf POST_BUILD COMMAND ${CMAKE_SIZE_UTIL} ${CMAKE_PROJECT_NAME}.elf )

add_custom_target( dump COMMAND ${CMAKE_OBJDUMP_UTIL} -t ${CMAKE_PROJECT_NAME}.elf )
add_custom_target( size COMMAND ${CMAKE_SIZE_UTIL} ${CMAKE_PROJECT_NAME}.elf )
