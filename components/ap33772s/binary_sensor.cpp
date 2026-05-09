#include "binary_sensor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace ap33772s {

static const char *const TAG = "ap33772s.binary_sensor";

static constexpr uint8_t AP33772S_REG_OPMODE = 0x03;

void AP33772SBinarySensorComponent::setup() { this->update(); }

void AP33772SBinarySensorComponent::update() {
  if (this->parent_ == nullptr || this->parent_->is_failed()) {
    ESP_LOGW(TAG, "AP33772S hub is not ready");
    this->status_set_warning();
    return;
  }

  bool ok = true;

  uint8_t opmode;
  if (this->pd_connected_ != nullptr || this->derating_ != nullptr) {
    if (!this->parent_->read_u8(AP33772S_REG_OPMODE, &opmode)) {
      ok = false;
    }
  }

  if (ok && this->pd_connected_ != nullptr) {
    bool connected = (opmode & 0x02) != 0;
    ESP_LOGD(TAG, "OPMODE raw=0x%02X → PD connected=%s", opmode, YESNO(connected));
    this->pd_connected_->publish_state(connected);
  }

  if (ok && this->derating_ != nullptr) {
    bool active = (opmode & 0x40) != 0;
    ESP_LOGD(TAG, "OPMODE raw=0x%02X → Derating=%s", opmode, YESNO(active));
    this->derating_->publish_state(active);
  }

  uint8_t faults = this->parent_->get_latched_faults();
  if (this->fault_otp_ != nullptr) {
    this->fault_otp_->publish_state((faults >> 6) & 1);
  }
  if (this->fault_ocp_ != nullptr) {
    this->fault_ocp_->publish_state((faults >> 5) & 1);
  }
  if (this->fault_ovp_ != nullptr) {
    this->fault_ovp_->publish_state((faults >> 4) & 1);
  }
  if (this->fault_uvp_ != nullptr) {
    this->fault_uvp_->publish_state((faults >> 3) & 1);
  }

  if (ok) {
    this->status_clear_warning();
  } else {
    this->status_set_warning();
  }
}

void AP33772SBinarySensorComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AP33772S Binary Sensors:");
  LOG_UPDATE_INTERVAL(this);
  LOG_BINARY_SENSOR("  ", "PD Connected", this->pd_connected_);
  LOG_BINARY_SENSOR("  ", "Fault OTP", this->fault_otp_);
  LOG_BINARY_SENSOR("  ", "Fault OCP", this->fault_ocp_);
  LOG_BINARY_SENSOR("  ", "Fault OVP", this->fault_ovp_);
  LOG_BINARY_SENSOR("  ", "Fault UVP", this->fault_uvp_);
  LOG_BINARY_SENSOR("  ", "Derating", this->derating_);
}

}  // namespace ap33772s
}  // namespace esphome
