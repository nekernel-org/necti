cmake_minimum_required(VERSION 3.10)
project(NeCTICodeGen)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(CodegenTestBasic NectarCodegen.test.cpp)
target_link_libraries(CodegenTestBasic gtest_main)

set_property(TARGET CodegenTestBasic PROPERTY CXX_STANDARD 20)
target_include_directories(CodegenTestBasic PUBLIC ../../)

include(GoogleTest)
gtest_discover_tests(CodegenTestBasic)
