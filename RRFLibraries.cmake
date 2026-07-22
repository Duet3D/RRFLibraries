# RRFLibraries as a reusable CMake component. See lib/FreeRTOS/FreeRTOS.cmake for the rationale
# behind the include()-a-module-and-call-a-function pattern.
#
#   rrflibraries_add_library(
#       TARGET <name>
#       MCU    <SAME70|SAME51|SAMC21>
#       ARCH   <interface target>
#       [RTOS] # build the RTOS variant (adds -DRTOS and the FreeRTOS include paths)
#   )

set(RRFLIBRARIES_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(RRFLIBRARIES_LIBRARY_FLAGS
    "RTOS"
)
set(RRFLIBRARIES_LIBRARY_ARGS
    "FREERTOS_INTERFACE"        # interface target for FreeRTOS, if RTOS is enabled
)

include("${LIBRARIES_DIR}/LibraryUtils.cmake")

function(rrflibraries_add_interface OUT_TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${RRFLIBRARIES_LIBRARY_FLAGS}" "${DEFAULT_INTERFACE_ARGS}" "")
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "rrflibraries_add_interface: unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    get_enabled_features(_enabled_features)
    make_library_name(_target "RRFLibraries" INTERFACE ${ARG_MCU} ${_enabled_features})
    set(${OUT_TARGET} "${_target}" PARENT_SCOPE)
    if(TARGET ${_target})
        return()  # already built for this MCU and feature set
    endif()

    add_library(${_target} INTERFACE)
    target_include_directories(${_target} INTERFACE "${RRFLIBRARIES_DIR}/src")
endfunction()

function(rrflibraries_add_library OUT_TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${RRFLIBRARIES_LIBRARY_FLAGS}" "${DEFAULT_LIBRARY_ARGS};${RRFLIBRARIES_LIBRARY_ARGS}" "")
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "rrflibraries_add_library: unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    get_enabled_features(_enabled_features)
    make_library_name(_target "RRFLibraries" STATIC ${ARG_MCU} ${_enabled_features})
    set(${OUT_TARGET} "${_target}" PARENT_SCOPE)
    if(TARGET ${_target})
        return()  # already built for this MCU and feature set
    endif()

    set(_src "${RRFLIBRARIES_DIR}/src")
    set(_freertos "${RRFLIBRARIES_DIR}/../FreeRTOS/src")

    if(ARG_MCU STREQUAL "SAME70")
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

    target_compile_options(${_target} PRIVATE
        -ffunction-sections
        -fdata-sections
        -nostdlib
        -fno-threadsafe-statics
        -fno-rtti
        -fno-exceptions
        -Wall
        -Wundef
        -Wdouble-promotion
        -Werror=return-type
        -Werror
        -Wnoexcept
        -Wshadow
        -Wsign-promo
        -fsingle-precision-constant
        $<$<NOT:$<CONFIG:Debug>>:-O2>
        $<$<CONFIG:Debug>:-Og;-g3>)

    target_link_libraries(${_target} PRIVATE ${ARG_ARCH})
endfunction()
