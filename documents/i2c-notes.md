# AP33772S I2C Notes

Source: `documents/AP33772S.pdf`, Diodes Incorporated AP33772S datasheet, document DS46176 Rev. 9-2, February 2026.

This file distills the information needed to implement and test the ESPHome component. It intentionally omits circuit diagrams, layout guidance, package data, and electrical limit tables.

## Bus Basics

- The AP33772S is an I2C slave at 7-bit address `0x52`.
- SDA and SCL require external pull-ups.
- Host MCU is the I2C master. Transactions use normal I2C start, stop, repeated-start, and ACK behavior.
- Typical register access is command/register byte followed by read or write data.
- Multi-byte values are little-endian: least-significant byte is stored at the lowest byte position.
- The `INT` pin is level-triggered. LOW means normal; HIGH means an enabled interrupt event occurred.

## Startup and Power Policy Flow

1. After VCC is powered from VBUS, the chip initializes internal defaults and becomes ready for I2C access.
2. `STATUS.STARTED` becomes `1`; the host may update configuration registers within the startup window described by the datasheet.
3. The chip receives source capabilities, requests default 5V first, then sets `STATUS.NEWPDO` and `STATUS.READY`.
4. Host reads source PDO data from `SRCPDO` or individual PDO registers.
5. Host writes `PD_REQMSG` to request a selected PDO/APDO.
6. After negotiation and `PS_RDY`, `PD_MSGRLT.RESPONSE` is set and status/result registers are updated.
7. If a protection fault occurs, VOUT is disabled. The host must issue a new request to resume negotiation.

## Register Summary

| Name | Cmd | Len | Default | Use |
| --- | ---: | ---: | ---: | --- |
| `STATUS` | `0x01` | 1 | `0x00` | Status and fault flags |
| `MASK` | `0x02` | 1 | `0x03` | Interrupt enable mask |
| `OPMODE` | `0x03` | 1 | `0x00` | PD/legacy mode, CC flip, de-rating, data role |
| `CONFIG` | `0x04` | 1 | `0xF8` | Enable/disable protection features |
| `PDCONFIG` | `0x05` | 1 | `0x03` | EPR, PPS/AVS, and data-role-swap configuration |
| `SYSTEM` | `0x06` | 1 | `0x10` | System control/information |
| `TR25` | `0x0C` | 2 | `0x2710` | NTC resistance at 25 C, ohms |
| `TR50` | `0x0D` | 2 | `0x1041` | NTC resistance at 50 C, ohms |
| `TR75` | `0x0E` | 2 | `0x0788` | NTC resistance at 75 C, ohms |
| `TR100` | `0x0F` | 2 | `0x03CE` | NTC resistance at 100 C, ohms |
| `VOLTAGE` | `0x11` | 2 | `0x0000` | VOUT voltage, 80 mV/LSB |
| `CURRENT` | `0x12` | 1 | `0x00` | VOUT current, 24 mA/LSB |
| `TEMP` | `0x13` | 1 | `0x19` | Temperature in C |
| `VREQ` | `0x14` | 2 | `0x0000` | Requested voltage, 50 mV/LSB |
| `IREQ` | `0x15` | 2 | `0x0000` | Requested current, 10 mA/LSB |
| `VSELMIN` | `0x16` | 1 | `0x19` | Minimum selection voltage, 200 mV/LSB |
| `UVPTHR` | `0x17` | 1 | `0x01` | UVP threshold selector |
| `OVPTHR` | `0x18` | 1 | `0x19` | OVP offset from VREQ, 80 mV/LSB |
| `OCPTHR` | `0x19` | 1 | `0x00` | OCP threshold, 50 mA/LSB |
| `OTPTHR` | `0x1A` | 1 | `0x78` | OTP threshold in C |
| `DRTHR` | `0x1B` | 1 | `0x78` | De-rating threshold in C |
| `SRCPDO` | `0x20` | 26 | all `0x00` | Bulk read PDO1-PDO13 |
| `SRC_SPR_PDO1`-`7` | `0x21`-`0x27` | 2 | `0x0000` | SPR source PDOs |
| `SRC_EPR_PDO8`-`13` | `0x28`-`0x2D` | 2 | `0x0000` | EPR source PDOs |
| `PD_REQMSG` | `0x31` | 2 | `0x0000` | Request selected PDO/APDO |
| `PD_CMDMSG` | `0x32` | 1 | `0x00` | Issue PD command |
| `PD_MSGRLT` | `0x33` | 1 | `0x00` | PD request/command result |
| `GPIO` | `0x52` | 1 | `0x00` | GPIO control on AP33772SDKZ-13-FA02 |

## Status and Mode Bits

`STATUS` (`0x01`) bits:

| Bit | Name | Meaning |
| ---: | --- | --- |
| 6 | `OTP` | Overtemperature protection status |
| 5 | `OCP` | Overcurrent protection status |
| 4 | `OVP` | Overvoltage protection status |
| 3 | `UVP` | Undervoltage protection status |
| 2 | `NEWPDO` | New source PDOs received; valid when `OPMODE.PDMOD = 1` |
| 1 | `READY` | Ready to receive I2C request/command |
| 0 | `STARTED` | System started; configuration update window is open |

`OPMODE` (`0x03`) bits:

