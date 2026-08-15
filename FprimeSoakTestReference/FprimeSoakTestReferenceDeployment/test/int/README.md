# Soak / integration tests — FprimeSoakTestReferenceDeployment

Radio-only pytest suite for the RF soak deployment. Compatible with
[`nasa/fprime-actions/soak-test`](https://github.com/nasa/fprime-actions): each
interval runs `soak_monitor.py` over accumulated telemetry, then this directory
against the persistent GDS.

No SSH / `fsw.log` side channel. Every pass/fail is a GDS telemetry sample or
downlinked event. Target runtime is 5–10 minutes.

Layout matches [Ref `test/int`](https://github.com/nasa/fprime/tree/devel/TestDeploymentsProject/Ref/test/int):
no custom `conftest.py` — fixtures come from `fprime-gds`; helpers live in
`soak_helpers.py`.

## What the soak monitor gates on

`soak_monitor.py` fails an interval on **any** `FATAL` / `WARNING_HI` /
`WARNING_LO` event in the GDS log, plus: `MEMORY_USED` leak ≥ 5 %, any
`BufferManager` `CurrBuffs` leak ≥ 20 %, any `NoBuffs`/`EmptyBuffs` > 0,
`CPU` > 95 %, or `NON_VOLATILE_FREE` < 1 GiB.

Tests must not emit warnings. Do not `STOP_XMIT_CATALOG` while idle, do not
`FileSize` / `RemoveFile` a path that may be missing (`FileSizeError` /
`FileRemoveError` are `WARNING_HI`).

## What is covered

| Module | Purpose |
|--------|---------|
| `test_01_link.py` | TM streaming; command → downlinked application EVR |
| `test_02_radio.py` | `TRANSMIT` mute/unmute; EVR after TX restored |
| `test_03_sequence.py` | `CS_VALIDATE` + `CS_RUN` of a pre-staged FSW `.bin` |
| `test_04_file_uplink.py` | Multi-chunk uplink (8 KiB >> 255 B MTU); `FileReceived` |
| `test_05_dataproducts.py` | Serialize one `.fdp`, then `SendFile` that product |

Flight TX stays enabled for file uplink so GDS can see the FileUplink handshake.
`test_02` mutes TX only for that case and always re-enables it. DP downlink is
`FileDownlink.SendFile` of the just-written file (not a full catalog xmit).

`test_03` does **not** uplink the sequence. Put `soak_radio_probe.bin` on the FSW
at `soak.seq_path` (`/tmp/soak_seq.bin` by default) before the suite runs.
Soak-setup still compiles the `.bin` into the artifact so you can stage it.

## Flight runtime requirement: realtime scheduling

The FSW binary **must** run with `CAP_SYS_NICE` so F´ Posix tasks get `SCHED_RR`.
Without it the 1 kHz rate group runs `SCHED_OTHER` and `RateGroupCycleSlip`
(`WARNING_HI`) fails the gate. `nasa/fprime-actions` `deploy.sh` does this via
`setcap cap_sys_nice=eip`.

## Parameter DB

On first boot with no `PrmDb.dat`, `PrmDb` emits `PrmFileReadError` /
`PrmIdNotFound`. Run `seq/fix_prm_missing.bin` once; later boots are clean.

## Known limitations

* **Multi-chunk file uplink has no ARQ.** A dropped DATA chunk can stall
  `Svc.FileUplink`. `rf_uplink` retries the transfer once.
* **`UnexpectedSequenceCount` (`WARNING_LO`)** can still appear from RF loss.

## Local HIL run

Do **not** start GDS from `fprime-rfm69-feather-groundstation` (that yml sets
`output-unframed-data: "-"` and empties UART).

Install the `space-packet-fprime` framer once into the GDS venv:

```bash
cd ~/InternshipWork/soak-testing/fprime-soak-test-reference
source fprime-venv/bin/activate
pip install -e gds-plugin
```

**Terminal 1 — GDS** (deployment directory; `fprime-gds.yml` uses
`/dev/rfm69-feather`, `space-packet-fprime`, TTS 52051, chunk 100, cooldown 1.00):

```bash
cd ~/InternshipWork/soak-testing/fprime-soak-test-reference
source fprime-venv/bin/activate
cd FprimeSoakTestReference/FprimeSoakTestReferenceDeployment
fprime-gds --uart-device /dev/rfm69-feather --framing-selection space-packet-fprime
```

**Terminal 2 — pytest** (repo root, same venv):

```bash
cd ~/InternshipWork/soak-testing/fprime-soak-test-reference
source fprime-venv/bin/activate
pytest FprimeSoakTestReference/FprimeSoakTestReferenceDeployment/test/int \
  --dictionary ./build-artifacts/aarch64-linux/FprimeSoakTestReference_FprimeSoakTestReferenceDeployment/dict/FprimeSoakTestReferenceDeploymentTopologyDictionary.json \
  --deployment-config FprimeSoakTestReference/FprimeSoakTestReferenceDeployment/test/int/int_config.json
```

`pytest.ini` supplies `-v`, chunk 100, cooldown 1.00, and `--tts-port 52051`.

## Config knobs (`int_config.json`)

| Key | Role |
|-----|------|
| `soak.seq_path` | Onboard sequence for `CS_VALIDATE` / `CS_RUN` |
| `soak.cmd_timeout_s` | Command / EVR waits |
| `soak.uplink_timeout_s` | Multi-chunk RF uplink wait |
| `soak.dp_produce_timeout_s` | Wait for one filled container |
| `soak.dp_xmit_timeout_s` | `SendFile` of the produced `.fdp` |
