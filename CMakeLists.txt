# RRFLibraries for the Duet3D firmware, built for DUET_MCU, which the consuming project's toolchain
# file sets along with the core's code generation flags.
#
#   DUET_MCU                    SAME70, SAME5x, SAMC21, STM32H5, or HOST for an ordinary hosted
#                               (Linux) build
#   DUET_PART_DEFINE            the macro naming the MCU part, e.g. __SAME70Q20B__ (none for HOST)
#   DUET_RTL_DUMPS              write each source's RTL (-fdump-rtl-expand), for call graph and
#                               stack analysis; ccache cannot cache these compiles
#   DUET_RTOS                   build the RTOS interface against Duet::FreeRTOS, which must already
#                               exist
#   RRFLIBRARIES_PRINTF_FLOAT   SafeVsnprintf formats floating point numbers (default ON)
#   RRFLIBRARIES_OPTIMISATION   Release optimisation for this library, if not the build type's
#
# Target: Duet::RRFLibraries. On SAMC21 it also needs Duet::CoreN2G, for the DIVAS square root.
#
# HOST exists for the SBC-side code, which reuses the portable parts of this library - Bitmap,
# StringRef, SimpleMath, Isqrt and the eCv annotation headers. It leaves out RTOSIface (a hosted
# consumer brings its own threading), builds position-independent objects for linking into a
# shared library, and does not apply -Werror: this is source built for a target it was not written
# for, so a new warning must not break a downstream build.
#
# A consumer that must impose options on this library's own sources - a forced include, say - can
# add them with target_compile_options(RRFLibraries PRIVATE ...).

cmake_minimum_required(VERSION 3.25)
project(RRFLibraries LANGUAGES CXX)

option(RRFLIBRARIES_PRINTF_FLOAT "Build SafeVsnprintf with floating point support" ON)

set(_options)
set(_werror -Werror)
if(DUET_MCU STREQUAL "HOST")
    set(_werror)
elseif(DUET_MCU STREQUAL "STM32H5")
    set(_options -mno-unaligned-access)
elseif(NOT DUET_MCU MATCHES "^(SAME70|SAME5x|SAMC21)$")
    message(FATAL_ERROR "RRFLibraries: unsupported DUET_MCU '${DUET_MCU}'")
endif()

file(GLOB_RECURSE _srcs CONFIGURE_DEPENDS src/*.cpp)
if(DUET_MCU STREQUAL "HOST")
    list(FILTER _srcs EXCLUDE REGEX "/src/RTOSIface/")
endif()

add_library(RRFLibraries STATIC ${_srcs})
add_library(Duet::RRFLibraries ALIAS RRFLibraries)

# C++20 for this library's own sources (SafeVsnprintf.cpp uses std::bit_cast); the headers need no
# more than C++17, so a C++17 consumer stays C++17.
set_target_properties(RRFLibraries PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

target_include_directories(RRFLibraries PUBLIC src)
target_compile_definitions(RRFLibraries PRIVATE
    ${DUET_PART_DEFINE}
    $<$<NOT:$<BOOL:${RRFLIBRARIES_PRINTF_FLOAT}>>:NO_PRINTF_FLOAT>)

if(DUET_RTOS)
    target_link_libraries(RRFLibraries PUBLIC Duet::FreeRTOS)
endif()
if(DUET_MCU STREQUAL "SAMC21")
    target_link_libraries(RRFLibraries PRIVATE Duet::CoreN2G)
endif()

target_compile_options(RRFLibraries PRIVATE
    $<$<CONFIG:Release>:${RRFLIBRARIES_OPTIMISATION}>
    ${_options}
    -fno-math-errno
    # Decides whether a literal like 0.0 is float or double, so a HOST build keeps it to compute
    # the same results as the firmware
    -fsingle-precision-constant
    -fno-threadsafe-statics
    -fno-rtti
    -fno-exceptions
    -Wall
    -Wundef
    -Wdouble-promotion
    -Wnoexcept
    -Wshadow
    -Wsign-promo
    ${_werror}
    -fstack-usage
    $<$<BOOL:${DUET_RTL_DUMPS}>:-fdump-rtl-expand>
    # __FILE__ relative to the library root, so the image does not depend on the checkout location
    -fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/=)

if(DUET_MCU STREQUAL "HOST")
    set_target_properties(RRFLibraries PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
