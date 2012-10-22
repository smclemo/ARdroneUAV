cmake_minimum_required(VERSION 2.6)

include ( ../ardrone2_toolchain.cmake )

link_libraries(pthread)

add_executable(video
	video.c
	main_video.c
	../util/util.c
	blocksum.c
)

	

