#pragma once

#include "ap33772s.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace ap33772s {

class AP33772SBinarySensorComponent : public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_parent(AP33772SComponent *parent) { this->parent_ = parent; }
  void set_pd_connected_sensor(binary_sensor::BinarySensor *pd_connected) { this->pd_connected_ = pd_connected; }

 protected:
  AP33772SComponent *parent_{nullptr};
  binary_sensor::BinarySensor *pd_connected_{nullptr};
};

}  // namespace ap33772s
}  // namespace esphome
