cmake_minimum_required( VERSION 3.11 )

project( librrf )

# skip compiler build tests
set(CMAKE_C_COMPILER_FORCED True)
set(CMAKE_CXX_COMPILER_FORCED True)

add_library(rrf STATIC
	src/General/IP4String.cpp
	src/General/IPAddress.cpp
	src/General/NamedEnum.cpp
	src/General/NumericConverter.cpp
	src/General/SafeStrtod.cpp
	src/General/SafeVsnprintf.cpp
	src/General/StringBuffer.cpp
	src/General/StringFunctions.cpp
	src/General/StringRef.cpp
	src/General/Strnlen.cpp
	src/Math/Deviation.cpp
	src/Math/DeviationAccumulator.cpp
	src/Math/Isqrt.cpp
	src/RTOSIface/RTOSIface.cpp
)

target_include_directories(rrf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
