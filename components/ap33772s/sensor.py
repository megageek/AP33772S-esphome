import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_CURRENT,
    CONF_ID,
    CONF_TEMPERATURE,
    CONF_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_VOLT,
)

from . import AP33772SComponent, CONF_AP33772S_ID, ap33772s_ns

DEPENDENCIES = ["ap33772s"]

AP33772SSensorComponent = ap33772s_ns.class_(
    "AP33772SSensorComponent", cg.PollingComponent
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AP33772SSensorComponent),
            cv.GenerateID(CONF_AP33772S_ID): cv.use_id(AP33772SComponent),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    ).extend(cv.polling_component_schema("10s")),
    cv.has_at_least_one_key(CONF_VOLTAGE, CONF_CURRENT, CONF_TEMPERATURE),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_AP33772S_ID])
    cg.add(var.set_parent(parent))

    if voltage_config := config.get(CONF_VOLTAGE):
        sens = await sensor.new_sensor(voltage_config)
        cg.add(var.set_voltage_sensor(sens))

    if current_config := config.get(CONF_CURRENT):
        sens = await sensor.new_sensor(current_config)
        cg.add(var.set_current_sensor(sens))

    if temperature_config := config.get(CONF_TEMPERATURE):
        sens = await sensor.new_sensor(temperature_config)
        cg.add(var.set_temperature_sensor(sens))
