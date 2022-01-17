cmake_minimum_required(VERSION 3.10)
# cmake_minimum_required(VERSION 3.16)
project(Comm)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-std=c++17 -pthread)

option(WITH_OPENCV "Build the code with opencv" OFF)
set(OPENCV_VERSION "" CACHE STRING "The opencv version to use in the project")

if (WIN32)
    set(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} wsock32 ws2_32)
else (WIN32)
    set(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} pthread)
endif (WIN32)
set(MASTER_LIBRARIES ${EXTERNAL_LIBRARIES})

if (WITH_OPENCV)
    if (WIN32)
        set(OpenCV_DIR "C:/Program Files (x86)/opencv/opencv_4.4.0")
    endif (WIN32)

    if ("${OPENCV_VERSION}" STREQUAL "")
        find_package(OpenCV REQUIRED COMPONENTS highgui)
    else ()
        find_package(OpenCV ${OPENCV_VERSION} REQUIRED COMPONENTS highgui)
    endif ()

    include_directories(${OpenCV_INCLUDE_DIRS})
endif ()

include_directories(include)

set(UTILS_STUFF src/utils/NetworkIncludes.cpp src/utils/Buffer.cpp src/utils/SerializationHeader.cpp include/comm/utils/utils.hpp src/utils/NetworkData.cpp)
set(SOCKET_STUFF src/socket/SocketPartner.cpp src/socket/SocketType.cpp src/socket/Socket.cpp src/socket/utils.cpp)
set(COMMUNICATION_STUFF src/communication/Communication.cpp src/communication/TCPServer.cpp src/communication/Communicator.cpp)
set(DATA_STUFF src/data/MessageType.cpp src/data/messages.cpp src/data/CommunicationData.cpp src/data/CoordinateData.cpp src/data/ImageData.cpp src/data/StatusData.cpp src/data/dataUtils.cpp src/data/CommunicatorState.cpp src/data/DataCollection.cpp src/data/BytesData.cpp src/data/ImageEncodeData.cpp)

set(OPENCV_STUFF src/data/dataUtilsWithOpenCV.cpp src/data/DataCollectionWithOpenCV.cpp src/data/ImageDataWithOpenCV.cpp src/data/ImageEncodeDataWithOpenCV.cpp)

set(COMM_LIB_SHARED ${PROJECT_TARGET})
add_library(${COMM_LIB_SHARED} SHARED ${COMMUNICATION_STUFF} ${DATA_STUFF} ${SOCKET_STUFF} ${UTILS_STUFF})
target_link_libraries(${COMM_LIB_SHARED} ${MASTER_LIBRARIES})
target_include_directories(${COMM_LIB_SHARED}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

set(COMM_LIB "${PROJECT_TARGET}_static")
add_library(${COMM_LIB} ${COMMUNICATION_STUFF} ${DATA_STUFF} ${SOCKET_STUFF} ${UTILS_STUFF})
target_link_libraries(${COMM_LIB} ${MASTER_LIBRARIES})
target_include_directories(${COMM_LIB}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

if (WITH_OPENCV)
    set(OPENCV_COMM_LIB_SHARED ${PROJECT_TARGET}_OpenCV)
    add_library(${OPENCV_COMM_LIB_SHARED} SHARED ${OPENCV_STUFF})
    target_link_libraries(${OPENCV_COMM_LIB_SHARED} ${PROJECT_TARGET} ${OpenCV_LIBS} ${MASTER_LIBRARIES})
    target_include_directories(${OPENCV_COMM_LIB_SHARED}
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
            )

    set(OPENCV_COMM_LIB "${PROJECT_TARGET}_OpenCV_static")
    add_library(${OPENCV_COMM_LIB} ${OPENCV_STUFF})
    target_link_libraries(${OPENCV_COMM_LIB} ${PROJECT_TARGET}_static ${OpenCV_LIBS} ${MASTER_LIBRARIES})
    target_include_directories(${OPENCV_COMM_LIB}
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
            )
endif ()

set(MASTER_LIBRARIES ${COMM_LIB} ${OPENCV_COMM_LIB} ${MASTER_LIBRARIES})

add_subdirectory(tests)
