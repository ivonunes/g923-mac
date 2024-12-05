cmake_minimum_required( VERSION 3.21 )

project( fffb VERSION 0.0.1 )

set( CMAKE_CXX_STANDARD            23 )
set( CMAKE_CXX_STANDARD_REQUIRED True )

set( CMAKE_OSX_ARCHITECTURES "x86_64" )

add_compile_options( -Wall -Wextra -pedantic -Werror -fno-exceptions -O3 -DUTI_RELEASE )

add_library( fffb SHARED plugin.cpp source/util.cpp source/command.cpp source/device.cpp )

target_include_directories( fffb PUBLIC
                                include
                                deps
                                deps/uti
                                deps/scs/include
                                deps/scs/include/common
                                deps/scs/include/amtrucks
                                deps/scs/include/eurotrucks2
)

target_link_libraries( fffb "-framework CoreFoundation" )
target_link_libraries( fffb "-framework          IOKit" )
