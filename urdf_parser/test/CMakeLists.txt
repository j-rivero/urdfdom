include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/gtest/include
  ${CMAKE_CURRENT_SOURCE_DIR}/gtest
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Build gtest
add_library(gtest STATIC gtest/src/gtest-all.cc)
add_library(gtest_main STATIC gtest/src/gtest_main.cc)
target_link_libraries(gtest_main gtest)
target_compile_features(gtest PUBLIC cxx_std_11)

execute_process(COMMAND cmake -E remove_directory ${CMAKE_BINARY_DIR}/test_results)
execute_process(COMMAND cmake -E make_directory ${CMAKE_BINARY_DIR}/test_results)

# unit test to fix geometry problems
set(tests
     urdf_double_convert.cpp
     urdf_unit_test.cpp
     urdf_version_test.cpp
     urdf_model_test.cpp
     urdf_joint_test.cpp
     urdf_link_test.cpp
     urdf_error_test.cpp
     urdf_pose_advanced_test.cpp
     urdf_joint_dynamics_test.cpp
     urdf_geometry_test.cpp
)

# Performance test (separate handling)
set(performance_tests
     urdf_performance_test.cpp
)

#################################################
# Build all the tests
foreach(GTEST_SOURCE_file ${tests})
  string(REGEX REPLACE ".cpp" "" BINARY_NAME ${GTEST_SOURCE_file})
  add_executable(${BINARY_NAME} ${GTEST_SOURCE_file})

  target_link_libraries(${BINARY_NAME}
    gtest_main
    gtest
    urdfdom_model
  )
  if (UNIX)
    target_link_libraries(${BINARY_NAME} pthread)
  endif()

if(GTEST_SOURCE_file STREQUAL "urdf_export_test.cpp")
    target_link_libraries(${BINARY_NAME}
      tinyxml2::tinyxml2
      urdfdom_world
    )
endif()

  add_test(NAME    ${BINARY_NAME}
           COMMAND ${BINARY_NAME}
                   --gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${BINARY_NAME}.xml)

  set_tests_properties(${BINARY_NAME} PROPERTIES TIMEOUT 240 ENVIRONMENT LC_ALL=C)

  add_test(NAME    ${BINARY_NAME}_locale
           COMMAND ${BINARY_NAME}
                   --gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${BINARY_NAME}_locale.xml)

  set_tests_properties(${BINARY_NAME}_locale PROPERTIES TIMEOUT 240 ENVIRONMENT LC_ALL=nl_NL.UTF-8)
endforeach()

#################################################
# Build performance tests (with different handling)
foreach(PERF_SOURCE_file ${performance_tests})
  string(REGEX REPLACE ".cpp" "" BINARY_NAME ${PERF_SOURCE_file})
  add_executable(${BINARY_NAME} ${PERF_SOURCE_file})

  target_link_libraries(${BINARY_NAME}
    urdfdom_model
    urdfdom_world
  )
  if (UNIX)
    target_link_libraries(${BINARY_NAME} pthread)
  endif()

  # Performance tests are not added to the regular test suite
  # They can be run manually or with make performance-test target

endforeach()

# Add custom target for performance testing
add_custom_target(performance-test
  COMMAND ${CMAKE_BINARY_DIR}/bin/urdf_performance_test
  DEPENDS urdf_performance_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running URDF parser performance benchmark"
)

# Add quick performance test target
add_custom_target(performance-test-quick
  COMMAND ${CMAKE_BINARY_DIR}/bin/urdf_performance_test --quick
  DEPENDS urdf_performance_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running quick URDF parser performance benchmark"
)
