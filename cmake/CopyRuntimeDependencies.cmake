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

set(pulse_private_dir "")
foreach(dep IN LISTS resolved_deps)
    if(dep MATCHES "/pulseaudio/[^/]+$")
        get_filename_component(pulse_private_dir "${dep}" DIRECTORY)
        break()
    endif()
endforeach()

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

        # libpulse.so carries an absolute RUNPATH to PulseAudio's private helper
        # directory.  If a user runs the copied bundle on a system without that
        # exact distro path, the dynamic loader ignores the helper that we copied
        # beside ./kainote and fails at startup.  Repoint only libpulse's copied
        # RUNPATH with CMake's built-in ELF editor so the bundle does not depend
        # on patchelf being installed on the build host.
        if(pulse_private_dir AND dep_name MATCHES "^libpulse\\.so")
            file(RPATH_CHANGE
                FILE "${copied_dep}"
                OLD_RPATH "${pulse_private_dir}"
                NEW_RPATH "$ORIGIN:$ORIGIN/pulseaudio"
            )
        endif()

        math(EXPR copied_count "${copied_count} + 1")
    endif()
endforeach()

message(STATUS "Copied ${copied_count} bundled runtime dependencies to ${RUNTIME_DIR}")
