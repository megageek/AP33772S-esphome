# AP33772S ESPHome External Component

This repository contains an ESPHome external component skeleton for the AP33772S chip. The current implementation registers a top-level `ap33772s` component on the I2C bus and does not expose sensors or read AP33772S registers yet.

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
```

The AP33772S uses fixed I2C address `0x52`.

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
make hardware-run DEVICE=/dev/ttyUSB0
make hardware-logs DEVICE=/dev/ttyUSB0
```

`hardware-run` uploads the firmware, resets before attaching logs, and should show the I2C scan finding `0x52`. Use it only when the D1 mini and AP33772S wiring match `examples/ap33772s-d1-mini.yaml`.
