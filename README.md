# AP33772S ESPHome External Component

This repository contains an ESPHome external component for the AP33772S chip. The implementation registers a top-level `ap33772s` hub on the I2C bus, verifies the chip during setup, and exposes voltage, current, and temperature sensors.

## Usage

Use the component from a local checkout while developing:

```yaml
external_components:
  - source:
      type: local
      path: ../components
    components: [ap33772s]

i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true

ap33772s:
  id: ap33772s_hub
  address: 0x52

sensor:
  - platform: ap33772s
    ap33772s_id: ap33772s_hub
    voltage:
      name: "AP33772S Voltage"
    current:
      name: "AP33772S Current"
    temperature:
      name: "AP33772S Temperature"
```

The AP33772S uses fixed I2C address `0x52`. Sensor polling defaults to `10s` and can be overridden with `update_interval`.

## Reference Material

Use `documents/i2c-notes.md` as the working source for component implementation details. It distills the AP33772S datasheet into the I2C address, register map, status/configuration bits, PD request format, and sensor conversion factors relevant to this ESPHome component.

## Development

Validate and compile all example configurations from the repository root:

```bash
make config
make compile
```

The Makefile keeps ESPHome and PlatformIO state local to this workspace by default:

```bash
ESPHOME=.venv/bin/esphome
PLATFORMIO_CORE_DIR="$PWD/.platformio"
UV_CACHE_DIR=/tmp/ap33772s-uv-cache
```

## Hardware Testing

The D1 mini hardware test uses ESP8266 board `d1_mini`, SDA on `GPIO4`, SCL on `GPIO5`, and AP33772S address `0x52`.

```bash
make compile-d1-mini
make hardware-smoke DEVICE=/dev/ttyUSB0
make hardware-run DEVICE=/dev/ttyUSB0
make hardware-logs DEVICE=/dev/ttyUSB0
```

`hardware-smoke` uploads the firmware, watches startup logs for the I2C scan and AP33772S probe, then exits once the expected lines are observed. Override its deadline with `HARDWARE_TIMEOUT=120` when needed. Use `hardware-run` or `hardware-logs` only for interactive debugging because they keep the serial monitor attached. Hardware targets assume the D1 mini and AP33772S wiring match `examples/ap33772s-d1-mini.yaml`.
