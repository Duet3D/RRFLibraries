# RRFLibraries as a reusable CMake component. See lib/FreeRTOS/FreeRTOS.cmake for the rationale
# behind the include()-a-module-and-call-a-function pattern.
#
#   rrflibraries_add_library(
#       TARGET <name>
#       MCU    <SAME70|SAME51|SAMC21|HOST>
#       ARCH   <interface target>
#       [RTOS] # build the RTOS variant (adds -DRTOS and the FreeRTOS include paths)
#       [EXTRA_COMPILE_OPTIONS <opts...>]
#   )
#
# MCU HOST builds for an ordinary hosted target (Linux) rather than a Duet MCU. It exists for the
# SBC-side code, which reuses the portable parts of this library - Bitmap, StringRef, SimpleMath,
# Isqrt and the eCv annotation headers - and must not pull in anything freestanding. Differences
# from an MCU build:
#   - RTOSIface/ is excluded; it needs FreeRTOS, and a hosted consumer brings its own threading.
#   - the freestanding-only compile options (-nostdlib, -fsingle-precision-constant) are dropped,
#     as is -Werror: this is upstream source built for a target it was not written for, so a new
#     warning here must not break a downstream project's build.
#   - the objects are position-independent, because they get linked into a shared library.
#
# EXTRA_COMPILE_OPTIONS is for options the consumer must impose on this library's own translation
# units, not just on its usage requirements - a forced include being the motivating case (see
# float16_t in DuetSbcInterface/src/Compat).

set(RRFLIBRARIES_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(RRFLIBRARIES_LIBRARY_FLAGS
    "RTOS"
)
set(RRFLIBRARIES_LIBRARY_ARGS
    "FREERTOS_INTERFACE"        # interface target for FreeRTOS, if RTOS is enabled
)
set(RRFLIBRARIES_LIBRARY_MULTI_ARGS
    "EXTRA_COMPILE_OPTIONS"     # options applied to this library's own sources
)

include("${LIBRARIES_DIR}/LibraryUtils.cmake")

function(rrflibraries_add_interface OUT_TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${RRFLIBRARIES_LIBRARY_FLAGS}" "${DEFAULT_INTERFACE_ARGS}" "")
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "rrflibraries_add_interface: unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    get_enabled_features(_enabled_features ${RRFLIBRARIES_LIBRARY_FLAGS})
    make_library_name(_target "RRFLibraries" INTERFACE ${ARG_MCU} ${_enabled_features})
    set(${OUT_TARGET} "${_target}" PARENT_SCOPE)
    if(TARGET ${_target})
        return()  # already built for this MCU and feature set
    endif()

    add_library(${_target} INTERFACE)
    target_include_directories(${_target} INTERFACE "${RRFLIBRARIES_DIR}/src")
endfunction()

function(rrflibraries_add_library OUT_TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${RRFLIBRARIES_LIBRARY_FLAGS}" "${DEFAULT_LIBRARY_ARGS};${RRFLIBRARIES_LIBRARY_ARGS}" "${RRFLIBRARIES_LIBRARY_MULTI_ARGS}")
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "rrflibraries_add_library: unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    get_enabled_features(_enabled_features ${RRFLIBRARIES_LIBRARY_FLAGS})
    make_library_name(_target "RRFLibraries" STATIC ${ARG_MCU} ${_enabled_features})
    set(${OUT_TARGET} "${_target}" PARENT_SCOPE)
    if(TARGET ${_target})
        return()  # already built for this MCU and feature set
    endif()

    set(_src "${RRFLIBRARIES_DIR}/src")
    set(_freertos "${RRFLIBRARIES_DIR}/../FreeRTOS/src")

    if(ARG_MCU STREQUAL "HOST")
        # RTOSIface is the FreeRTOS shim; a hosted consumer supplies its own synchronisation.
        set(_excludes "/RP2040/" "/SAME5x_C21/" "/RTOSIface/")
    elseif(ARG_MCU STREQUAL "SAME70")
        set(_excludes "/RP2040/" "/SAME5x_C21/")
    elseif(ARG_MCU STREQUAL "SAME51")
        set(_excludes "/RP2040/")
    elseif(ARG_MCU STREQUAL "SAMC21")
        set(_excludes "/RP2040/")
    else()
        message(FATAL_ERROR "rrflibraries_add_library: unsupported MCU '${ARG_MCU}'")
    endif()

    file(GLOB_RECURSE _srcs CONFIGURE_DEPENDS "${_src}/*.cpp")
    foreach(_ex IN LISTS _excludes)
        list(FILTER _srcs EXCLUDE REGEX "${_ex}")
    endforeach()

    add_library(${_target} STATIC ${_srcs})

    target_link_libraries(${_target} PUBLIC I_${_target}) # link own interface target

    if(ARG_RTOS)
        target_link_libraries(${_target} PRIVATE ${ARG_FREERTOS_INTERFACE})
    endif()

    # Options that only make sense freestanding, or that turn an upstream warning into a downstream
    # build failure. A HOST build is this source compiled for a target it was not written for, so it
    # opts out of both groups.
    #
    # -fsingle-precision-constant is deliberately NOT in this group: it decides whether a literal
    # like 0.0 is float or double, so dropping it would silently change the results of the motion
    # maths relative to the firmware. It stays PRIVATE to this library's own sources either way.
    set(_freestanding_options
        -nostdlib
        -Werror)
    if(ARG_MCU STREQUAL "HOST")
        set(_freestanding_options)
        # C++20 for this library's own sources (SafeVsnprintf.cpp uses std::bit_cast), which is what
        # the firmware projects already compile it as. PRIVATE, so a C++17 consumer stays C++17 -
        # nothing in the headers needs more than C++17.
        set_target_properties(${_target} PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON)
    endif()

    target_compile_options(${_target} PRIVATE
        -ffunction-sections
        -fdata-sections
        -fno-threadsafe-statics
        -fno-rtti
        -fno-exceptions
        -Wall
        -Wundef
        -Wdouble-promotion
        -Werror=return-type
        -Wnoexcept
        -Wshadow
        -Wsign-promo
        -fsingle-precision-constant
        ${_freestanding_options}
        ${ARG_EXTRA_COMPILE_OPTIONS}
        $<$<NOT:$<CONFIG:Debug>>:-O2>
        $<$<CONFIG:Debug>:-Og;-g3>)

    target_link_libraries(${_target} PRIVATE ${ARG_ARCH})
endfunction()
