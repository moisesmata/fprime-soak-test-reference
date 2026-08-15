"""RFM69 TRANSMIT mute/unmute over the ground radio."""

from soak_helpers import quiet, send_await


def test_transmit_mute_unmute(fprime_test_api):
    """TX off: uplink still works. TX on: a command EVR comes back."""
    radio = fprime_test_api.get_mnemonic("Rfm69.Rfm69Manager")
    try:
        fprime_test_api.send_command(f"{radio}.TRANSMIT", ["DISABLED"])
        quiet(1.5)
        # Do not await EVRs while muted — flight TX is off.
        fprime_test_api.send_command("CdhCore.cmdDisp.CMD_NO_OP")
        quiet(1.0)
    finally:
        fprime_test_api.send_command(f"{radio}.TRANSMIT", ["ENABLED"])
        quiet(3.0)
    send_await(
        fprime_test_api,
        "CdhCore.cmdDisp.CMD_NO_OP",
        "CdhCore.cmdDisp.NoOpReceived",
    )
