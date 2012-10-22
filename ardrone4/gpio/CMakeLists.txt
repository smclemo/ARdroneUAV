cmake_minimum_required(VERSION 2.6)

include ( ../ardrone2_toolchain.cmake )

link_libraries(m)

add_executable(gpio
	../util/util.c
	gpio.c
	main_gpio.c
)

	

