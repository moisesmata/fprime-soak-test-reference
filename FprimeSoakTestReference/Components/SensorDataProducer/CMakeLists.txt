register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/SensorDataProducer.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/SensorDataProducer.cpp"
)

### UTs ###
register_fprime_ut(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/SensorDataProducer.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/SensorDataProducerTestMain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/SensorDataProducerTester.cpp"
    UT_AUTO_HELPERS
)
