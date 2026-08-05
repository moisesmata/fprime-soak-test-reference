####
# FprimeSoakTestReference/project/config/CMakeLists.txt:
#
# Configuration overrides for F Prime subtopologies and components.
# This enables TlmPacketizer instead of TlmChan for the CdhCore subtopology.
# ComCfg/FpConstants are sized for a single RFM69 radio packet (255-byte TM).
# ModeManager configuration defines custom modes: STARTUP, IDLE, EXPERIMENTATION, SAFE.
####
register_fprime_config(
    CONFIGURATION_OVERRIDES
        "${CMAKE_CURRENT_LIST_DIR}/CdhCoreTlmConfig.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/ComCfg.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/FpConstants.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/AcConstants.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/ModeManagerCfg.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/ModeMachine.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/ModeManagerCfg.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/ModeList.inc"
    INTERFACE
)

# Both ModeManagerConfig CMake gaps (missing DEPENDS on the config module itself,
# and the ModeList.inc override-shadowing in ModeManager.cpp) are fixed upstream in
# lib/fprime as of devel_modeManager -- no project-level workaround needed here.
