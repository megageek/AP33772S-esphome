#include "ap33772s.h"

#include "esphome/core/log.h"

#include <algorithm>

namespace esphome {
namespace ap33772s {

static const char *const TAG = "ap33772s";

static constexpr uint8_t AP33772S_REG_STATUS = 0x01;
static constexpr uint8_t AP33772S_REG_OPMODE = 0x03;
static constexpr uint8_t AP33772S_REG_CONFIG = 0x04;
static constexpr uint8_t AP33772S_REG_PDCONFIG = 0x05;
static constexpr uint8_t AP33772S_REG_UVPTHR = 0x17;

static const char *uvp_threshold_str(uint8_t raw) {
  switch (raw) {
    case 1:
      return "80%";
    case 2:
      return "75%";
    case 3:
      return "70%";
    default:
      return "unknown";
  }
}

static const char *ocp_threshold_str(uint8_t raw) {
  if (raw == 0)
    return "auto";
  static char buf[16];
  snprintf(buf, sizeof(buf), "%.2fA", raw * 0.050f);
  return buf;
}
static constexpr uint8_t AP33772S_REG_OVPTHR = 0x18;
static constexpr uint8_t AP33772S_REG_OCPTHR = 0x19;
static constexpr uint8_t AP33772S_REG_OTPTHR = 0x1A;
static constexpr uint8_t AP33772S_REG_DRTHR = 0x1B;
static constexpr uint8_t AP33772S_REG_SRCPDO = 0x20;
static constexpr uint8_t AP33772S_REG_PD_REQMSG = 0x31;
static constexpr uint8_t AP33772S_REG_PD_CMDMSG = 0x32;
static constexpr uint8_t AP33772S_REG_PD_MSGRLT = 0x33;

static const char *const SPR_VOLTAGE_MIN[] = {"Reserved", "3300mV~",
                                               "3300mV < VOLTAGE_MIN \xe2\x89\xa4 5000mV", "others"};
static const char *const EPR_VOLTAGE_MIN[] = {"Reserved", "15000mV~",
                                               "15000mV < VOLTAGE_MIN \xe2\x89\xa4 20000mV", "others"};

bool AP33772SComponent::read_register_(uint8_t reg, uint8_t *value) {
  if (this->read_byte(reg, value)) {
    return true;
  }

  ESP_LOGE(TAG, "Failed to read register 0x%02X from AP33772S at address 0x%02X", reg, this->get_i2c_address());
  return false;
}

bool AP33772SComponent::write_register_(uint8_t reg, uint8_t value) {
  if (this->write_byte(reg, value)) {
    return true;
  }

  ESP_LOGE(TAG, "Failed to write register 0x%02X to AP33772S at address 0x%02X", reg, this->get_i2c_address());
  return false;
}

bool AP33772SComponent::read_u8(uint8_t reg, uint8_t *value) { return this->read_register_(reg, value); }

bool AP33772SComponent::write_u8(uint8_t reg, uint8_t value) { return this->write_register_(reg, value); }

bool AP33772SComponent::read_u16_le(uint8_t reg, uint16_t *value) {
  uint8_t data[2];
  if (!this->read_bytes(reg, data, 2)) {
    ESP_LOGE(TAG, "Failed to read register 0x%02X from AP33772S at address 0x%02X", reg, this->get_i2c_address());
    return false;
  }

  *value = static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
  return true;
}

void AP33772SComponent::read_pdos_() {
  uint8_t buf[26];
  if (!this->read_bytes(AP33772S_REG_SRCPDO, buf, 26)) {
    ESP_LOGE(TAG, "Failed to read source PDOs");
    return;
  }
  for (int i = 0; i < 13; i++) {
    this->pdos_[i].raw = static_cast<uint16_t>(buf[2 * i]) |
                         (static_cast<uint16_t>(buf[2 * i + 1]) << 8);
  }
  ESP_LOGCONFIG(TAG, "  Source capabilities:");
  for (int i = 0; i < 13; i++) {
    this->log_pdo_(i);
  }
}

void AP33772SComponent::log_pdo_(int idx) {
  PDOData pdo = this->pdos_[idx];
  if (pdo.raw == 0)
    return;

  bool is_epr = (idx >= 7);
  const char *prefix = is_epr ? "EPR" : "SPR";

  if (pdo.fixed.type == 0) {
    uint16_t mv = pdo.fixed.voltage_max * (is_epr ? 200 : 100);
    ESP_LOGCONFIG(TAG, "    PDO%d (%s): Fixed PDO: %dmV, current=%s", idx + 1, prefix, mv,
                  this->current_range_str_(pdo.fixed.current_max));
  } else {
    uint16_t mv = pdo.avs.voltage_max * (is_epr ? 200 : 100);
    const char *min_str = is_epr ? EPR_VOLTAGE_MIN[pdo.avs.voltage_min & 0x03]
                                 : SPR_VOLTAGE_MIN[pdo.pps.voltage_min & 0x03];
    const char *type_str = is_epr ? "AVS" : "PPS";
    ESP_LOGCONFIG(TAG, "    PDO%d (%s): %s PDO: min=%s, max=%dmV, current=%s", idx + 1, prefix, type_str,
                  min_str, mv, this->current_range_str_(pdo.avs.current_max));
  }
}

const char *AP33772SComponent::current_range_str_(uint8_t val) const {
  switch (val) {
    case 0:
      return "0.00A ~ 1.24A";
    case 1:
      return "1.25A ~ 1.49A";
    case 2:
      return "1.50A ~ 1.74A";
    case 3:
      return "1.75A ~ 1.99A";
    case 4:
      return "2.00A ~ 2.24A";
    case 5:
      return "2.25A ~ 2.49A";
    case 6:
      return "2.50A ~ 2.74A";
    case 7:
      return "2.75A ~ 2.99A";
    case 8:
      return "3.00A ~ 3.24A";
    case 9:
      return "3.25A ~ 3.49A";
    case 10:
      return "3.50A ~ 3.74A";
    case 11:
      return "3.75A ~ 3.99A";
    case 12:
      return "4.00A ~ 4.24A";
    case 13:
      return "4.25A ~ 4.49A";
    case 14:
      return "4.50A ~ 4.99A";
    case 15:
      return "5.00A ~";
    default:
      return "Invalid";
  }
}

float AP33772SComponent::pdo_voltage_(int idx) const {
  bool is_epr = (idx >= 7);
  float step = is_epr ? 0.2f : 0.1f;
  return (this->pdos_[idx].raw & 0xFF) * step;
}

float AP33772SComponent::pdo_min_voltage_(int idx) const {
  if (this->pdo_is_fixed_(idx))
    return this->pdo_voltage_(idx);

  uint8_t vmin = (this->pdos_[idx].raw >> 8) & 0x03;
  if (idx >= 7) {
    if (vmin == 1) return 15.0f;
    if (vmin == 2) return 15.0f;
    return 0.0f;
  }
  if (vmin == 1) return 3.3f;
  if (vmin == 2) return 3.3f;
  return 0.0f;
}

float AP33772SComponent::pdo_max_current_(int idx) const {
  uint8_t val = this->pdo_current_sel_(idx);
  if (val == 0) return 1.25f;
  if (val == 15) return 5.0f;
  return 1.25f + 0.25f * val;
}

uint8_t AP33772SComponent::pdo_current_sel_(int idx) const {
  return (this->pdos_[idx].raw >> 10) & 0x0F;
}

bool AP33772SComponent::pdo_is_fixed_(int idx) const {
  return ((this->pdos_[idx].raw >> 14) & 1) == 0;
}

bool AP33772SComponent::pdo_is_detected_(int idx) const {
  return ((this->pdos_[idx].raw >> 15) & 1) != 0;
}

uint8_t AP33772SComponent::get_pdo_count() const {
  uint8_t count = 0;
  for (int i = 0; i < 13; i++) {
    if (this->pdo_is_detected_(i))
      count++;
  }
  return count;
}

PDOInfo AP33772SComponent::get_pdo(uint8_t index) const {
  PDOInfo info{};
  info.index = index + 1;
  info.is_detected = this->pdo_is_detected_(index);
  if (!info.is_detected)
    return info;
  info.voltage = this->pdo_voltage_(index);
  info.min_voltage = this->pdo_min_voltage_(index);
  info.max_current = this->pdo_max_current_(index);
  info.is_fixed = this->pdo_is_fixed_(index);
  return info;
}

void AP33772SComponent::write_pd_reqmsg_(uint8_t pdo_index, uint8_t voltage_sel, uint8_t current_sel) {
  uint8_t data[2] = {voltage_sel, static_cast<uint8_t>((pdo_index << 4) | current_sel)};
  if (!this->write_bytes(AP33772S_REG_PD_REQMSG, data, 2)) {
    ESP_LOGE(TAG, "Failed to write PD_REQMSG");
    this->request_sent_ = false;
  }
}

bool AP33772SComponent::request_power_profile(float voltage, float current) {
  for (int i = 0; i < 13; i++) {
    if (!this->pdo_is_detected_(i))
      continue;

    float pdo_v = this->pdo_voltage_(i);
    float step = (i >= 7) ? 0.2f : 0.1f;

    bool voltage_match;
    if (this->pdo_is_fixed_(i)) {
      voltage_match = (pdo_v - voltage) <= step && (voltage - pdo_v) <= step;
    } else {
      float min_v = this->pdo_min_voltage_(i);
      voltage_match = voltage >= min_v - 0.01f && voltage <= pdo_v + 0.01f;
    }

    if (!voltage_match)
      continue;

    if (current >= 0.0f) {
      if (this->pdo_max_current_(i) < current - 0.01f)
        continue;
    }

    bool is_fixed = this->pdo_is_fixed_(i);
    bool is_epr = (i >= 7);

    uint8_t voltage_sel;
    if (is_fixed) {
      voltage_sel = 0xFF;
    } else if (is_epr) {
      voltage_sel = static_cast<uint8_t>(voltage / 0.2f);
    } else {
      voltage_sel = static_cast<uint8_t>(voltage / 0.1f);
    }

    uint8_t current_sel = this->pdo_current_sel_(i);
    if (this->request_current_limit_ && current >= 0.0f) {
      if (current >= 5.0f) {
        current_sel = 0x0F;
      } else if (current <= 1.0f) {
        current_sel = 0x00;
      } else {
        current_sel = static_cast<uint8_t>((current - 1.0f) * 3.75f + 0.5f);
      }
      current_sel = std::min(current_sel, this->pdo_current_sel_(i));
    }

    uint16_t reqmsg = static_cast<uint16_t>(voltage_sel) |
                      (static_cast<uint16_t>(current_sel) << 8) |
                      (static_cast<uint16_t>(i + 1) << 12);
    ESP_LOGCONFIG(TAG, "  Requesting PDO%d (%s, target=%.1fV, max=%.1fV, cur_sel=%u, vol_sel=%u, "
                       "PD_REQMSG=0x%04X)",
                  i + 1, is_fixed ? "Fixed" : (is_epr ? "AVS" : "PPS"), voltage, pdo_v, current_sel,
                  voltage_sel, reqmsg);
    this->last_pdo_index_ = i + 1;
    this->last_voltage_sel_ = voltage_sel;
    this->last_current_sel_ = current_sel;
    this->write_pd_reqmsg_(i + 1, voltage_sel, current_sel);
    this->request_sent_ = true;
    this->msgrlt_retries_ = 0;
    this->request_done_ = false;
    return true;
  }

  return false;
}

void AP33772SComponent::request_power_profiles_() {
  bool any_pdo = false;
  for (int i = 0; i < 13; i++) {
    if (this->pdo_is_detected_(i)) {
      any_pdo = true;
      break;
    }
  }

  if (!any_pdo) {
    for (const auto &profile : this->target_profiles_) {
      if (fabs(profile.target_voltage - 5.0f) <= 0.1f) {
        ESP_LOGCONFIG(TAG, "  No PDOs (non-PD charger), using default 5V");
        this->request_sent_ = true;
        this->use_default_5v_ = true;
        return;
      }
    }
    ESP_LOGW(TAG, "  No PDOs detected yet, cannot request power profile");
    return;
  }

  for (const auto &profile : this->target_profiles_) {
    if (this->request_power_profile(profile.target_voltage, profile.target_current))
      return;
  }

  ESP_LOGW(TAG, "  No matching PDO found for any target profile");
}

void AP33772SComponent::send_keep_alive_() {
  if (this->last_pdo_index_ == 0)
    return;

  uint8_t data[2] = {this->last_voltage_sel_,
                     static_cast<uint8_t>((this->last_pdo_index_ << 4) | this->last_current_sel_)};
  if (!this->write_bytes(AP33772S_REG_PD_REQMSG, data, 2)) {
    ESP_LOGW(TAG, "  Keep-alive: failed to write PD_REQMSG");
  }
}

void AP33772SComponent::hard_reset() {
  ESP_LOGCONFIG(TAG, "  Issuing hard reset");
  if (!this->write_register_(AP33772S_REG_PD_CMDMSG, 0x01)) {
    ESP_LOGE(TAG, "  Failed to write PD_CMDMSG for hard reset");
  }
  this->request_sent_ = false;
  this->request_done_ = true;
}

void AP33772SComponent::loop() {
  {
    uint8_t status;
    if (this->read_register_(AP33772S_REG_STATUS, &status)) {
      this->latched_faults_ |= status & 0x78;
      if (status & 0x04)
        this->new_pdo_pending_ = true;
    }
  }

  if (this->initial_negotiation_pending_) {
    uint32_t now = millis();
    if (now - this->setup_millis_ < this->initial_negotiation_delay_ms_)
      return;

    this->initial_negotiation_pending_ = false;
    this->first_loop_ = false;
    ESP_LOGCONFIG(TAG, "  Starting deferred initial PD negotiation");
    this->request_power_profiles_();

    if (this->use_default_5v_) {
      ESP_LOGCONFIG(TAG, "  Using default 5V, firing success trigger");
      this->latched_faults_ = 0;
      this->pd_negotiation_success_trigger_.trigger();
      this->request_done_ = true;
      return;
    } else if (!this->request_sent_) {
      ESP_LOGW(TAG, "  No matching PDO found, firing failure trigger");
      this->pd_negotiation_failure_trigger_.trigger();
      this->request_done_ = true;
      return;
    }
    return;
  }

  if (this->request_done_) {
    if (this->new_pdo_pending_) {
      this->new_pdo_pending_ = false;
      ESP_LOGCONFIG(TAG, "  NEWPDO detected, re-reading capabilities");
      this->read_pdos_();
      this->on_new_pdo_trigger_.trigger();

      if (this->target_profiles_.empty())
        return;

      this->request_sent_ = false;
      this->request_done_ = false;
      this->use_default_5v_ = false;
      this->last_pdo_index_ = 0;

      this->request_power_profiles_();

      if (this->use_default_5v_) {
        ESP_LOGCONFIG(TAG, "  NEWPDO: using default 5V");
        this->latched_faults_ = 0;
        this->pd_negotiation_success_trigger_.trigger();
        this->request_done_ = true;
      } else if (!this->request_sent_) {
        ESP_LOGW(TAG, "  NEWPDO: no matching PDO found, firing failure trigger");
        this->pd_negotiation_failure_trigger_.trigger();
        this->request_done_ = true;
      }
      return;
    }

    if (this->keep_alive_interval_ms_ > 0 && this->last_pdo_index_ > 0) {
      uint32_t now = millis();
      if (now - this->last_keep_alive_millis_ >= this->keep_alive_interval_ms_) {
        this->last_keep_alive_millis_ = now;
        this->send_keep_alive_();
      }
    }
    return;
  }

  if (this->first_loop_) {
    this->first_loop_ = false;
    if (this->use_default_5v_) {
      ESP_LOGCONFIG(TAG, "  Using default 5V, firing success trigger");
      this->latched_faults_ = 0;
      this->pd_negotiation_success_trigger_.trigger();
      this->request_done_ = true;
      return;
    }
    if (!this->target_profiles_.empty() && !this->request_sent_) {
      ESP_LOGW(TAG, "  No matching PDO found, firing failure trigger");
      this->pd_negotiation_failure_trigger_.trigger();
      this->request_done_ = true;
      return;
    }
  }

  if (!this->request_sent_)
    return;

  if (this->msgrlt_retries_ > 50) {
    ESP_LOGW(TAG, "  Power profile request timed out");
    this->pd_negotiation_failure_trigger_.trigger();
    this->request_done_ = true;
    return;
  }
  this->msgrlt_retries_++;

  uint8_t msgrlt;
  if (!this->read_register_(AP33772S_REG_PD_MSGRLT, &msgrlt))
    return;

  if (msgrlt & 0x01) {
    ESP_LOGCONFIG(TAG, "  Power profile request accepted (PD_MSGRLT=0x%02X)", msgrlt);
    this->latched_faults_ = 0;
    this->pd_negotiation_success_trigger_.trigger();
    this->request_done_ = true;
  }
}

void AP33772SComponent::set_epr_mode(bool x) {
  if (x)
    this->pdconfig_user_ |= 0x01;
  else
    this->pdconfig_user_ &= ~0x01;
}

void AP33772SComponent::set_pps_avs(bool x) {
  if (x)
    this->pdconfig_user_ |= 0x02;
  else
    this->pdconfig_user_ &= ~0x02;
}

void AP33772SComponent::set_dr_swap(bool x) {
  if (x)
    this->pdconfig_user_ |= 0x04;
  else
    this->pdconfig_user_ &= ~0x04;
}

void AP33772SComponent::set_de_rating_enable(bool x) {
  if (x)
    this->config_user_ |= 0x80;
  else
    this->config_user_ &= ~0x80;
}

void AP33772SComponent::set_otp_enable(bool x) {
  if (x)
    this->config_user_ |= 0x40;
  else
    this->config_user_ &= ~0x40;
}

void AP33772SComponent::set_ocp_enable(bool x) {
  if (x)
    this->config_user_ |= 0x20;
  else
    this->config_user_ &= ~0x20;
}

void AP33772SComponent::set_ovp_enable(bool x) {
  if (x)
    this->config_user_ |= 0x10;
  else
    this->config_user_ &= ~0x10;
}

void AP33772SComponent::set_uvp_enable(bool x) {
  if (x)
    this->config_user_ |= 0x08;
  else
    this->config_user_ &= ~0x08;
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

  uint8_t apply_config = (this->config_ & 0x07) | (this->config_user_ & 0xF8);
  if (apply_config != this->config_) {
    this->write_register_(AP33772S_REG_CONFIG, apply_config);
    this->config_ = apply_config;
    ESP_LOGCONFIG(TAG, "  CONFIG applied: 0x%02X", apply_config);
  }

  uint8_t apply_pdconfig = (this->pdconfig_ & 0xF8) | (this->pdconfig_user_ & 0x07);
  if (apply_pdconfig != this->pdconfig_) {
    this->write_register_(AP33772S_REG_PDCONFIG, apply_pdconfig);
    this->pdconfig_ = apply_pdconfig;
    ESP_LOGCONFIG(TAG, "  PDCONFIG applied: 0x%02X", apply_pdconfig);
  }

  this->write_register_(AP33772S_REG_UVPTHR, this->uvp_threshold_user_);
  this->write_register_(AP33772S_REG_OVPTHR, this->ovp_offset_user_);
  this->write_register_(AP33772S_REG_OCPTHR, this->ocp_threshold_user_);
  this->write_register_(AP33772S_REG_OTPTHR, this->otp_threshold_user_);
  this->write_register_(AP33772S_REG_DRTHR, this->derating_threshold_user_);

  ESP_LOGCONFIG(TAG, "  Thresholds: UVP=%s, OVP=%.1fV, OCP=%s, OTP=%d°C, DR=%d°C",
                uvp_threshold_str(this->uvp_threshold_user_),
                this->ovp_offset_user_ * 0.080f,
                ocp_threshold_str(this->ocp_threshold_user_),
                this->otp_threshold_user_,
                this->derating_threshold_user_);

  this->read_pdos_();
  this->setup_millis_ = millis();
  if (!this->target_profiles_.empty() &&
      (this->defer_initial_negotiation_ || this->initial_negotiation_delay_ms_ > 0)) {
    this->initial_negotiation_pending_ = true;
    ESP_LOGCONFIG(TAG, "  Initial PD negotiation deferred by %u ms", this->initial_negotiation_delay_ms_);
  } else {
    this->request_power_profiles_();
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
  ESP_LOGCONFIG(TAG, "  De-rating: %s", (this->config_ & 0x80) ? "ENABLED" : "DISABLED");
  ESP_LOGCONFIG(TAG, "  OTP: %s", (this->config_ & 0x40) ? "ENABLED" : "DISABLED");
  ESP_LOGCONFIG(TAG, "  OCP: %s", (this->config_ & 0x20) ? "ENABLED" : "DISABLED");
  ESP_LOGCONFIG(TAG, "  OVP: %s", (this->config_ & 0x10) ? "ENABLED" : "DISABLED");
  ESP_LOGCONFIG(TAG, "  UVP: %s", (this->config_ & 0x08) ? "ENABLED" : "DISABLED");
  ESP_LOGCONFIG(TAG, "  EPR Mode: %s", (this->pdconfig_ & 0x01) ? "ENABLED" : "DISABLED");
  ESP_LOGCONFIG(TAG, "  PPS/AVS: %s", (this->pdconfig_ & 0x02) ? "ENABLED" : "DISABLED");
  ESP_LOGCONFIG(TAG, "  DR Swap: %s", (this->pdconfig_ & 0x04) ? "ENABLED" : "DISABLED");
  ESP_LOGCONFIG(TAG, "  UVP threshold: %s", uvp_threshold_str(this->uvp_threshold_user_));
  ESP_LOGCONFIG(TAG, "  OVP offset: %.1f V", this->ovp_offset_user_ * 0.080f);
  ESP_LOGCONFIG(TAG, "  OCP threshold: %s", ocp_threshold_str(this->ocp_threshold_user_));
  ESP_LOGCONFIG(TAG, "  OTP threshold: %d °C", this->otp_threshold_user_);
  ESP_LOGCONFIG(TAG, "  Derating threshold: %d °C", this->derating_threshold_user_);
  ESP_LOGCONFIG(TAG, "  Source capabilities:");
  for (int i = 0; i < 13; i++) {
    this->log_pdo_(i);
  }
  ESP_LOGCONFIG(TAG, "  Request current limit: %s", YESNO(this->request_current_limit_));
  ESP_LOGCONFIG(TAG, "  Defer initial negotiation: %s", YESNO(this->defer_initial_negotiation_));
  if (this->initial_negotiation_delay_ms_ > 0) {
    ESP_LOGCONFIG(TAG, "  Initial negotiation delay: %u ms", this->initial_negotiation_delay_ms_);
  }
  if (this->keep_alive_interval_ms_ > 0) {
    ESP_LOGCONFIG(TAG, "  Keep-alive interval: %u ms", this->keep_alive_interval_ms_);
  } else {
    ESP_LOGCONFIG(TAG, "  Keep-alive: disabled");
  }
  if (!this->target_profiles_.empty()) {
    ESP_LOGCONFIG(TAG, "  Target profiles:");
    for (const auto &profile : this->target_profiles_) {
      if (profile.target_current >= 0.0f) {
        ESP_LOGCONFIG(TAG, "    %.1fV %.2fA", profile.target_voltage, profile.target_current);
      } else {
        ESP_LOGCONFIG(TAG, "    %.1fV", profile.target_voltage);
      }
    }
  }
}

}  // namespace ap33772s
}  // namespace esphome
