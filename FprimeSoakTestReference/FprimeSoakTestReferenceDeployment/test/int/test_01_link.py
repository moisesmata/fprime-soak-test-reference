"""Telemetry streaming and command EVRs over the ground radio."""

from soak_helpers import CMD_TIMEOUT_S, send_await


def test_telemetry_streaming(fprime_test_api):
    """FSW is alive and TM is crossing the RF link into GDS."""
    results = fprime_test_api.assert_telemetry_count(3, timeout=CMD_TIMEOUT_S)
    assert results, "Expected telemetry updates over the radio link"


def test_command_event_over_radio(fprime_test_api):
    """A command produces a downlinked application event (not just an opcode EVR)."""
    send_await(
        fprime_test_api,
        "CdhCore.cmdDisp.CMD_NO_OP",
        "CdhCore.cmdDisp.NoOpReceived",
    )


def test_command_event_with_args(fprime_test_api):
    """Same path with a unique argument so the EVR cannot be a stale match."""
    value = "soak-radio"
    pred = fprime_test_api.get_event_pred(
        "CdhCore.cmdDisp.NoOpStringReceived", [value]
    )
    send_await(
        fprime_test_api,
        "CdhCore.cmdDisp.CMD_NO_OP_STRING",
        pred,
        args=[value],
    )
