####
# FprimeSoakTestReference/project/config/CMakeLists.txt:
#
# Configuration overrides for F Prime subtopologies and components.
# This enables TlmPacketizer instead of TlmChan for the CdhCore subtopology.
# The framework default AcConstants are used as-is; the default
# RateGroupDriverRateGroupPorts of 3 matches the deployment's three rate
# groups: 100Hz, 10Hz, and 1Hz.
####
register_fprime_config(
    CONFIGURATION_OVERRIDES
        "${CMAKE_CURRENT_LIST_DIR}/CdhCoreTlmConfig.fpp"
    INTERFACE
)
