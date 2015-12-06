project (comictranslator)

add_compile_options("-g")

ADD_EXECUTABLE( detect
	detect.cpp
)
TARGET_LINK_LIBRARIES( detect
	opencv_core
	opencv_imgproc
	opencv_highgui
)

ADD_EXECUTABLE( draw
	draw.cpp
	comicfile.o
)
TARGET_LINK_LIBRARIES( draw
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
