project(proposed_test LANGUAGES CXX C)

message(STATUS "Configuring unit testing program...")
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} 
PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests_wrapper.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests.c
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/timer.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/unity.c 
)

target_include_directories(${PROJECT_NAME} 
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/resources/>  
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Link with SAF
target_link_libraries(${PROJECT_NAME} PUBLIC interface core saf)
