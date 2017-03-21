include_directories(${CMAKE_SOURCE_DIR})

set(all "ALL")
if(DEFINED ENV{APPVEYOR})
   set(all)
endif()

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
function(vir_add_test name)
   add_executable(${name} ${name}.cpp)
   if(NOT MSVC)
      set_target_properties(${name} PROPERTIES COMPILE_FLAGS "-std=c++1y -Wall -Wextra")
   endif()
   add_test(NAME ${name} COMMAND ${name})
   vir_add_run_target(${name})
endfunction()

vir_add_test(checks)
vir_add_test(empty)
