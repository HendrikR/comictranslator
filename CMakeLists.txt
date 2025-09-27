cmake_minimum_required(VERSION 4.1)
project (comictranslator)
set(CMAKE_CXX_STANDARD 17) # needed for qt6

set(SOURCES_COMMON bubble.cpp bubble_rectangle.cpp bubble_ellipse.cpp comicfile.cpp cfont.cpp color.cpp)
file(GLOB SOURCES *.cpp)
list(APPEND SOURCES "jute/jute.cpp")
list(FILTER SOURCES EXCLUDE REGEX "(detect.cpp|draw.cpp|edit.cpp|gui_fltk.cpp)")

# message(STATUS "    these are all the files: ${SOURCES}")

find_package(OpenCV REQUIRED core imgproc highgui )
find_package(EXPAT REQUIRED)
find_package(FLTK REQUIRED)
# find_package(Imlib2 REQUIRED) # imlib2 does not provide cmake files, include it manually
find_library(Imlib2_LIBRARIES NAMES Imlib2 PATHS /usr/local/lib /usr/lib)
include_directories(/usr/local/include /usr/include)

set(ALL_LIBRARIES
    ${OpenCV_LIBRARIES}
    ${Imlib2_LIBRARIES}
    ${FLTK_LIBRARIES}
    ${EXPAT_LIBRARIES}
    m  # C math library
)

set(ALL_INCLUDE_DIRS
    ${OpenCV_INCLUDE_DIRS}
    ${Imlib2_INCLUDE_DIRS}
    ${FLTK_INCLUDE_DIRS}
    ${EXPAT_INCLUDE_DIRS}
)

# Debug output for OpenCV variables
message(STATUS "    these are all the libs: ${ALL_LIBRARIES}")

ADD_EXECUTABLE( detect detect.cpp )
TARGET_SOURCES( detect PRIVATE ${COMMON_SOURCES} detect.cpp )
TARGET_INCLUDE_DIRECTORIES( detect PRIVATE ${ALL_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES( detect PRIVATE ${ALL_LIBRARIES} )

ADD_EXECUTABLE( draw draw.cpp )
TARGET_SOURCES( draw PRIVATE draw.cpp ${SOURCES_COMMON} )
TARGET_INCLUDE_DIRECTORIES( draw PRIVATE ${ALL_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES( draw PRIVATE ${ALL_LIBRARIES} )

ADD_EXECUTABLE( edit edit.cpp )
TARGET_SOURCES( edit PRIVATE edit.cpp ${SOURCES} )
TARGET_INCLUDE_DIRECTORIES( edit PRIVATE ${ALL_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES( edit PRIVATE ${ALL_LIBRARIES} )
