cmake_minimum_required (VERSION 2.8)
project (ColorCounter)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
elseif(MSVC)
    #C++11 is enabled by default on Visual Studio compilers that support it
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

find_package(wxWidgets REQUIRED core base)

include(${wxWidgets_USE_FILE})
include_directories(${wxWidgets_INCLUDE_DIRS})
link_directories(${wxWidgets_LIBRARY_DIRS})

set(SOURCE_FILES 
    src/Main.cpp 
    src/MainWindow.cpp
    src/ImagePanel.cpp
    src/ImageProcessing.cpp
    src/Histogram.cpp
    src/HistogramPlotter.cpp
    src/HueProcessor.cpp
    src/ColorPercent.cpp
    src/ColorListBox.cpp)

set(HEADER_FILES 
    src/Main.h 
    src/MainWindow.h
    src/ImagePanel.h
    src/ImageProcessing.h
    src/Histogram.h
    src/HistogramPlotter.h
    src/HueProcessor.h
    src/ColorPercent.h
    src/ColorListBox.h)

add_executable(ColorCounter ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(ColorCounter ${wxWidgets_LIBRARIES})

if(WIN32)
    if(MSVC)
        set_target_properties(ColorCounter PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    elseif(CMAKE_COMPILER_IS_GNUCXX)
        set_target_properties(ColorCounter PROPERTIES LINK_FLAGS "--subsystem,windows")        
    endif()
endif()
