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

include_directories("../yariv")

add_executable(
		VKme
		${SOURCES} 
)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(-DVK_USE_PLATFORM_WIN32_KHR /Wall /MD /O1 /Ob1 /DNDEBUG)
  target_link_libraries(VKme ${Vulkan_LIBRARY})
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  function(shader_compile SHADER_SOURCE)
  execute_process(
    COMMAND python ../yariv_to_hex.py ${SHADER_SOURCE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  endfunction(shader_compile)

  shader_compile("main.vert")
  shader_compile("main.frag")

  target_link_libraries(VKme m xcb ${Vulkan_LIBRARY})
else()
  message(WARNING "Unknown compiler '${CMAKE_C_COMPILER_ID}'!")
endif()
