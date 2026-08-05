# ======================================================================
# ModeManagerCfg.fpp
#
# FprimeSoakTestReference deployment mode configuration.
# Overrides default Svc/ModeManager/config/ModeManagerConfig.
# ======================================================================

module Svc {

    @ Spacecraft modes for this deployment
    enum Mode {
        STARTUP          @< Initial mode before operations begin
        IDLE             @< Quiescent operational state
        EXPERIMENTATION  @< Active data collection mode
        SAFE             @< Fault-safe mode with limited operations
    }

    @ Where a transition request originated
    enum ModeRequestSource {
        GROUND
        COMPONENT
    }

    @ Which component requested a transition. Each value MUST equal the
    @ index of that component's requestMode port.
    enum Requester {
        NONE = 0     @< Used when the source is GROUND
        POLICY = 1   @< Mode policy component (if it requests transitions)
    }

    @ Provenance of a transition request
    struct ModeRequest {
        source: ModeRequestSource
        requester: Requester
    }

    @ Which policy decided a transition
    enum ModePolicySource {
        DEFAULT
        EXTERNAL
    }

}

module ModeManagerCfg {

    @ Size of the requestMode port array
    constant NUM_REQUESTERS = 2

    @ Size of the modeChanged port array
    constant NUM_SUBSCRIBERS = 8

}
