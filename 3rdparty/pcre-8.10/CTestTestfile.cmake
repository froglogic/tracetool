# CMake generated Testfile for 
# Source directory: S:/tracelib/3rdparty/pcre-8.10
# Build directory: S:/tracelib/3rdparty/pcre-8.10
# 
# This file includes the relevent testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
ADD_TEST(pcre_test "cmd" "/C" "S:/tracelib/3rdparty/pcre-8.10/RunTest.bat")
ADD_TEST(pcrecpp_test "S:/tracelib/3rdparty/pcre-8.10/pcrecpp_unittest.exe")
ADD_TEST(pcre_scanner_test "S:/tracelib/3rdparty/pcre-8.10/pcre_scanner_unittest.exe")
ADD_TEST(pcre_stringpiece_test "S:/tracelib/3rdparty/pcre-8.10/pcre_stringpiece_unittest.exe")
