
# Use all .cpp-files for starcamera project
FILE(GLOB starcamera_SRC *.cpp)
# Build starcamera
add_executable(starcamera ${starcamera_SRC} ${starcamera_HEADER})
link_directories("/usr/local/lib")
target_link_libraries(starcamera opencv_core opencv_highgui
                      opencv_imgproc opencv_features2d opencv_calib3d
                      sqlite3 usb-1.0 midlib2 apbase python3.3m)
set_target_properties(starcamera PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

