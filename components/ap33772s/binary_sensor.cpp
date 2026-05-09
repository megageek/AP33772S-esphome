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

  if (this->pd_connected_ == nullptr)
    return;

  uint8_t opmode;
  if (!this->parent_->read_u8(AP33772S_REG_OPMODE, &opmode)) {
    this->status_set_warning();
    return;
  }

  bool connected = (opmode & 0x02) != 0;
  ESP_LOGD(TAG, "OPMODE raw=0x%02X → PD connected=%s", opmode, YESNO(connected));
  this->pd_connected_->publish_state(connected);
  this->status_clear_warning();
}

void AP33772SBinarySensorComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AP33772S Binary Sensors:");
  LOG_UPDATE_INTERVAL(this);
  LOG_BINARY_SENSOR("  ", "PD Connected", this->pd_connected_);
}

}  // namespace ap33772s
}  // namespace esphome
