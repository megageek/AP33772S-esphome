#include "sensor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace ap33772s {

static const char *const TAG = "ap33772s.sensor";

static constexpr uint8_t AP33772S_REG_VOLTAGE = 0x11;
static constexpr uint8_t AP33772S_REG_CURRENT = 0x12;
static constexpr uint8_t AP33772S_REG_TEMP = 0x13;
static constexpr uint8_t AP33772S_REG_VREQ = 0x14;
static constexpr uint8_t AP33772S_REG_IREQ = 0x15;

void AP33772SSensorComponent::setup() {
  this->update();
}

void AP33772SSensorComponent::update() {
  if (this->parent_ == nullptr || this->parent_->is_failed()) {
    ESP_LOGW(TAG, "AP33772S hub is not ready");
    this->status_set_warning();
    return;
  }

  bool success = true;
  if (this->voltage_sensor_ != nullptr) {
    success &= this->publish_voltage_();
  }
  if (this->current_sensor_ != nullptr) {
    success &= this->publish_current_();
  }
  if (this->temperature_sensor_ != nullptr) {
    success &= this->publish_temperature_();
  }
  if (this->voltage_requested_sensor_ != nullptr) {
    success &= this->publish_voltage_requested_();
  }
  if (this->current_requested_sensor_ != nullptr) {
    success &= this->publish_current_requested_();
  }

  if (success) {
    this->status_clear_warning();
  } else {
    this->status_set_warning();
  }
}

void AP33772SSensorComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AP33772S Sensors:");
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("  ", "Current", this->current_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Requested Voltage", this->voltage_requested_sensor_);
  LOG_SENSOR("  ", "Requested Current", this->current_requested_sensor_);
}

bool AP33772SSensorComponent::publish_voltage_() {
  uint16_t raw;
  if (!this->parent_->read_u16_le(AP33772S_REG_VOLTAGE, &raw)) {
    return false;
  }

  float voltage = static_cast<float>(raw) * 0.080f;
  ESP_LOGD(TAG, "Voltage raw=%u → %.2f V", raw, voltage);
  this->voltage_sensor_->publish_state(voltage);
  return true;
}

bool AP33772SSensorComponent::publish_current_() {
  uint8_t raw;
  if (!this->parent_->read_u8(AP33772S_REG_CURRENT, &raw)) {
    return false;
  }

  float current = static_cast<float>(raw) * 0.024f;
  ESP_LOGD(TAG, "Current raw=%u → %.3f A", raw, current);
  this->current_sensor_->publish_state(current);
  return true;
}

bool AP33772SSensorComponent::publish_temperature_() {
  uint8_t raw;
  if (!this->parent_->read_u8(AP33772S_REG_TEMP, &raw)) {
    return false;
  }

  ESP_LOGD(TAG, "Temperature raw=%u → %u °C", raw, raw);
  this->temperature_sensor_->publish_state(static_cast<float>(raw));
  return true;
}

bool AP33772SSensorComponent::publish_voltage_requested_() {
  uint16_t raw;
  if (!this->parent_->read_u16_le(AP33772S_REG_VREQ, &raw)) {
    return false;
  }

  float voltage = static_cast<float>(raw) * 0.050f;
  ESP_LOGD(TAG, "Requested voltage raw=%u → %.2f V", raw, voltage);
  this->voltage_requested_sensor_->publish_state(voltage);
  return true;
}

bool AP33772SSensorComponent::publish_current_requested_() {
  uint16_t raw;
  if (!this->parent_->read_u16_le(AP33772S_REG_IREQ, &raw)) {
    return false;
  }

  float current = static_cast<float>(raw) * 0.010f;
  ESP_LOGD(TAG, "Requested current raw=%u → %.3f A", raw, current);
  this->current_requested_sensor_->publish_state(current);
  return true;
}

}  // namespace ap33772s
}  // namespace esphome
