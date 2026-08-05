// ======================================================================
// ModeManagerCfg.hpp
//
// FprimeSoakTestReference default transition policy.
//
// This default policy is unrestricted (permits all transitions) because
// we use an external ModePolicy component connected to checkTransition.
// The external policy takes precedence when connected.
// ======================================================================

#ifndef FPRIME_SOAK_TEST_REFERENCE_MODEMANAGERCFG_HPP
#define FPRIME_SOAK_TEST_REFERENCE_MODEMANAGERCFG_HPP

#include "Svc/ModeManager/config/ModeManagerConfig/ModeEnumAc.hpp"
#include "Svc/ModeManager/config/ModeManagerConfig/ModeRequestSerializableAc.hpp"

namespace ModeManagerCfg {

//! Default transition policy (fallback when checkTransition is unconnected)
//!
//! For this deployment, the external ModePolicy component enforces all
//! transition rules. This default remains unrestricted.
inline bool defaultPolicy(
    Svc::Mode::T /* current */,
    Svc::Mode::T /* target */,
    const Svc::ModeRequest& /* req */
) {
    return true;  // Permit all (policy is in ModePolicy component)
}

}  // namespace ModeManagerCfg

#endif
