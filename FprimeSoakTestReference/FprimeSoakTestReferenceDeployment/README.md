# FprimeSoakTestReferenceDeployment (UDP / Space Packet branch)

This branch keeps the same **CCSDS Space Packet framing** stack as `master`
(`ComCcsds.SpacePacket` = `SpacePacketFraming` + `ComStub`) but replaces the
RFM69 radio with **`Drv.Udp`**. Use `master` for the packet-radio soak path.

## Build

```bash
cd FprimeSoakTestReferenceDeployment
fprime-util generate
fprime-util build
```

## Run (laptop)

Terminal 1 — GDS (uses `fprime-gds.yml`: UDP port 50000, raw-space-packet):

```bash
cd FprimeSoakTestReferenceDeployment
fprime-gds
```

Terminal 2 — FSW (send to GDS on 50000, bind local recv on 50001 by default):

```bash
./build-artifacts/Darwin/FprimeSoakTestReference_FprimeSoakTestReferenceDeployment/bin/FprimeSoakTestReference_FprimeSoakTestReferenceDeployment \
  -a 127.0.0.1 -p 50000
# optional: -l <local-recv-port>
```

## Architecture notes

- **CdhCore / FileHandling / DataProducts / ModeManager** unchanged from radio branch
- **ComCcsds.SpacePacket**: Space Packet framer/deframer + aggregator + ComStub
- **Drv.Udp** (`comDriver`): ByteStream adapter wired to ComStub
- Sensors (BMP280 / MPU) remain in the topology for application parity
