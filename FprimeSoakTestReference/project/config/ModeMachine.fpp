# ======================================================================
# ModeMachine.fpp
#
# FprimeSoakTestReference mode state machine.
# Defines transitions for STARTUP, IDLE, EXPERIMENTATION, SAFE.
# ======================================================================

module Svc {

    @ Mode state machine. Records the current mode and announces changes.
    @ Enforces no policy of its own -- policy is in ModePolicy component.
    state machine ModeMachine {

        @ STARTUP performs no output. The initial transition runs during
        @ init(), before topology connections exist.
        initial enter STARTUP

        @ Transition to IDLE
        signal goToIdle: ModeRequest

        @ Transition to EXPERIMENTATION
        signal goToExperimentation: ModeRequest

        @ Transition to SAFE
        signal goToSafe: ModeRequest

        @ Record the new mode, emit telemetry and the transition event,
        @ and notify all connected subscribers
        action announce: ModeRequest

        state STARTUP {
            on goToIdle            do { announce } enter IDLE
            on goToExperimentation do { announce } enter EXPERIMENTATION
            on goToSafe            do { announce } enter SAFE
        }

        state IDLE {
            on goToIdle            do { announce } enter IDLE
            on goToExperimentation do { announce } enter EXPERIMENTATION
            on goToSafe            do { announce } enter SAFE
        }

        state EXPERIMENTATION {
            on goToIdle            do { announce } enter IDLE
            on goToExperimentation do { announce } enter EXPERIMENTATION
            on goToSafe            do { announce } enter SAFE
        }

        state SAFE {
            on goToIdle            do { announce } enter IDLE
            on goToExperimentation do { announce } enter EXPERIMENTATION
            on goToSafe            do { announce } enter SAFE
        }

    }
}
