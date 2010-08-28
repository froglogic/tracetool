SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/core)

ADD_EXECUTABLE(test_filter test_filter.cpp)
TARGET_LINK_LIBRARIES(test_filter tracelib)

ADD_EXECUTABLE(test_info test_info.cpp)
TARGET_LINK_LIBRARIES(test_info tracelib)

ADD_TEST(NAME test_filter COMMAND test_filter)
ADD_TEST(NAME test_processid COMMAND test_info --processid)
ADD_TEST(NAME test_threadid COMMAND test_info --threadid)
