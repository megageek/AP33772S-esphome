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
  void set_pd_connected_sensor(binary_sensor::BinarySensor *s) { this->pd_connected_ = s; }
  void set_fault_otp_sensor(binary_sensor::BinarySensor *s) { this->fault_otp_ = s; }
  void set_fault_ocp_sensor(binary_sensor::BinarySensor *s) { this->fault_ocp_ = s; }
  void set_fault_ovp_sensor(binary_sensor::BinarySensor *s) { this->fault_ovp_ = s; }
  void set_fault_uvp_sensor(binary_sensor::BinarySensor *s) { this->fault_uvp_ = s; }
  void set_derating_sensor(binary_sensor::BinarySensor *s) { this->derating_ = s; }

 protected:
  AP33772SComponent *parent_{nullptr};
  binary_sensor::BinarySensor *pd_connected_{nullptr};
  binary_sensor::BinarySensor *fault_otp_{nullptr};
  binary_sensor::BinarySensor *fault_ocp_{nullptr};
  binary_sensor::BinarySensor *fault_ovp_{nullptr};
  binary_sensor::BinarySensor *fault_uvp_{nullptr};
  binary_sensor::BinarySensor *derating_{nullptr};
};

}  // namespace ap33772s
}  // namespace esphome
