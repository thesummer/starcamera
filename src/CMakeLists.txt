
# Use all .cpp-files for starcamera project
FILE(GLOB starcamera_SRC *.cpp)
FILE(GLOB starcamera_SRC *.c)
# Build starcamera
add_executable(starcamera ${starcamera_SRC} ${starcamera_HEADER})
#link_directories("/usr/local/lib")
target_link_libraries(starcamera gpulabeling opencv_core opencv_highgui
                          opencv_imgproc opencv_features2d opencv_calib3d
                          dl rt)

set_target_properties(starcamera PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

