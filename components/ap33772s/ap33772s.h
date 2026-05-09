#pragma once

#include <vector>

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"

namespace esphome {
namespace ap33772s {

union PDOData {
  uint16_t raw;
  struct {
    uint8_t voltage_max : 8;
    uint8_t peak_current : 2;
    uint8_t current_max : 4;
    uint8_t type : 1;
    uint8_t detect : 1;
  } fixed;
  struct {
    uint8_t voltage_max : 8;
    uint8_t voltage_min : 2;
    uint8_t current_max : 4;
    uint8_t type : 1;
    uint8_t detect : 1;
  } pps;
  struct {
    uint8_t voltage_max : 8;
    uint8_t voltage_min : 2;
    uint8_t current_max : 4;
    uint8_t type : 1;
    uint8_t detect : 1;
  } avs;
};

struct PowerProfile {
  float target_voltage;
  float target_current;  // -1.0 means "no current requirement"
};

struct PDOInfo {
  uint8_t index;
  float voltage;
  float min_voltage;
  float max_current;
  bool is_fixed;
  bool is_detected;
};

class AP33772SComponent : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void loop() override;
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
  void add_target_profile(float voltage, float current) { this->target_profiles_.push_back({voltage, current}); }
  void set_request_current_limit(bool x) { this->request_current_limit_ = x; }
  void set_keep_alive_interval(uint32_t ms) { this->keep_alive_interval_ms_ = ms; }
  Trigger<> *get_pd_negotiation_success_trigger() { return &this->pd_negotiation_success_trigger_; }
  Trigger<> *get_pd_negotiation_failure_trigger() { return &this->pd_negotiation_failure_trigger_; }
  bool request_power_profile(float voltage, float current);
  uint8_t get_pdo_count() const;
  PDOInfo get_pdo(uint8_t index) const;

 protected:
  bool read_register_(uint8_t reg, uint8_t *value);
  bool write_register_(uint8_t reg, uint8_t value);
  void read_pdos_();
  void log_pdo_(int idx);
  const char *current_range_str_(uint8_t val) const;
  float pdo_voltage_(int idx) const;
  float pdo_min_voltage_(int idx) const;
  float pdo_max_current_(int idx) const;
  bool pdo_is_fixed_(int idx) const;
  bool pdo_is_detected_(int idx) const;
  void write_pd_reqmsg_(uint8_t pdo_index, uint8_t voltage_sel, uint8_t current_sel);
  void request_power_profiles_();
  void send_keep_alive_();

  bool detected_{false};
  uint8_t status_{0};
  uint8_t opmode_{0};
  uint8_t config_{0};
  uint8_t pdconfig_{0};
  PDOData pdos_[13];

  std::vector<PowerProfile> target_profiles_;
  bool request_sent_{false};
  bool request_done_{false};
  bool request_current_limit_{false};
  bool first_loop_{true};
  int msgrlt_retries_{0};
  uint8_t last_pdo_index_{0};
  uint8_t last_voltage_sel_{0};
  uint8_t last_current_sel_{0};
  uint32_t keep_alive_interval_ms_{1000};
  uint32_t last_keep_alive_millis_{0};
  Trigger<> pd_negotiation_success_trigger_;
  Trigger<> pd_negotiation_failure_trigger_;

  uint8_t config_user_{0xF8};
  uint8_t pdconfig_user_{0x03};
  uint8_t uvp_threshold_user_{0x01};
  uint8_t ovp_offset_user_{0x19};
  uint8_t ocp_threshold_user_{0x00};
  uint8_t otp_threshold_user_{0x78};
  uint8_t derating_threshold_user_{0x78};
};

template<typename... Ts> class PowerProfileRequestAction : public Action<Ts...> {
 public:
  PowerProfileRequestAction(AP33772SComponent *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(float, voltage)
  TEMPLATABLE_VALUE(float, current)

  void play(const Ts &...x) {
    auto voltage = this->voltage_.value(x...);
    auto current = this->current_.value_or(x..., -1.0f);
    this->parent_->request_power_profile(voltage, current);
  }

 protected:
  AP33772SComponent *parent_;
};

}  // namespace ap33772s
}  // namespace esphome
