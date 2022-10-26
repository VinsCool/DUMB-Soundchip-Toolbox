cmake_minimum_required (VERSION 3.8)

project ("DUMB POKEY Toolbox") 

option (FORCE_STATIC_LINKING "Force static linking in case it's not the compiler's default" OFF)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY 
	${CMAKE_BINARY_DIR}/bin
) 

set (SRCS
	src/main.cpp
	#src/main.h 	# Might not be needed for now but we'll see 
	src/general.h	# I guess? Not sure how I will actually merge all these things together heh
	src/kbhit.cpp
	src/kbhit.h
	src/pcm.cpp	# Or something like that maybe 
	src/pcm.h
	src/tuning.cpp
	src/tuning.h
	
) 

add_executable (DUMB_POKEY_TOOLBOX ${SRCS}) 

if (FORCE_STATIC_LINKING)
	if (MSVC)
		message ("TODO: Implement forced static linking on MSVC")
	else ()
		target_link_libraries(DUMB_POKEY_TOOLBOX PUBLIC "-static")
	endif ()
endif (FORCE_STATIC_LINKING)

install (TARGETS DUMB_POKEY_TOOLBOX RUNTIME DESTINATION "bin")

