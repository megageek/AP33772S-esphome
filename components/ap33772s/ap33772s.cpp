#include "ap33772s.h"

#include "esphome/core/log.h"

namespace esphome {
namespace ap33772s {

static const char *const TAG = "ap33772s";

void AP33772SComponent::setup() {}

void AP33772SComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AP33772S:");
  LOG_I2C_DEVICE(this);
}

}  // namespace ap33772s
}  // namespace esphome
