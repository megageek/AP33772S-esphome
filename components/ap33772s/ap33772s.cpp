#include "ap33772s.h"

#include "esphome/core/log.h"

namespace esphome {
namespace ap33772s {

static const char *const TAG = "ap33772s";

static constexpr uint8_t AP33772S_REG_STATUS = 0x01;
static constexpr uint8_t AP33772S_REG_OPMODE = 0x03;
static constexpr uint8_t AP33772S_REG_CONFIG = 0x04;
static constexpr uint8_t AP33772S_REG_PDCONFIG = 0x05;

bool AP33772SComponent::read_register_(uint8_t reg, uint8_t *value) {
  if (this->read_byte(reg, value)) {
    return true;
  }

  ESP_LOGE(TAG, "Failed to read register 0x%02X from AP33772S at address 0x%02X", reg, this->get_i2c_address());
  return false;
}

bool AP33772SComponent::read_u8(uint8_t reg, uint8_t *value) { return this->read_register_(reg, value); }

bool AP33772SComponent::read_u16_le(uint8_t reg, uint16_t *value) {
  uint8_t data[2];
  if (!this->read_bytes(reg, data, 2)) {
    ESP_LOGE(TAG, "Failed to read register 0x%02X from AP33772S at address 0x%02X", reg, this->get_i2c_address());
    return false;
  }

  *value = static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
  return true;
}

void AP33772SComponent::setup() {
  ESP_LOGCONFIG(TAG, "Probing AP33772S at address 0x%02X", this->get_i2c_address());

  if (!this->read_register_(AP33772S_REG_STATUS, &this->status_) ||
      !this->read_register_(AP33772S_REG_OPMODE, &this->opmode_) ||
      !this->read_register_(AP33772S_REG_CONFIG, &this->config_) ||
      !this->read_register_(AP33772S_REG_PDCONFIG, &this->pdconfig_)) {
    this->mark_failed();
    return;
  }

  this->detected_ = true;

  ESP_LOGCONFIG(TAG, "AP33772S responded: STATUS=0x%02X, OPMODE=0x%02X, CONFIG=0x%02X, PDCONFIG=0x%02X",
                this->status_, this->opmode_, this->config_, this->pdconfig_);

  ESP_LOGCONFIG(TAG, "  STATUS:");
  ESP_LOGCONFIG(TAG, "    Protection: OTP=%s OCP=%s OVP=%s UVP=%s",
                (this->status_ & 0x40) ? "FAULT" : "OK",
                (this->status_ & 0x20) ? "FAULT" : "OK",
                (this->status_ & 0x10) ? "FAULT" : "OK",
                (this->status_ & 0x08) ? "FAULT" : "OK");
  ESP_LOGCONFIG(TAG, "    NEWPDO=%s READY=%s STARTED=%s",
                (this->status_ & 0x04) ? "YES" : "NO",
                (this->status_ & 0x02) ? "YES" : "NO",
                (this->status_ & 0x01) ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  OPMODE:");
  ESP_LOGCONFIG(TAG, "    CC: %s", (this->opmode_ & 0x80) ? "CC2" : "CC1");
  ESP_LOGCONFIG(TAG, "    De-rating: %s", (this->opmode_ & 0x40) ? "Active" : "Inactive");
  ESP_LOGCONFIG(TAG, "    Data Role: %s", (this->opmode_ & 0x20) ? "DFP" : "UFP");
  ESP_LOGCONFIG(TAG, "    PD Mode: %s", (this->opmode_ & 0x02) ? "Connected" : "Not connected");
  ESP_LOGCONFIG(TAG, "    Legacy Mode: %s", (this->opmode_ & 0x01) ? "Active" : "Inactive");
  ESP_LOGCONFIG(TAG, "  CONFIG protections enabled:");
  ESP_LOGCONFIG(TAG, "    De-rating=%s OTP=%s OCP=%s OVP=%s UVP=%s",
                (this->config_ & 0x80) ? "YES" : "NO",
                (this->config_ & 0x40) ? "YES" : "NO",
                (this->config_ & 0x20) ? "YES" : "NO",
                (this->config_ & 0x10) ? "YES" : "NO",
                (this->config_ & 0x08) ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  PDCONFIG:");
  ESP_LOGCONFIG(TAG, "    EPR Mode=%s PPS/AVS=%s DR Swap=%s",
                (this->pdconfig_ & 0x01) ? "Enabled" : "Disabled",
                (this->pdconfig_ & 0x02) ? "Enabled" : "Disabled",
                (this->pdconfig_ & 0x04) ? "Enabled" : "Disabled");

  if ((this->status_ & 0x80) != 0) {
    ESP_LOGW(TAG, "STATUS reserved bit 7 is set; device response is unexpected");
    this->status_set_warning();
  }
  if ((this->opmode_ & 0x1C) != 0) {
    ESP_LOGW(TAG, "OPMODE reserved bits 4:2 are set; device response is unexpected");
    this->status_set_warning();
  }
  if ((this->pdconfig_ & 0xF8) != 0) {
    ESP_LOGW(TAG, "PDCONFIG reserved bits 7:3 are set; device response is unexpected");
    this->status_set_warning();
  }
}

void AP33772SComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AP33772S:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Communication with AP33772S failed");
    return;
  }
  ESP_LOGCONFIG(TAG, "  Detected: %s", YESNO(this->detected_));
  ESP_LOGCONFIG(TAG, "  STATUS: 0x%02X", this->status_);
  ESP_LOGCONFIG(TAG, "  OPMODE: 0x%02X", this->opmode_);
  ESP_LOGCONFIG(TAG, "  CONFIG: 0x%02X", this->config_);
  ESP_LOGCONFIG(TAG, "  PDCONFIG: 0x%02X", this->pdconfig_);
}

}  // namespace ap33772s
}  // namespace esphome
