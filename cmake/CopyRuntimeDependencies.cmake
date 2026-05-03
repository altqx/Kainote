if(NOT DEFINED KAINOTE_EXE OR NOT DEFINED RUNTIME_DIR)
    message(FATAL_ERROR "KAINOTE_EXE and RUNTIME_DIR are required")
endif()

file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES "${KAINOTE_EXE}"
    RESOLVED_DEPENDENCIES_VAR resolved_deps
    UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
)

if(unresolved_deps)
    message(WARNING "Unresolved runtime dependencies: ${unresolved_deps}")
endif()

# Build a self-contained Linux runtime directory.  Earlier allowlist-based
# bundling fixed direct deps such as Lua/ICU but missed transitive deps such as
# libass -> libunibreak and FFmpeg codec/network helpers.  Copy every resolved
# shared library except the fundamental glibc/ELF loader set that must come from
# the target system.  The executable is linked with $ORIGIN RPATH, so these
# copied libraries are preferred when launching ./kainote from the build/package
# directory.
set(system_dep_regex
    "^(ld-linux.*|linux-vdso.*|lib(c|m|dl|pthread|rt|resolv|nsl|util|anl)\\.so.*)$"
)

set(copied_count 0)
foreach(dep IN LISTS resolved_deps)
    get_filename_component(dep_name "${dep}" NAME)
    if(NOT dep_name MATCHES "${system_dep_regex}")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${dep}" "${RUNTIME_DIR}/${dep_name}"
            RESULT_VARIABLE copy_result
        )
        if(NOT copy_result EQUAL 0)
            message(FATAL_ERROR "Failed to copy runtime dependency ${dep}")
        endif()
        math(EXPR copied_count "${copied_count} + 1")
    endif()
endforeach()

message(STATUS "Copied ${copied_count} bundled runtime dependencies to ${RUNTIME_DIR}")
