module Components {

    @ External policy component for Svc::ModeManager. Enforces deployment-specific
    @ mode transition rules:
    @
    @ Transition Rules:
    @ - SAFE → IDLE (allowed)
    @ - SAFE → EXPERIMENTATION (blocked)
    @ - IDLE → EXPERIMENTATION (allowed)
    @ - IDLE → SAFE (allowed)
    @ - EXPERIMENTATION → SAFE (always allowed, safety override)
    @ - EXPERIMENTATION → IDLE (allowed only when NOT serializing)
    @
    @ See REQ-MPOL-001 through REQ-MPOL-010 in docs/sdd.md
    passive component ModePolicy {

        # ----------------------------------------------------------------------
        # Mode policy interface
        # ----------------------------------------------------------------------

        @ Check whether a mode transition is permitted. Called by ModeManager
        @ with the current mode, target mode, and requester provenance.
        @ Returns SUCCESS if permitted, FAILURE if blocked.
        sync input port checkTransition: Svc.CheckTransition

        @ Query SensorDataProducer serialization state. Returns SUCCESS if NOT
        @ serializing (safe to transition), FAILURE if actively serializing.
        output port querySerialization: Fw.SuccessCondition

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ A mode transition was blocked by this policy component
        event TransitionBlocked(
            fromMode: Svc.Mode @< The current mode
            toMode: Svc.Mode @< The requested (denied) mode
            reason: string size 80 @< Human-readable reason for denial
        ) \
          severity warning low \
          format "Mode transition {} -> {} blocked: {}"

        # ----------------------------------------------------------------------
        # Special ports
        # ----------------------------------------------------------------------

        @ Event port
        event port logOut

        @ Text event port
        text event port logTextOut

        @ Time get port
        time get port timeCaller

    }
}
