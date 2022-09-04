cmake_minimum_required (VERSION 3.8)

project ("Dumb 4-bit PCM Converter")

option (FORCE_STATIC_LINKING "Force static linking in case it's not the compiler's default" OFF)

set (SRCS
	main.cpp
)

add_executable (DUMB4BITPCM ${SRCS})

if (FORCE_STATIC_LINKING)
	if (MSVC)
		message ("TODO: Implement forced static linking on MSVC")
	else ()
		target_link_libraries(DUMB4BITPCM PUBLIC "-static")
	endif ()
endif (FORCE_STATIC_LINKING)

install (TARGETS DUMB4BITPCM RUNTIME DESTINATION "bin")
