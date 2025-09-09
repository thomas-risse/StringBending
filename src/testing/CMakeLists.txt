cmake_minimum_required(VERSION 3.19)
project(testNlBendProcessor)

set(AUDIO_PROCESSORS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../engine)

message("${AUDIO_PROCESSORS_DIR}")

set(SOURCE_FILES
  testNlBendProcessor.cpp
  ${AUDIO_PROCESSORS_DIR}/NlBendProcessor.cpp
)

add_executable(testNlBendProcessor ${SOURCE_FILES})

target_include_directories(testNlBendProcessor PRIVATE ${AUDIO_PROCESSORS_DIR})

find_package(Eigen3 REQUIRED NO_MODULE)
target_link_libraries(testNlBendProcessor PUBLIC Eigen3::Eigen)

target_compile_features(testNlBendProcessor PRIVATE cxx_std_17)