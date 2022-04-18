add_subdirectory(misc)

add_executable(chat_client chat_client.cpp)
target_include_directories(chat_client PUBLIC ${PROJECT_INCLUDE_DIR} ./misc)
target_link_libraries(chat_client ${PROJECT_LINK_LIBRARIES} misc)

add_executable(sync_client sync_client.cpp)
target_include_directories(sync_client PUBLIC ${PROJECT_INCLUDE_DIR} ./misc)
target_link_libraries(sync_client ${PROJECT_LINK_LIBRARIES} misc)
