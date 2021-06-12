cmake_minimum_required(VERSION 3.10)
# cmake_minimum_required(VERSION 3.16)
project(Comm)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-std=c++17 -pthread)

option(PERCEPTRON_BUILD "Build the code on Perceptron computer" OFF)
option(NUC_BUILD "Build the code on the NUC" ON)

option(WITH_OPENCV "Build the code with opencv" OFF)

include_directories(include)

if (WITH_OPENCV)
    if (NUC_BUILD)

    else (NUC_BUILD)
        if (WIN32)
            SET(OpenCV_DIR "C:/Program Files (x86)/opencv/opencv_4.4.0")
        endif (WIN32)
    endif (NUC_BUILD)

    if (PERCEPTRON_BUILD)
        FIND_PACKAGE(OpenCV 3.2 REQUIRED)
    else (PERCEPTRON_BUILD)
        FIND_PACKAGE(OpenCV REQUIRED)
    endif (PERCEPTRON_BUILD)

    include_directories(${OpenCV_INCLUDE_DIRS})

    set(EXTERNAL_LIBRARIES ${OpenCV_LIBS})
endif ()

if (WIN32)
    set(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} wsock32 ws2_32)
else (WIN32)
    set(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} pthread)
endif (WIN32)
set(MASTER_LIBRARIES ${EXTERNAL_LIBRARIES})

set(UTILS_STUFF src/utils/NetworkIncludes.cpp src/utils/Buffer.cpp src/utils/SerializationHeader.cpp include/comm/utils/utils.hpp src/utils/NetworkData.cpp)
set(SOCKET_STUFF src/socket/SocketPartner.cpp src/socket/SocketType.cpp src/socket/Socket.cpp src/socket/utils.cpp)
SET(COMMUNICATION_STUFF src/communication/Communication.cpp src/communication/TCPServer.cpp src/communication/Communicator.cpp)
SET(DATA_STUFF src/data/MessageType.cpp src/data/messages.cpp src/data/CommunicationData.cpp src/data/CoordinateData.cpp src/data/ImageData.cpp src/data/StatusData.cpp src/data/dataUtils.cpp src/data/CommunicatorState.cpp src/data/DataCollection.cpp src/data/BytesData.cpp src/data/ImageEncodeData.cpp)

set(COMM_LIB "Communication")
add_library(${COMM_LIB} SHARED ${COMMUNICATION_STUFF} ${DATA_STUFF} ${SOCKET_STUFF} ${UTILS_STUFF})
target_link_libraries(${COMM_LIB} ${MASTER_LIBRARIES})
if (WITH_OPENCV)
    # No idea what PUBLIC or PRIVATE does here...
    target_compile_definitions(${COMM_LIB} PUBLIC -DWITH_OPENCV)
endif ()

set(COMM_LIB "${COMM_LIB}_static")
add_library(${COMM_LIB} ${COMMUNICATION_STUFF} ${DATA_STUFF} ${SOCKET_STUFF} ${UTILS_STUFF})
target_link_libraries(${COMM_LIB} ${MASTER_LIBRARIES})
if (WITH_OPENCV)
    # No idea what PUBLIC or PRIVATE does here...
    target_compile_definitions(${COMM_LIB} PUBLIC -DWITH_OPENCV)
endif ()

set(MASTER_LIBRARIES ${COMM_LIB} ${MASTER_LIBRARIES})

add_subdirectory(tests)
