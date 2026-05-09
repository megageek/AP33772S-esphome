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

Validate the example configuration from the repository root:

```bash
.venv/bin/esphome config examples/ap33772s-component.yaml
UV_CACHE_DIR=/tmp/ap33772s-uv-cache \
PLATFORMIO_CORE_DIR="$PWD/.platformio" \
  .venv/bin/esphome compile examples/ap33772s-component.yaml
```

Use `esphome run examples/ap33772s-component.yaml` only when AP33772S hardware is connected and the example pin assignments match the target board.
