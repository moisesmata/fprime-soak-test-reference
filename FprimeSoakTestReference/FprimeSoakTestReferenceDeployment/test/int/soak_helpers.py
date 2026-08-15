"""Radio-only soak helpers. Every check is a GDS event or channel — no SSH."""

from __future__ import annotations

import json
import time
from pathlib import Path

INT_DIR = Path(__file__).parent.resolve()

with open(INT_DIR / "int_config.json", encoding="utf-8") as _cfg:
    CONFIG: dict = json.load(_cfg)

CMD_TIMEOUT_S = int(CONFIG.get("soak.cmd_timeout_s", 20))
UPLINK_TIMEOUT_S = int(CONFIG.get("soak.uplink_timeout_s", 90))
DP_PRODUCE_TIMEOUT_S = int(CONFIG.get("soak.dp_produce_timeout_s", 60))
DP_XMIT_TIMEOUT_S = int(CONFIG.get("soak.dp_xmit_timeout_s", 60))


def quiet(seconds: float = 2.0) -> None:
    time.sleep(seconds)


def send_await(api, command, event, args=None, timeout_s: int = CMD_TIMEOUT_S):
    """Send a command and require the named event on the downlink."""
    return api.send_and_assert_event(
        command, args or [], events=event, timeout=timeout_s
    )


def await_event(api, event, start, timeout_s: int, msg: str | None = None):
    ev = api.await_event(event, start=start, timeout=int(timeout_s))
    assert ev is not None, msg or f"{event} not received over radio"
    return ev


def rf_uplink(api, local_path: Path, dest: str, timeout_s: int = UPLINK_TIMEOUT_S) -> None:
    """Uplink over RF with TX enabled (GDS needs the FileUplink handshake).

    FileReceived means FileUplink's onboard checksum passed. One retry if the
    EVR is dropped; do not RemoveFile first (missing-file is WARNING_HI).
    """
    fu = api.get_mnemonic("Svc.FileUplink")
    event = f"{fu}.FileReceived"
    for attempt in (1, 2):
        start = api.get_event_test_history().size()
        api.uplink_file(str(local_path), dest)
        ev = api.await_event(event, start=start, timeout=int(timeout_s))
        if ev is not None:
            api.log(f"FileReceived for {dest} (attempt {attempt})")
            return
        api.log(f"FileReceived missed for {dest} (attempt {attempt})")
        quiet(2.0)
    raise AssertionError(f"FileReceived not seen for {dest} after 2 RF uplinks")
