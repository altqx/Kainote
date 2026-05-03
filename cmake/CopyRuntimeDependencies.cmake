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

# Bundle version-sensitive libraries that are commonly absent or have different
# SONAMEs on other Linux installations.  Leave core libc/desktop/driver stacks
# to the host system to avoid overriding system GTK/OpenGL/X11 components.
set(bundle_dep_regex
    "^(lib(lua5\\.1|icu.*|hunspell.*|uchardet.*|ffms2.*|ass.*|av(format|codec|util|filter|device).*|sw(resample|scale).*|postproc.*|wx_.*3\\.2.*|wx_base.*3\\.2.*|curl.*|ssl.*|crypto.*|nghttp2.*|idn2.*|ssh2.*|psl.*|unistring.*|zstd.*|lzma.*|bz2.*|xml2.*|fontconfig.*|freetype.*|harfbuzz.*|fribidi.*|png16.*|jpeg.*|openjp2.*|webp.*|vpx.*|x264.*|x265.*|dav1d.*|aom.*|Svt.*|jxl.*|brotli.*|ogg.*|vorbis.*|opus.*|mp3lame.*|soxr.*|theora.*|speex.*|gsm.*|bluray.*|udfread.*))\\.so"
)

set(copied_count 0)
foreach(dep IN LISTS resolved_deps)
    get_filename_component(dep_name "${dep}" NAME)
    if(dep_name MATCHES "${bundle_dep_regex}")
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
