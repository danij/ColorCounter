cmake_minimum_required (VERSION 2.8)
project (ColorCounter)

find_package(wxWidgets REQUIRED core base)

include(${wxWidgets_USE_FILE})
include_directories(${wxWidgets_INCLUDE_DIRS})
link_directories(${wxWidgets_LIBRARY_DIRS})

set(SOURCE_FILES 
    Main.cpp 
    MainWindow.cpp
    ImagePanel.cpp
    ImageProcessing.cpp
    Histogram.cpp
    HueProcessor.cpp
    ColorPercent.cpp
    ColorListBox.cpp)

set(HEADER_FILES 
    Main.h 
    MainWindow.h
    ImagePanel.h
    ImageProcessing.h
    Histogram.h
    HueProcessor.h
    ColorPercent.h
    ColorListBox.h)

add_executable(ColorCounter ${SOURCE_FILES} ${HEADER_FILES})
    
target_link_libraries(ColorCounter ${wxWidgets_LIBRARIES})