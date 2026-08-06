"""HIL: 5KB file uplink with flight TX left enabled (concurrent downlink)."""
from __future__ import annotations

import time
from pathlib import Path

from soak_helpers import (
    FSW_TMP,
    pi_ssh,
    send_cmd,
)

SIZE = 5 * 1024
LOCAL = Path("/tmp/soak_5kb_uplink.bin")
DEST = f"{FSW_TMP}/soak_5kb_uplink.bin"


def test_5kb_uplink_with_tx_enabled(fprime_test_api):
    api = fprime_test_api
    LOCAL.write_bytes(bytes((i * 17 + 3) & 0xFF for i in range(SIZE)))
    assert LOCAL.stat().st_size == SIZE

    # Prove downlink is live before uplink
    send_cmd(api, "CdhCore.cmdDisp.CMD_NO_OP")
    tlm_before = api.get_telemetry_test_history().size()

    try:
        pi_ssh(f"rm -f {DEST}")
    except Exception:
        pass

    # Do NOT mute TRANSMIT — goal is concurrent downlink during uplink
    api.log(f"Starting 5KB uplink TX-ON -> {DEST}")
    api.uplink_file(str(LOCAL), DEST)

    deadline = time.time() + 180
    last = -1
    while time.time() < deadline:
        out = pi_ssh(f"if [ -f {DEST} ]; then wc -c < {DEST}; else echo 0; fi").strip()
        size = int(out.splitlines()[-1])
        if size != last:
            api.log(f"uplink size={size}/{SIZE}")
            last = size
        if size == SIZE:
            break
        time.sleep(1.0)
    else:
        raise AssertionError(f"5KB uplink incomplete: {last}/{SIZE}")

    # Telemetry should have advanced (TX stayed on)
    tlm_after = api.get_telemetry_test_history().size()
    api.log(f"tlm history before={tlm_before} after={tlm_after}")
    assert tlm_after > tlm_before, "Expected telemetry while TX enabled during uplink"

    # Still commandable
    send_cmd(api, "CdhCore.cmdDisp.CMD_NO_OP")
    api.log("5KB uplink with TX-ON succeeded")
