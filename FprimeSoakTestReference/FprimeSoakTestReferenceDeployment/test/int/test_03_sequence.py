"""Run a sequence that is already on the FSW. No uplink."""

from soak_helpers import CMD_TIMEOUT_S, CONFIG, send_await

# Stage this file on the FSW (not via this test). Default matches soak-setup dest.
SEQ_PATH = str(CONFIG.get("soak.seq_path", "/tmp/soak_seq.bin"))


def test_sequence_validate_and_run(fprime_test_api):
    """CS_VALIDATE then CS_RUN BLOCK; sequence must already exist onboard."""
    sequencer = fprime_test_api.get_mnemonic("Svc.CmdSequencer")
    send_await(
        fprime_test_api,
        f"{sequencer}.CS_VALIDATE",
        f"{sequencer}.CS_SequenceValid",
        args=[SEQ_PATH],
    )
    send_await(
        fprime_test_api,
        f"{sequencer}.CS_RUN",
        f"{sequencer}.CS_SequenceComplete",
        args=[SEQ_PATH, "BLOCK"],
        timeout_s=CMD_TIMEOUT_S,
    )
