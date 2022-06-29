# ---- Testing ----

function(add_unit_test)
  if(BUILD_TESTING)
    add_executable(test_${ARGV0} test_${ARGV0}.cpp test_factory.cpp)
    target_link_libraries(test_${ARGV0} OmniTools)
    add_test(
      NAME test_${ARGV0}
      COMMAND test_${ARGV0}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endif()
endfunction(add_unit_test)

add_unit_test(endian)
add_unit_test(prime)
add_unit_test(config)
add_unit_test(flowkey)
add_unit_test(hierarchy)
add_unit_test(data)
add_unit_test(metric)
add_unit_test(sketch)