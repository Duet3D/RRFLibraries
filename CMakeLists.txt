cmake_minimum_required(VERSION 3.10)
project(RRFLibraries)

add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>")

set(srcs
    "src/RTOSIface/RTOSIface.cpp"
    "src/Math/Deviation.cpp"
    "src/Math/Isqrt.cpp"
    "src/General/IP4String.cpp"
    "src/General/IPAddress.cpp"
    "src/General/NamedEnum.cpp"
    "src/General/NumericConverter.cpp"
    "src/General/SafeStrtod.cpp"
    "src/General/StringBuffer.cpp"
    "src/General/StringFunctions.cpp"
    "src/General/SafeVsnprintf.cpp"
    "src/General/StringRef.cpp"
    "src/General/Strnlen.cpp"
)

add_library(RRFLibraries ${srcs})
target_include_directories(RRFLibraries PUBLIC "src")

target_link_libraries(RRFLibraries PRIVATE FreeRTOS)