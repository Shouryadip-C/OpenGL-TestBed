set(
  STB_IMAGE_SOURCE_FILES
  stb_image.cpp
)

set(
  STB_IMAGE_HEADER_FILES
  stb_image.h
  stb_image_write.h
)

add_library(stb_image STATIC ${STB_IMAGE_SOURCE_FILES} ${STB_IMAGE_HEADER_FILES})
target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
