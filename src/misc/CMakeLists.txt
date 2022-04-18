set(SOURCES
  utils.cpp)
add_library(misc ${SOURCES})
target_include_directories(misc PUBLIC ${PROJECT_INCLUDE_DIR})
target_link_libraries(misc ${PROJECT_LINK_LIBRARIES})
