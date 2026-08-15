"""Multi-chunk file uplink over the RFM69 radio (TX stays enabled)."""

import tempfile
from pathlib import Path

from soak_helpers import UPLINK_TIMEOUT_S, rf_uplink

# 8 KiB >> 255-byte RF MTU (~82 DATA chunks at 100 B, ~80 s at 1.00 s cooldown).
LARGE_UPLINK_BYTES = 8 * 1024


def test_file_uplink_larger_than_mtu(fprime_test_api):
    """Uplink a multi-chunk file; FileReceived is the radio checksum."""
    payload = bytes((i * 17 + 3) & 0xFF for i in range(LARGE_UPLINK_BYTES))
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as handle:
        handle.write(payload)
        local = Path(handle.name)
    rf_uplink(fprime_test_api, local, "/tmp/soak_up.bin", UPLINK_TIMEOUT_S)