| Bit | Name | Meaning |
| ---: | --- | --- |
| 7 | `CCFLIP` | `0`: CC1 or unattached; `1`: CC2 |
| 6 | `DR` | `1`: de-rating mode active |
| 5 | `DATARL` | Current port data role |
| 1 | `PDMOD` | PD source connected |
| 0 | `LGCYMOD` | Legacy non-PD source connected |

`MASK` (`0x02`) enables interrupt generation for status events. Bits 6:0 map to `OTP`, `OCP`, `OVP`, `UVP`, `NEWPDO`, `READY`, and `STARTED`; defaults enable `READY` and `STARTED`.

## Configuration and Protection

`SYSTEM` (`0x06`) bits:

| Bit | Name | Meaning |
| ---: | --- | --- |
| 4 | — | Set by default (0x10); function unknown |
| 2:1 | `VOUTCTL` | VOUT control: `00` = normal, `01` = discharge, `10` = disable, `11` = reserved |

`CONFIG` (`0x04`) enables protection features:

| Bit | Name | Meaning |
| ---: | --- | --- |
| 7 | `DR_EN` | Enable de-rating |
| 6 | `OTP_EN` | Enable overtemperature protection |
| 5 | `OCP_EN` | Enable overcurrent protection |
| 4 | `OVP_EN` | Enable overvoltage protection |
| 3 | `UVP_EN` | Enable undervoltage protection |

`PDCONFIG` (`0x05`) controls PD capabilities:

| Bit | Name | Meaning |
| ---: | --- | --- |
| 2 | `DRSWP_EN` | Reject/accept data role swap from partner |
| 1 | `PPS_AVS_EN` | Disable/enable PPS and AVS sink capability |
| 0 | `EPR_MODE_EN` | Disable/enable EPR mode |

Threshold conversions:

- `UVPTHR`: `1` = 80% of `VREQ`, `2` = 75%, `3` = 70%.
- `OVPTHR`: offset from `VREQ`, 80 mV/LSB. If `VREQ + offset` exceeds mode limit, threshold becomes 110% of `VREQ`.
- `OCPTHR`: 50 mA/LSB. If zero after negotiation, threshold is 110% of selected PDO/APDO maximum current.
- `OTPTHR` and `DRTHR`: degrees C. Default `0x78` = 120 C.

## PDO and Request Handling

- Read all source capabilities with `SRCPDO` (`0x20`, 26 bytes). It returns 13 two-byte PDO slots.
- Individual PDO reads are available at `0x21`-`0x27` for SPR PDO1-PDO7 and `0x28`-`0x2D` for EPR PDO8-PDO13.
- Each PDO entry is two bytes and encodes PDO type, voltage range, and maximum current. The datasheet does not provide a full bit decoding table beyond that description.
- `PD_REQMSG` (`0x31`, 16 bits, little-endian) requests a new source profile:

| Bits | Field | Meaning |
| ---: | --- | --- |
| 15:12 | `PDO_INDEX` | `1`-`7` for SPR PDO1-PDO7; `8`-`13` for EPR PDO8-PDO13 |
| 11:8 | `CURRENT_SEL` | Operating current selector, `0`-`15` = 1.00 A to 5.00 A |
| 7:0 | `VOLTAGE_SEL` | Fixed PDO: ignored; PPS APDO: 100 mV/LSB; AVS APDO: 200 mV/LSB |

To request maximum voltage and current for a detected PDO, set `CURRENT_SEL = 0xF` and `VOLTAGE_SEL = 0xFF`. Example from the datasheet: PDO3 maximum request is `0x3FFF`.

## Monitoring Values for ESPHome Sensors

Likely first sensors:

- VOUT voltage: read `VOLTAGE` (`0x11`, 2 bytes), value in volts = raw * 0.080.
- VOUT current: read `CURRENT` (`0x12`, 1 byte), value in amps = raw * 0.024.
- Temperature: read `TEMP` (`0x13`, 1 byte), value in C = raw.
- Requested voltage: read `VREQ` (`0x14`, 2 bytes), value in volts = raw * 0.050.
- Requested current: read `IREQ` (`0x15`, 2 bytes), value in amps = raw * 0.010.

Status-style binary sensors or text diagnostics can be derived from `STATUS` and `OPMODE`.

## PD Commands

`PD_CMDMSG` (`0x32`) bits:

| Bit | Name | Meaning |
| ---: | --- | --- |
| 1 | `DRSWP` | Issue data role swap |
| 0 | `HRST` | Issue hard reset |

`PD_MSGRLT` (`0x33`) reports PD request/command result. The flowchart specifically checks `PD_MSGRLT.RESPONSE = 1` after the AP33772S receives `PS_RDY` from the source.

## Implementation Notes

- Keep I2C reads/writes little-endian for all two-byte values.
- Treat reserved bits as zero on write and ignore them on read.
- Initial D1 mini hardware testing read `CONFIG = 0xFC`, even though the datasheet lists bit 2 as reserved with a power-on value of `0`; do not use that bit as an identity check.
- Do not assume detailed PDO bit decoding until confirmed by hardware behavior or additional documentation.
- For initial component work, prioritize periodic reads of `STATUS`, `OPMODE`, `VOLTAGE`, `CURRENT`, `TEMP`, `VREQ`, and `IREQ`.
- For write support, start conservatively with explicit configuration/threshold setters and `PD_REQMSG`; avoid automatic power renegotiation until sensor reads and status handling are stable.
