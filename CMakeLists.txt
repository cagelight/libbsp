cmake_minimum_required( VERSION 3.15 )
project( libbsp VERSION 0.0.1 )

enable_language( CXX )
set( CMAKE_CONFIGURATION_TYPES "Debug;Release;Native" )

set( CMAKE_CXX_FLAGS         "-std=c++2a" )
set( CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -Wpedantic -Og -march=core2 -mtune=native -ggdb3" )
set( CMAKE_CXX_FLAGS_RELEASE "-w -O2 -march=core2 -mtune=generic -DNDEBUG" )
set( CMAKE_CXX_FLAGS_NATIVE  "-w -Ofast -march=native -mtune=native -DNDEBUG" )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Release )
endif()
message( "Using build configuration: ${CMAKE_BUILD_TYPE}" )

configure_file( "${CMAKE_SOURCE_DIR}/src/include/libbsp_version.hh.in" "include/libbsp_version.hh" )
include_directories( "${CMAKE_SOURCE_DIR}/src/include" "${CMAKE_BINARY_DIR}/include" )

file( GLOB_RECURSE LIB_FILES 
	"${CMAKE_SOURCE_DIR}/src/lib/*.cc"
)

add_library( libbsp SHARED ${LIB_FILES} )
set_target_properties( libbsp PROPERTIES PREFIX "" )
install( TARGETS libbsp LIBRARY DESTINATION "lib" )

file( GLOB_RECURSE TOOL_FILES 
	"${CMAKE_SOURCE_DIR}/src/tool/*.cc"
)

add_executable( bsptool ${TOOL_FILES} )
set_target_properties( libbsp PROPERTIES
	SOVERSION 0
	VERSION   ${PROJECT_VERSION}
)
target_link_libraries( bsptool PUBLIC libbsp )
install( TARGETS bsptool RUNTIME DESTINATION "bin" )

install( FILES "${CMAKE_SOURCE_DIR}/src/include/libbsp.hh" DESTINATION "include" )
