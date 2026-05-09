#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

namespace esphome {
namespace ap33772s {

class AP33772SComponent : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  bool read_u8(uint8_t reg, uint8_t *value);
  bool read_u16_le(uint8_t reg, uint16_t *value);

 protected:
  bool read_register_(uint8_t reg, uint8_t *value);

  bool detected_{false};
  uint8_t status_{0};
  uint8_t opmode_{0};
  uint8_t config_{0};
  uint8_t pdconfig_{0};
};

}  // namespace ap33772s
}  // namespace esphome
