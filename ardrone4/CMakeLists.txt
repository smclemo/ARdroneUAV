cmake_minimum_required(VERSION 2.6)

include ( ardrone2_toolchain.cmake )

SUBDIRS(
    fly
    navboard
    vbat
    attitude
    motorboard
    udp
    horizontal_velocities
)
