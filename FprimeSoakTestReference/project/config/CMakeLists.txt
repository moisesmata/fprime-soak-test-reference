####
# FprimeSoakTestReference/project/config/CMakeLists.txt:
#
# Configuration overrides for F Prime subtopologies and components.
# This enables TlmPacketizer instead of TlmChan for the CdhCore subtopology, and
# raises RateGroupDriverRateGroupPorts from 3 to 4 (via AcConstants.fpp) so the
# deployment can run four rate groups: 1kHz, 100Hz, 10Hz, and 1Hz.
####
register_fprime_config(
    CONFIGURATION_OVERRIDES
        "${CMAKE_CURRENT_LIST_DIR}/CdhCoreTlmConfig.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/AcConstants.fpp"
    INTERFACE
)
