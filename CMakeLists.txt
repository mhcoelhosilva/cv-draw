cmake_minimum_required(VERSION 2.8) 
PROJECT (fd)

SET(OpenCV_DIR "/Users/matheussilva/dev/other/build")

find_package(OpenCV REQUIRED )
set( NAME_SRC
    src/main.cpp    
     handy/Handy/BackgroundRemover.cpp
     handy/Handy/FaceDetector.cpp
     handy/Handy/FingerCount.cpp
     handy/Handy/SkinDetector.cpp
)

set( NAME_HEADERS       
     handy/Handy/BackgroundRemover.h
     handy/Handy/FaceDetector.h
     handy/Handy/FingerCount.h
     handy/Handy/SkinDetector.h
)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include )
link_directories( ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable( fd ${NAME_SRC} ${NAME_HEADERS} )

target_link_libraries( fd ${OpenCV_LIBS} )