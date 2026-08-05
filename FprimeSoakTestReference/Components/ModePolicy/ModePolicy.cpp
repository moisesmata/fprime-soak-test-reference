// ======================================================================
// \title  ModePolicy.cpp
// \brief  cpp file for ModePolicy component implementation class
// ======================================================================

#include "FprimeSoakTestReference/Components/ModePolicy/ModePolicy.hpp"

namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  ModePolicy ::
    ModePolicy(const char* const compName) :
      ModePolicyComponentBase(compName)
  {

  }

  ModePolicy ::
    ~ModePolicy()
  {

  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  Fw::Success ModePolicy ::
    checkTransition_handler(
        FwIndexType portNum,
        const Svc::Mode& current,
        const Svc::Mode& target,
        const Svc::ModeRequest& req
    )
  {
    // STARTUP can only transition to IDLE (via START command)
    if (current == Svc::Mode::STARTUP) {
      if (target == Svc::Mode::IDLE) {
        return Fw::Success::SUCCESS;
      }
      this->log_WARNING_LO_TransitionBlocked(
          current,
          target,
          Fw::String("STARTUP can only transition to IDLE")
      );
      return Fw::Success::FAILURE;
    }

    // SAFE can only transition to IDLE (REQ-MPOL-002, REQ-MPOL-003)
    if (current == Svc::Mode::SAFE) {
      if (target == Svc::Mode::IDLE) {
        return Fw::Success::SUCCESS;
      }
      this->log_WARNING_LO_TransitionBlocked(
          current,
          target,
          Fw::String("SAFE can only transition to IDLE")
      );
      return Fw::Success::FAILURE;
    }

    // IDLE can transition to EXPERIMENTATION or SAFE freely (REQ-MPOL-004, REQ-MPOL-005)
    if (current == Svc::Mode::IDLE) {
      if (target == Svc::Mode::EXPERIMENTATION || target == Svc::Mode::SAFE) {
        return Fw::Success::SUCCESS;
      }
      // Fallthrough to generic denial
    }

    // EXPERIMENTATION → SAFE always allowed (safety override) (REQ-MPOL-006)
    if (current == Svc::Mode::EXPERIMENTATION && target == Svc::Mode::SAFE) {
      return Fw::Success::SUCCESS;
    }

    // EXPERIMENTATION → IDLE: check serialization state (REQ-MPOL-007, REQ-MPOL-008, REQ-MPOL-009)
    if (current == Svc::Mode::EXPERIMENTATION && target == Svc::Mode::IDLE) {
      // Query serialization status if port is connected
      if (this->isConnected_querySerialization_OutputPort(0)) {
        // Fail safe: defaults to FAILURE (blocks the transition) if the
        // callee somehow returns without setting the condition.
        Fw::Success isNotSerializing(Fw::Success::FAILURE);
        this->querySerialization_out(0, isNotSerializing);
        if (isNotSerializing == Fw::Success::SUCCESS) {
          // NOT serializing - safe to transition
          return Fw::Success::SUCCESS;
        } else {
          // Currently serializing - block transition
          this->log_WARNING_LO_TransitionBlocked(
              current,
              target,
              Fw::String("Cannot leave EXPERIMENTATION while serializing")
          );
          return Fw::Success::FAILURE;
        }
      } else {
        // Port not connected - allow transition (graceful degradation)
        return Fw::Success::SUCCESS;
      }
    }

    // All other transitions: deny
    this->log_WARNING_LO_TransitionBlocked(
        current,
        target,
        Fw::String("Transition not permitted by policy")
    );
    return Fw::Success::FAILURE;
  }

}
