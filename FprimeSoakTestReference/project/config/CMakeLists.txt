####
# FprimeSoakTestReference/project/config/CMakeLists.txt:
#
# Configuration overrides for F Prime subtopologies and components.
# This enables TlmPacketizer instead of TlmChan for the CdhCore subtopology.
####
register_fprime_config(
    CONFIGURATION_OVERRIDES
        "${CMAKE_CURRENT_LIST_DIR}/CdhCoreTlmConfig.fpp"
    INTERFACE
)
