add_executable(main main.cpp)
target_include_directories(main PUBLIC ${PROJECT_INCLUDE_DIR})
target_link_libraries(main ${PROJECT_LINK_LIBRARIES})
