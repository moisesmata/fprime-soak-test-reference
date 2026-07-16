register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/SensorDataApp.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/SensorDataApp.cpp"
)

### UTs ###
register_fprime_ut(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/SensorDataApp.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/SensorDataAppTestMain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/SensorDataAppTester.cpp"
    UT_AUTO_HELPERS
)
