option(ENABLE_CPPCHECK "Enable static analysis with cppcheck" OFF)
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
option(ENABLE_INCLUDE_WHAT_YOU_USE "Enable static analysis with include-what-you-use" OFF)

if(ENABLE_CPPCHECK)
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)
    # Create cppcheck output/cache dir for faster runs
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cppcheck_output)
    set(CMAKE_CXX_CPPCHECK
        ${CPPCHECK}
        # Waiting on https://gitlab.kitware.com/cmake/cmake/-/issues/25641
        # Once using project cppcheck should have way less false flags
        # e.g. unused functions because it'll scan more than just individual files
        #--project=${CMAKE_BINARY_DIR}/compile_commands.json
        --cppcheck-build-dir=${CMAKE_BINARY_DIR}/cppcheck_output
        --checkers-report=${CMAKE_BINARY_DIR}/cppcheck_output/checkers-report
        --suppress=missingIncludeSystem
        --enable=all
        --inline-suppr
        --inconclusive
        )
        # To exclude from check use --i <dir or file>
        # e.g. -i ${CMAKE_SOURCE_DIR}/imgui/lib)
  else()
    message(SEND_ERROR "cppcheck requested but executable not found")
  endif()
endif()

if(ENABLE_CLANG_TIDY)
  find_program(CLANGTIDY clang-tidy)
  if(CLANGTIDY)
      set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY} -p ${CMAKE_BINARY_DIR}/ -extra-arg=-Wno-unknown-warning-option)
      set(CMAKE_CXX_CLANG_TIDY_EXPORT_FIXES_DIR ${CMAKE_BINARY_DIR}/clang-tidy-fixes)
  else()
    message(SEND_ERROR "clang-tidy requested but executable not found")
  endif()
endif()

if(ENABLE_INCLUDE_WHAT_YOU_USE)
  find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
  if(INCLUDE_WHAT_YOU_USE)
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
  else()
    message(SEND_ERROR "include-what-you-use requested but executable not found")
  endif()
endif()
