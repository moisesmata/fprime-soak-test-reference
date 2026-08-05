// ======================================================================
// ModeList.inc
//
// FprimeSoakTestReference mode dispatch table.
// Maps each requestable mode to its ModeMachine signal.
//
// STARTUP is omitted -- it is the initial state and cannot be requested.
// ======================================================================

MODE_ENTRY(IDLE,           goToIdle)
MODE_ENTRY(EXPERIMENTATION, goToExperimentation)
MODE_ENTRY(SAFE,           goToSafe)
