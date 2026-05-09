# AP33772S ESPHome External Component

An ESPHome external component for the [Diodes AP33772S](https://www.diodes.com/part/view/AP33772S) USB PD sink controller. Reads voltage, current, temperature, requested voltage/current, PD connection state, and protection status from the chip over I2C. Supports configurable power profile requests (PDO matching), automatic source capability change detection, and a keep-alive mechanism to prevent charger disconnects.

## Hardware Wiring

| AP33772S | ESP32 | ESP8266 (D1 Mini) |
|----------|-------|-------------------|
| SDA | GPIO21 | GPIO4 |
| SCL | GPIO22 | GPIO5 |
| VCC | 3.3V | 3.3V |
| GND | GND | GND |

The chip uses fixed I2C address `0x52`. Both SDA and SCL require external pull-up resistors (4.7kΩ typical).

## Quick Start

```yaml
external_components:
  - source:
      type: local
      path: components
    components: [ap33772s]

i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true

ap33772s:
  id: ap33772s_hub
  target_profiles:
    - voltage: 20.0
    - voltage: 12.0
    - voltage: 5.0

sensor:
  - platform: ap33772s
    ap33772s_id: ap33772s_hub
    voltage:
      name: "AP33772S Voltage"
    current:
      name: "AP33772S Current"
    temperature:
      name: "AP33772S Temperature"

binary_sensor:
  - platform: ap33772s
    ap33772s_id: ap33772s_hub
    pd_connected:
      name: "AP33772S PD Connected"
```

See `examples/` for complete configurations for ESP32 and ESP8266 (D1 Mini).

## Hub Configuration

```yaml
ap33772s:
  id: ap33772s_hub
  address: 0x52                       # default, can be omitted
  epr_mode: true                      # enable extended power range
  pps_avs: true                       # enable PPS and AVS
  dr_swap: false                      # accept data role swap
  de_rating_enable: true              # thermal de-rating
  otp_enable: true                    # overtemperature protection
  ocp_enable: true                    # overcurrent protection
  ovp_enable: true                    # overvoltage protection
  uvp_enable: true                    # undervoltage protection
  uvp_threshold: 80                   # 80%, 75%, or 70% of VREQ
  ovp_offset: 2.0                     # volts above VREQ
  ocp_threshold: auto                 # auto or amps (50mA/LSB)
  otp_threshold: 120                  # degrees C
  derating_threshold: 120             # degrees C
  request_current_limit: false        # limit CURRENT_SEL to profile value
  keep_alive_interval: 1000ms         # re-request interval, 0 to disable
  target_profiles:                    # priority-ordered list (optional)
    - voltage: 20.0                   # voltage only (max current)
    - voltage: 12.0
      current: 3.0                    # voltage with minimum current
    - voltage: 5.0
  on_pd_negotiation_success:          # automation trigger
    - logger.log: "PD negotiation succeeded"
  on_pd_negotiation_failure:          # automation trigger
    - logger.log: "PD negotiation failed"
  on_new_pdo:                         # automation trigger
    - logger.log: "New source capabilities detected"
```

### `target_profiles`

A priority-ordered list of desired power profiles. The component tries each profile in order and requests the first matching PDO from the source.

- **`voltage`** (required): Target voltage in volts. Must be within ±100mV (SPR) or ±200mV (EPR) of a fixed PDO, or within the voltage range of a PPS/AVS PDO.
- **`current`** (optional): Minimum required current. If omitted, the maximum available current is requested. Has no effect unless `request_current_limit: true`.

If no PDOs are detected (non-PD charger, legacy USB) and 5V is listed in `target_profiles`, the component treats this as a successful negotiation at 5V and fires the success trigger.

If no PDO matches any target profile, the failure trigger fires in the first `loop()`.

### `request_current_limit`

When `false` (default), `CURRENT_SEL` is always `0xF` (maximum, 5A+). Some chargers ignore non-maximum current requests, so this default is safest.

When `true` and the profile specifies a current, `CURRENT_SEL` is scaled to match: `0x0` = 1.0A, `0xF` = 5.0A+, 1.0-5.0A uses the formula `(current - 1.0) × 3.75 + 0.5`, rounded.

### `keep_alive_interval`

Some USB PD chargers disconnect if they don't receive a new PD request periodically. The keep-alive re-writes the last successful PD_REQMSG at this interval to prevent disconnection. Default `1000ms`. Set to `0ms` to disable.

When the charger changes capabilities (NEWPDO), keep-alive is automatically suspended while re-negotiation runs, then resumes with the new profile.

## Sensors

All sensors are optional. At least one sensor or binary sensor must be configured.

```yaml
sensor:
  - platform: ap33772s
    ap33772s_id: ap33772s_hub
    update_interval: 10s                # default
    voltage:
      name: "AP33772S Voltage"          # VOUT, 80mV/LSB
    current:
      name: "AP33772S Current"          # VOUT, 24mA/LSB
    temperature:
      name: "AP33772S Temperature"      # °C
    voltage_requested:
      name: "AP33772S Requested Voltage" # VREQ, 50mV/LSB
    current_requested:
      name: "AP33772S Requested Current" # IREQ, 10mA/LSB
```

## Binary Sensors

```yaml
binary_sensor:
  - platform: ap33772s
    ap33772s_id: ap33772s_hub
    update_interval: 10s                # default
    pd_connected:
      name: "AP33772S PD Connected"     # OPMODE bit 1
    fault_otp:
      name: "AP33772S OTP Fault"        # latching, cleared on re-negotiation
    fault_ocp:
      name: "AP33772S OCP Fault"
    fault_ovp:
      name: "AP33772S OVP Fault"
    fault_uvp:
      name: "AP33772S UVP Fault"
    derating:
      name: "AP33772S Derating"         # OPMODE bit 6
```

Protection fault sensors are latched — once a fault is detected (STATUS register bit), it stays `true` until the next successful PD negotiation. This prevents the clear-on-read STATUS register from losing the fault indication.

## Automation Triggers

### `on_pd_negotiation_success`

Fires once when a power profile request is accepted by the source. This fires after the initial `target_profiles` match, after a runtime `request_power_profile` action succeeds, or when using the default 5V fallback with a non-PD charger.

### `on_pd_negotiation_failure`

Fires when no matching PDO is found for any target profile, or when the PD request times out waiting for MSGRLT confirmation.

### `on_new_pdo`

Fires when the source announces new capabilities (STATUS NEWPDO bit). The component automatically re-reads PDOs and re-runs profile matching. This trigger fires before the subsequent success/failure trigger.

## Actions

### `request_power_profile`

Request a specific power profile at runtime. Voltage is required, current is optional. Both accept lambdas for dynamic values.

```yaml
on_...:
  - request_power_profile:
      voltage: 9.0

  - request_power_profile:
      voltage: 20.0
      current: 3.0
```

If no matching PDO is found, the action silently does nothing (the failure trigger is not fired).

### `hard_reset`

Issue a hard reset (PD_CMDMSG bit 0). Resets the PD link as if the cable was unplugged.

```yaml
on_...:
  - hard_reset:
```

## Lambda API

Use the hub ID to query PDO info from lambdas:

```cpp
auto n = id(ap33772s_hub).get_pdo_count();
for (uint8_t i = 0; i < n; i++) {
  auto p = id(ap33772s_hub).get_pdo(i);
  // p.index (1-13), p.voltage, p.min_voltage, p.max_current,
  // p.is_fixed, p.is_detected
}
```

`get_latched_faults()` returns a bitmask of latched protection faults (bit 6 = OTP, 5 = OCP, 4 = OVP, 3 = UVP).

## Development

```bash
# Validate YAML schemas
make config

# Compile all examples
make compile

# Compile D1 Mini only (faster)
make compile-d1-mini

# Upload and monitor
make hardware-run DEVICE=/dev/ttyUSB0
```

The Makefile keeps ESPHome and PlatformIO state local:
- `ESPHOME=.venv/bin/esphome`
- `PLATFORMIO_CORE_DIR=$PWD/.platformio`
- `UV_CACHE_DIR=/tmp/ap33772s-uv-cache`

## License

MIT
