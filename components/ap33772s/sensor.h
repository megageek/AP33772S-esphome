#pragma once

#include "ap33772s.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace ap33772s {

class AP33772SSensorComponent : public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_parent(AP33772SComponent *parent) { this->parent_ = parent; }
  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { this->voltage_sensor_ = voltage_sensor; }
  void set_current_sensor(sensor::Sensor *current_sensor) { this->current_sensor_ = current_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }

 protected:
  bool publish_voltage_();
  bool publish_current_();
  bool publish_temperature_();

  AP33772SComponent *parent_{nullptr};
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
};

}  // namespace ap33772s
}  // namespace esphome
