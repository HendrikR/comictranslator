cmake_minimum_required(VERSION 4.1)
project (comictranslator)

add_compile_options(-g -std=c++11)

file(GLOB SOURCES *.cpp)
list(APPEND SOURCES "jute/jute.cpp")


find_package(OpenCV REQUIRED core imgproc highgui )
find_package(Expat REQUIRED)
find_package(FLTK REQUIRED)
# find_package(Imlib2 REQUIRED) # imlib2 does not provide cmake files, include it manually
find_library(IMLIB2_LIBRARIES NAMES Imlib2 PATHS /usr/local/lib /usr/lib)
include_directories(/usr/local/include /usr/include)

# TODO sometimes libs, sometimes libraries, what's up with that?
set(ALL_LIBS
    ${OpenCV_LIBS}
    ${IMLIB2_LIBRARIES}
    ${FLTK_LIBRARIES}
    ${EXPAT_LIBRARIES}
    m  # C math library
)

# TODO sometimes libs, sometimes libraries, what's up with that?
set(ALL_INCLUDE_DIRS
    ${OpenCV_INCLUDE_DIRS}
    ${IMLIB2_INCLUDE_DIRS}
    ${FLTK_INCLUDE_DIRS}
    ${EXPAT_INCLUDE_DIRS}
)

# Debug output for OpenCV variables
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")

ADD_EXECUTABLE( detect detect.cpp )
#TARGET_SOURCES( detect PRIVATE ${SOURCES} )
#TARGET_INCLUDE_DIRECTORIES( detect PRIVATE ${ALL_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES( detect PRIVATE ${ALL_LIBS} )

ADD_EXECUTABLE( draw
	draw.cpp
	jute/jute.cpp
)
TARGET_INCLUDE_DIRECTORIES(draw PRIVATE jute/)
TARGET_LINK_LIBRARIES( draw PRIVATE
	Imlib2
	expat
)

ADD_EXECUTABLE( gui
	gui_fltk.cpp
)

TARGET_LINK_LIBRARIES( gui
	fltk
	fltk_images
)
