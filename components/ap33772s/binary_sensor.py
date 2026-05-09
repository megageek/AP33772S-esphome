import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID

from . import AP33772SComponent, CONF_AP33772S_ID, ap33772s_ns

DEPENDENCIES = ["ap33772s"]

CONF_PD_CONNECTED = "pd_connected"

AP33772SBinarySensorComponent = ap33772s_ns.class_(
    "AP33772SBinarySensorComponent", cg.PollingComponent
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AP33772SBinarySensorComponent),
            cv.GenerateID(CONF_AP33772S_ID): cv.use_id(AP33772SComponent),
            cv.Optional(CONF_PD_CONNECTED): binary_sensor.binary_sensor_schema(
                device_class="connectivity",
            ),
        }
    ).extend(cv.polling_component_schema("10s")),
    cv.has_at_least_one_key(CONF_PD_CONNECTED),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_AP33772S_ID])
    cg.add(var.set_parent(parent))

    if pd_config := config.get(CONF_PD_CONNECTED):
        sens = await binary_sensor.new_binary_sensor(pd_config)
        cg.add(var.set_pd_connected_sensor(sens))
