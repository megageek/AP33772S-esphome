import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, DEVICE_CLASS_PROBLEM

from . import AP33772SComponent, CONF_AP33772S_ID, ap33772s_ns

DEPENDENCIES = ["ap33772s"]

CONF_PD_CONNECTED = "pd_connected"
CONF_FAULT_OTP = "fault_otp"
CONF_FAULT_OCP = "fault_ocp"
CONF_FAULT_OVP = "fault_ovp"
CONF_FAULT_UVP = "fault_uvp"
CONF_DERATING = "derating"

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
            cv.Optional(CONF_FAULT_OTP): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_PROBLEM,
            ),
            cv.Optional(CONF_FAULT_OCP): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_PROBLEM,
            ),
            cv.Optional(CONF_FAULT_OVP): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_PROBLEM,
            ),
            cv.Optional(CONF_FAULT_UVP): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_PROBLEM,
            ),
            cv.Optional(CONF_DERATING): binary_sensor.binary_sensor_schema(),
        }
    ).extend(cv.polling_component_schema("10s")),
    cv.has_at_least_one_key(
        CONF_PD_CONNECTED, CONF_FAULT_OTP, CONF_FAULT_OCP,
        CONF_FAULT_OVP, CONF_FAULT_UVP, CONF_DERATING,
    ),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_AP33772S_ID])
    cg.add(var.set_parent(parent))

    for key, setter in (
        (CONF_PD_CONNECTED, "set_pd_connected_sensor"),
        (CONF_FAULT_OTP, "set_fault_otp_sensor"),
        (CONF_FAULT_OCP, "set_fault_ocp_sensor"),
        (CONF_FAULT_OVP, "set_fault_ovp_sensor"),
        (CONF_FAULT_UVP, "set_fault_uvp_sensor"),
        (CONF_DERATING, "set_derating_sensor"),
    ):
        if cfg := config.get(key):
            sens = await binary_sensor.new_binary_sensor(cfg)
            cg.add(getattr(var, setter)(sens))
