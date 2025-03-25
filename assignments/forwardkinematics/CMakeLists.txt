file(
 GLOB_RECURSE FORWARDKINEMATICS_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE FORWARDKINEMATICS_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignment4's asset folder to bin when it is built
add_custom_target(copyAssetsA6 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${FORWARDKINEMATICS_INC} DESTINATION include/forwardkinematics)
add_executable(forwardkinematics ${FORWARDKINEMATICS_SRC} ${FORWARDKINEMATICS_INC})
target_link_libraries(forwardkinematics PUBLIC core IMGUI assimp)
target_include_directories(forwardkinematics PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when forwardkinematics is built
add_dependencies(assignment4 copyAssetsA6)