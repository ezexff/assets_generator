cmake_minimum_required(VERSION 3.23.1 FATAL_ERROR)

project(AssetsGenerator)

ADD_SUBDIRECTORY(assimp)

add_executable(AssetsGenerator code/assets_generator.cpp code/assets_generator.h code/engine_types.h code/engine_intrinsics.h code/engine_math.h code/libs/stb/stb_image.h code/libs/stb/stb_image_write.h)

TARGET_LINK_LIBRARIES(AssetsGenerator assimp)
