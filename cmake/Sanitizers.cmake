# enable sanitizers if they are configured
function(
  arquebus_enable_sanitizers
  this_project_name
  enable_sanitizer_address
  enable_sanitizer_leak
  enable_sanitizer_undefined_behavior
  enable_sanitizer_thread
  enable_sanitizer_memory
)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(sanitizers "")

    if(${enable_sanitizer_address})
      list(APPEND sanitizers "address")
    endif()

    if(${enable_sanitizer_leak})
      list(APPEND sanitizers "leak")
    endif()

    if(${enable_sanitizer_undefined_behavior})
      list(APPEND sanitizers "undefined")
    endif()

    if(${enable_sanitizer_thread})
      if("address" IN_LIST sanitizers OR "leak" IN_LIST sanitizers)
        message(WARNING "Thread sanitizer does not work with Address and Leak sanitizer enabled")
      else()
        list(APPEND sanitizers "thread")
      endif()
    endif()

    if(${enable_sanitizer_memory} AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      message(WARNING "Memory sanitizer requires all the code (including libc++) to be MSan-instrumented "
                      "otherwise it reports false positives"
      )
      if("address" IN_LIST sanitizers
         OR "thread" IN_LIST sanitizers
         OR "leak" IN_LIST sanitizers
      )
        message(WARNING "Memory sanitizer does not work with Address, Thread or Leak sanitizer enabled")
      else()
        list(APPEND sanitizers "memory")
      endif()
    endif()
  elseif(MSVC)
    if(${enable_sanitizer_address})
      list(APPEND sanitizers "address")
    endif()
    if(${enable_sanitizer_leak}
       OR ${enable_sanitizer_undefined_behavior}
       OR ${enable_sanitizer_thread}
       OR ${enable_sanitizer_memory}
    )
      message(WARNING "MSVC only supports address sanitizer")
    endif()
  endif()

  list(JOIN sanitizers "," list_of_sanitizers)

  if(list_of_sanitizers)
    if(NOT "${list_of_sanitizers}" STREQUAL "")
      if(NOT MSVC)
        target_compile_options(${this_project_name} INTERFACE -fsanitize=${list_of_sanitizers})
        target_link_options(${this_project_name} INTERFACE -fsanitize=${list_of_sanitizers})
      else()
        string(FIND "$ENV{PATH}" "$ENV{VSINSTALLDIR}" index_of_vs_install_dir)
        if("${index_of_vs_install_dir}" STREQUAL "-1")
          message(SEND_ERROR "Using MSVC sanitizers requires setting the MSVC environment before building the project. "
                             "Please manually open the MSVC command prompt and rebuild the project."
          )
        endif()
        target_compile_options(${this_project_name} INTERFACE /fsanitize=${list_of_sanitizers} /Zi /INCREMENTAL:NO)
        target_compile_definitions(${this_project_name} INTERFACE _DISABLE_VECTOR_ANNOTATION _DISABLE_STRING_ANNOTATION)
        target_link_options(${this_project_name} INTERFACE /INCREMENTAL:NO)
      endif()
    endif()
  endif()

endfunction()
