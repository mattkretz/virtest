include_directories(${CMAKE_PROJECT_DIR})

set(all "ALL")
if(DEFINED ENV{APPVEYOR})
   set(all)
endif()

add_executable(unittests test.cpp)
if(NOT MSVC)
   set_target_properties(unittests PROPERTIES COMPILE_FLAGS "-std=c++14 -Wall -Wextra")
endif()
add_test(NAME unittests COMMAND unittests)

function(vir_add_run_target target)
   if("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
      # do nothing. This just clutters the solution explorer
   else()
      add_custom_target(run_${target} ${all}
         ${target} -v
         DEPENDS ${target}
         COMMENT "Execute ${target} test"
         VERBATIM
         )
   endif()
endfunction()
vir_add_run_target(unittests)
