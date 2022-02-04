cmake_minimum_required(VERSION 3.20)

set(BOARD nrf52840dk_nrf52840)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(livelock_repro_app)

target_sources(app PRIVATE src/main.c)
