set(EXE "TestTCPServer")
add_executable(${EXE} testTCPServer.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestUDPCommunication")
add_executable(${EXE} testUDPCommunication.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestNormalSockets")
add_executable(${EXE} testNormalSockets.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestBothCommunication")
add_executable(${EXE} testBothCommunication.cpp string_utils.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestEndianness")
add_executable(${EXE} testEndianness.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestTCPClientUDPServer")
add_executable(${EXE} testTCPClientUDPServer.cpp string_utils.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestSendImageUDP")
add_executable(${EXE} testSendImageUDP.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestSignal")
add_executable(${EXE} testSignals.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestAppSendUDPImage")
add_executable(${EXE} testAppSendUDPImage.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestEchoUDP")
add_executable(${EXE} testEchoUDP.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestSendAngles")
add_executable(${EXE} testSendAngles.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})

set(EXE "TestSendImage")
add_executable(${EXE} testSendImage.cpp)
target_link_libraries(${EXE} ${MASTER_LIBRARIES})
