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
  bool write_u8(uint8_t reg, uint8_t value);

  void set_epr_mode(bool x);
  void set_pps_avs(bool x);
  void set_dr_swap(bool x);
  void set_de_rating_enable(bool x);
  void set_otp_enable(bool x);
  void set_ocp_enable(bool x);
  void set_ovp_enable(bool x);
  void set_uvp_enable(bool x);
  void set_uvp_threshold(uint8_t x) { this->uvp_threshold_user_ = x; }
  void set_ovp_offset(uint8_t x) { this->ovp_offset_user_ = x; }
  void set_ocp_threshold(uint8_t x) { this->ocp_threshold_user_ = x; }
  void set_otp_threshold(uint8_t x) { this->otp_threshold_user_ = x; }
  void set_derating_threshold(uint8_t x) { this->derating_threshold_user_ = x; }

 protected:
  bool read_register_(uint8_t reg, uint8_t *value);
  bool write_register_(uint8_t reg, uint8_t value);

  bool detected_{false};
  uint8_t status_{0};
  uint8_t opmode_{0};
  uint8_t config_{0};
  uint8_t pdconfig_{0};

  uint8_t config_user_{0xF8};
  uint8_t pdconfig_user_{0x03};
  uint8_t uvp_threshold_user_{0x01};
  uint8_t ovp_offset_user_{0x19};
  uint8_t ocp_threshold_user_{0x00};
  uint8_t otp_threshold_user_{0x78};
  uint8_t derating_threshold_user_{0x78};
};

}  // namespace ap33772s
}  // namespace esphome
