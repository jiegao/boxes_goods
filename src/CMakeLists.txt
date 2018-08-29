ADD_EXECUTABLE(checkBoxesGoods main.cc)
ADD_LIBRARY(libchecker boxes_goods_checker.cc)
TARGET_LINK_LIBRARIES(checkBoxesGoods libchecker)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
