cmake_minimum_required(VERSION 3.5.1)
project( YGO )
find_package( OpenCV REQUIRED )
add_executable( YGO YGO_real_final_3.cpp )
target_link_libraries( YGO libpthread.so ${OpenCV_LIBS} )




