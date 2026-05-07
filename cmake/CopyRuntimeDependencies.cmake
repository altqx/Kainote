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

find_program(PATCHELF_EXECUTABLE patchelf)

set(copied_count 0)
foreach(dep IN LISTS resolved_deps)
    get_filename_component(dep_name "${dep}" NAME)
    if(NOT dep_name MATCHES "${system_dep_regex}")
        set(copied_dep "${RUNTIME_DIR}/${dep_name}")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${dep}" "${copied_dep}"
            RESULT_VARIABLE copy_result
        )
        if(NOT copy_result EQUAL 0)
            message(FATAL_ERROR "Failed to copy runtime dependency ${dep}")
        endif()

        if(dep MATCHES "/pulseaudio/[^/]+$")
            execute_process(
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${RUNTIME_DIR}/pulseaudio"
                RESULT_VARIABLE pulse_mkdir_result
            )
            if(NOT pulse_mkdir_result EQUAL 0)
                message(FATAL_ERROR "Failed to create ${RUNTIME_DIR}/pulseaudio")
            endif()
            execute_process(
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                        "${dep}" "${RUNTIME_DIR}/pulseaudio/${dep_name}"
                RESULT_VARIABLE pulse_copy_result
            )
            if(NOT pulse_copy_result EQUAL 0)
                message(FATAL_ERROR "Failed to copy PulseAudio runtime dependency ${dep}")
            endif()
        endif()

        # Transitive system libraries such as libpulse.so often depend on
        # private/helper libraries (for example libpulsecommon-16.1.so) that live
        # outside the normal library search directories.  Kainote's executable
        # uses $ORIGIN RPATH, but copied shared libraries may still lack their own
        # origin-relative search path when users run the bundle from another
        # directory.  Stamp copied ELF libraries as well so each bundled .so can
        # resolve its siblings next to ./kainote.
        if(PATCHELF_EXECUTABLE AND dep_name MATCHES "\\.so(\\.|$)")
            execute_process(
                COMMAND "${PATCHELF_EXECUTABLE}" --force-rpath --set-rpath "$ORIGIN" "${copied_dep}"
                RESULT_VARIABLE patchelf_result
                ERROR_QUIET
            )
            if(NOT patchelf_result EQUAL 0)
                message(WARNING "Failed to set $ORIGIN RPATH on ${copied_dep}")
            endif()
        endif()

        math(EXPR copied_count "${copied_count} + 1")
    endif()
endforeach()

if(NOT PATCHELF_EXECUTABLE)
    message(WARNING "patchelf was not found; copied runtime libraries keep their original RPATH/RUNPATH")
endif()

message(STATUS "Copied ${copied_count} bundled runtime dependencies to ${RUNTIME_DIR}")
