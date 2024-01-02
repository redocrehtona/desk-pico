cmake_minimum_required(VERSION 3.12)

# Add source files to executable
add_executable(desk-pico
	main.cpp)

# Add include files from the 'include' directory
target_include_directories(desk-pico PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

# Link used libraries from pico-sdk and FreeRTOS-Kernel
target_link_libraries(desk-pico pico_stdlib hardware_i2c hardware_adc FreeRTOS-Kernel FreeRTOS-Kernel-Heap4)

# Set pico i/o states
pico_enable_stdio_usb(desk-pico 1)
pico_enable_stdio_uart(desk-pico 0)

# Create UF2 and other files
pico_add_extra_outputs(desk-pico)

