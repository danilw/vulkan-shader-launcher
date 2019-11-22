cmake_minimum_required(VERSION 2.8)

project("vk_launcher")

find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message("ERROR: Vulkan not found")
endif(NOT Vulkan_FOUND)
set(Vulkan_LIBRARY)

if (Vulkan_FOUND)
    MESSAGE(STATUS "  [Found Vulkan library]")
    MESSAGE(STATUS "  Vulkan include directory:" ${Vulkan_INCLUDE_DIR})
    FOREACH(item ${Vulkan_LIBRARIES})
        MESSAGE(STATUS "  using lib: " ${item})
    ENDFOREACH()
    include_directories(${Vulkan_INCLUDE_DIR})
endif()

set(SOURCES
	"vklauncher.c" 
)

add_definitions(-O3 -fdata-sections -ffunction-sections -Wl,--gc-sections -Wall -Wextra)

add_executable(
		VKme
		${SOURCES} 
)

IF (WIN32)

target_link_libraries(VKme ${Vulkan_LIBRARY})

ELSE()

function(shader_compile SHADER_SOURCE)
execute_process(
    COMMAND python ../shader_to_hex.py ${SHADER_SOURCE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
endfunction(shader_compile)

shader_compile("main.vert")
shader_compile("main.frag")

target_link_libraries(VKme m xcb ${Vulkan_LIBRARY})

ENDIF()
