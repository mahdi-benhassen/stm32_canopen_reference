# STM32F767 CANopen — CubeMX Integration Branch

CANopen firmware for the **STM32F767**, built on the **STM32CubeMX-generated**
project in this branch (`stm32f767_canopen.ioc`). The CubeMX base is kept
regenerable: all CANopen integration lives in USER CODE sections, new
project-owned directories, and the designated user areas of `CMakeLists.txt`.

See [`PORTING.md`](PORTING.md) for the layer map and the deliberate
peripheral fix-ups applied on top of the generated configuration.

## What this branch contains

- Pinned [CanOpenSTM32](https://github.com/CANopenNode/CanOpenSTM32)
  submodule (`third_party/`) providing CANopenNode and the bxCAN binding.
- Project runtime (`App/`): lifecycle wrapper, CiA 401 default personality,
  dormant CiA 402/418/Inventus adapters, bounded bus-off recovery, CRC
  dual-slot flash persistence, dual-rate watchdog with post-mortem fault
  record, diagnostics, LSS policy, fail-closed gateway seam.
- CiA 302 NMT-master helper and acceptance-filter policy
  (`middleware/canopen/core`).
- Full validation ecosystem ported from `main`: host unit tests, 105
  conformance vectors, wire contracts, OD/product validators, release gates.

## License

Project-owned material is available under the [STM32 CANopen Reference Research and Education License](LICENSE). It is free only for qualifying research and education; industrial, commercial, production, and large-scale use requires a separate paid commercial license. This is source-available and is not an OSI-approved Open Source license. See [COMMERCIAL-LICENSE.md](COMMERCIAL-LICENSE.md) for commercial licensing requests. Third-party components retain their upstream licenses as listed in [THIRD_PARTY.md](THIRD_PARTY.md).

## Build

Requires CMake + Ninja + GNU Arm Embedded toolchain. HAL/CMSIS are vendored
in-repo (`Drivers/`) — no external Cube directory is needed.

```sh
cmake --preset Release
cmake --build --preset Release --parallel
arm-none-eabi-size build/Release/stm32f767_canopen.elf
```

Optional personalities mirror `main`:
`-DCANOPEN_REFERENCE_ENABLE_CIA418=ON`,
`-DCANOPEN_REFERENCE_ENABLE_INVENTUS_BATTERY=ON`,
`-DCANOPEN_REFERENCE_ENABLE_CIA302_MASTER=ON`.

## Validation

```sh
python3 scripts/validate_repository.py
python3 tests/test_firmware_configuration.py
make -C tests/host all test-stm32-facade test-gateway-default-deny test-acceptance-filter
```

CI (`.github/workflows/ci.yml`) runs static analysis, the host validation
suite, and cross-builds every personality with memory-budget, coverage, and
sanitizer gates.

## UDS diagnostics

The opt-in UDS profile is a bounded classic-CAN ISO-TP and UDS reference subset. Review the [UDS/ISO-TP architecture](docs/11_uds_iso_tp.md), [DID and SecurityAccess policy](docs/12_uds_security_and_dids.md), [download and recovery boundary](docs/13_uds_download_and_recovery.md), and [validation and acceptance plan](docs/14_uds_validation_and_acceptance.md). The [STM32F767 hardware runner](tests/hardware/run_stm32f767_uds_acceptance.py) keeps reset and download operations disabled unless explicitly enabled. This branch does not claim complete ISO 14229 or ISO 15765-2 conformance, a production bootloader, or production cryptographic update security.

## Hardware notes

The generated pin map uses PI9 (CAN1_RX) / PA12 (CAN1_TX). Board bring-up,
transceiver control, and application I/O remain board-specific weak hooks in
`App/Src/canopen_reference_hw.c` and `canopen_reference_board.c`.

| Topic | Document |
|---|---|
| Porting layers and deviations | [PORTING.md](PORTING.md) |
| Reproducible build | [BUILD.md](BUILD.md) |
| License | [LICENSE](LICENSE) |
| Dependencies and licenses | [THIRD_PARTY.md](THIRD_PARTY.md) |
| Contribution process | [CONTRIBUTING.md](CONTRIBUTING.md) |
| Security boundaries | [SECURITY.md](SECURITY.md) |
| UDS/ISO-TP reference profile | [Architecture](docs/11_uds_iso_tp.md), [security and DIDs](docs/12_uds_security_and_dids.md), [download and recovery](docs/13_uds_download_and_recovery.md), and [acceptance plan](docs/14_uds_validation_and_acceptance.md) |
| Change history | [CHANGELOG.md](CHANGELOG.md) |
