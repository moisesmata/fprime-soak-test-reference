"""Produce one data product and downlink that file over the radio."""

from soak_helpers import (
    CMD_TIMEOUT_S,
    DP_PRODUCE_TIMEOUT_S,
    DP_XMIT_TIMEOUT_S,
    await_event,
    quiet,
    send_await,
)


def test_dp_produce_and_downlink(fprime_test_api):
    """START_SERIALIZING → one .fdp → STOP → SendFile of that product.

    Catalog xmit is not used: leftover ./DpCat files would make downlink unbounded.
    STOP always runs so the producer cannot keep flooding RF after the test.
    """
    producer = fprime_test_api.get_mnemonic("Components.SensorDataProducer")
    writer = fprime_test_api.get_mnemonic("Svc.DpWriter")
    downlink = fprime_test_api.get_mnemonic("Svc.FileDownlink")

    # Best-effort idle; do not fail if this EVR is dropped on RF.
    fprime_test_api.send_command(f"{producer}.STOP_SERIALIZING")
    quiet(2.0)

    start = fprime_test_api.get_event_test_history().size()
    try:
        send_await(
            fprime_test_api,
            f"{producer}.START_SERIALIZING",
            f"{producer}.DpProductionStarted",
        )
        # RECORD_COUNT=100, two 10 Hz sensors, stride 5 → ~4 records/s → ~25 s.
        await_event(
            fprime_test_api,
            f"{producer}.DpComplete",
            start,
            DP_PRODUCE_TIMEOUT_S,
            "DpComplete not received (sensors running?)",
        )
        written = await_event(
            fprime_test_api,
            f"{writer}.FileWritten",
            start,
            CMD_TIMEOUT_S,
            "DpWriter.FileWritten not received",
        )
    finally:
        send_await(
            fprime_test_api,
            f"{producer}.STOP_SERIALIZING",
            f"{producer}.DpProductionStopped",
        )

    # "Wrote <bytes> bytes to file <path>"
    dp_path = written.get_display_text().split().pop()
    fprime_test_api.log(f"Downlinking {dp_path}")
    send_await(
        fprime_test_api,
        f"{downlink}.SendFile",
        f"{downlink}.FileSent",
        args=[dp_path, "soak_dp.fdp"],
        timeout_s=DP_XMIT_TIMEOUT_S,
    )
