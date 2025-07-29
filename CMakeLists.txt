cmake_minimum_required( VERSION 3.21 )

project( g923mac VERSION 0.0.1 )

set( CMAKE_CXX_STANDARD            23 )
set( CMAKE_CXX_STANDARD_REQUIRED True )

set( CMAKE_OSX_ARCHITECTURES "x86_64" )

add_compile_options( -Wall -Wextra -pedantic -Werror -fno-exceptions -fno-rtti -O3 -DUTI_RELEASE )

add_library( g923mac SHARED plugin.cpp )

target_include_directories( g923mac PUBLIC
                                include
                                include/g923mac
                                include/scs/include
                                include/scs/include/common
                                include/scs/include/amtrucks
                                include/scs/include/eurotrucks2
)
target_link_libraries( g923mac "-framework CoreFoundation" )
target_link_libraries( g923mac "-framework          IOKit" )
