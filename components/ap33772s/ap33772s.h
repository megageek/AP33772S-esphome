#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

namespace esphome {
namespace ap33772s {

class AP33772SComponent : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
};

}  // namespace ap33772s
}  // namespace esphome
