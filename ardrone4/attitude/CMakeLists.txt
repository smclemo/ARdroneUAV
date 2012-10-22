cmake_minimum_required(VERSION 2.6)

include (../ardrone2_toolchain.cmake )


link_libraries(m)

add_executable(attitude
	attitude.c
	main_attitude.c
	../util/util.c
	../gpio/gpio.c
	ars.c
	../navboard/navboard.c
)

	

