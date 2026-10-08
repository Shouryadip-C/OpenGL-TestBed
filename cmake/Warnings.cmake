# Warning flags for our own targets. Third-party code under extern/ keeps its own settings.
function(testbed_set_warnings target)
  if(MSVC)
    # C4100: unused parameter, see below
    target_compile_options(${target} PRIVATE /W4 /permissive- /wd4100)
  else()
    # The Test hooks are empty virtuals with named parameters, so unused-parameter would fire
    # for every one of them in every file that includes Test.h
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter)
  endif()
endfunction()
