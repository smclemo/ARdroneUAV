cmake_minimum_required(VERSION 2.6)

include ( ../ardrone2_toolchain.cmake )

link_libraries(m)
link_libraries(pthread)

add_executable(horizontal_velocities
	horizontal_velocities.c
	main_horizontal_velocities.c
	../util/util.c
	../video/video.c
	../video/blocksum.c
	../attitude/attitude.c
	../attitude/ars.c
	../navboard/navboard.c
	../gpio/gpio.c
)

	

