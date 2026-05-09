import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import i2c
from esphome.const import CONF_ID, CONF_VOLTAGE, CONF_CURRENT

DEPENDENCIES = ["i2c"]

CONF_AP33772S_ID = "ap33772s_id"
CONF_EPR_MODE = "epr_mode"
CONF_PPS_AVS = "pps_avs"
CONF_DR_SWAP = "dr_swap"
CONF_DE_RATING_ENABLE = "de_rating_enable"
CONF_OTP_ENABLE = "otp_enable"
CONF_OCP_ENABLE = "ocp_enable"
CONF_OVP_ENABLE = "ovp_enable"
CONF_UVP_ENABLE = "uvp_enable"
CONF_UVP_THRESHOLD = "uvp_threshold"
CONF_OVP_OFFSET = "ovp_offset"
CONF_OCP_THRESHOLD = "ocp_threshold"
CONF_OTP_THRESHOLD = "otp_threshold"
CONF_DR_THRESHOLD = "derating_threshold"
CONF_TARGET_PROFILES = "target_profiles"
CONF_REQUEST_CURRENT_LIMIT = "request_current_limit"
CONF_ON_PD_NEGOTIATION_SUCCESS = "on_pd_negotiation_success"
CONF_ON_PD_NEGOTIATION_FAILURE = "on_pd_negotiation_failure"
CONF_ON_NEW_PDO = "on_new_pdo"
CONF_KEEP_ALIVE_INTERVAL = "keep_alive_interval"
CONF_HARD_RESET = "hard_reset"
_CONF_PROFILE_VOLTAGE = "voltage"
_CONF_PROFILE_CURRENT = "current"

ap33772s_ns = cg.esphome_ns.namespace("ap33772s")
AP33772SComponent = ap33772s_ns.class_(
    "AP33772SComponent", cg.Component, i2c.I2CDevice
)


def validate_uvp_threshold(value):
    value = cv.one_of(80, 75, 70, int=True)(value)
    return {80: 1, 75: 2, 70: 3}[value]


def validate_ovp_offset(value):
    value = cv.positive_float(value)
    raw = round(value / 0.080)
    if raw > 255:
        raise cv.Invalid(f"OVP offset register value {raw} exceeds max 255")
    return raw


def validate_ocp_threshold(value):
    if isinstance(value, str) and value.strip().lower() == "auto":
        return 0
    value = cv.positive_float(value)
    raw = round(value / 0.050)
    if raw > 255:
        raise cv.Invalid(f"OCP threshold register value {raw} exceeds max 255")
    return raw


PROFILE_SCHEMA = cv.Schema(
    {
        cv.Required(_CONF_PROFILE_VOLTAGE): cv.positive_float,
        cv.Optional(_CONF_PROFILE_CURRENT): cv.positive_float,
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AP33772SComponent),
            cv.Optional(CONF_EPR_MODE, default=True): cv.boolean,
            cv.Optional(CONF_PPS_AVS, default=True): cv.boolean,
            cv.Optional(CONF_DR_SWAP, default=False): cv.boolean,
            cv.Optional(CONF_DE_RATING_ENABLE, default=True): cv.boolean,
            cv.Optional(CONF_OTP_ENABLE, default=True): cv.boolean,
            cv.Optional(CONF_OCP_ENABLE, default=True): cv.boolean,
            cv.Optional(CONF_OVP_ENABLE, default=True): cv.boolean,
            cv.Optional(CONF_UVP_ENABLE, default=True): cv.boolean,
            cv.Optional(CONF_UVP_THRESHOLD, default=80): validate_uvp_threshold,
            cv.Optional(CONF_OVP_OFFSET, default=2.0): validate_ovp_offset,
            cv.Optional(CONF_OCP_THRESHOLD, default="auto"): validate_ocp_threshold,
            cv.Optional(CONF_OTP_THRESHOLD, default=120): cv.int_range(0, 255),
            cv.Optional(CONF_DR_THRESHOLD, default=120): cv.int_range(0, 255),
            cv.Optional(CONF_TARGET_PROFILES): cv.ensure_list(PROFILE_SCHEMA),
            cv.Optional(CONF_REQUEST_CURRENT_LIMIT, default=False): cv.boolean,
            cv.Optional(CONF_KEEP_ALIVE_INTERVAL, default="1000ms"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_ON_PD_NEGOTIATION_SUCCESS): automation.validate_automation(),
            cv.Optional(CONF_ON_PD_NEGOTIATION_FAILURE): automation.validate_automation(),
            cv.Optional(CONF_ON_NEW_PDO): automation.validate_automation(),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x52))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_epr_mode(config[CONF_EPR_MODE]))
    cg.add(var.set_pps_avs(config[CONF_PPS_AVS]))
    cg.add(var.set_dr_swap(config[CONF_DR_SWAP]))
    cg.add(var.set_de_rating_enable(config[CONF_DE_RATING_ENABLE]))
    cg.add(var.set_otp_enable(config[CONF_OTP_ENABLE]))
    cg.add(var.set_ocp_enable(config[CONF_OCP_ENABLE]))
    cg.add(var.set_ovp_enable(config[CONF_OVP_ENABLE]))
    cg.add(var.set_uvp_enable(config[CONF_UVP_ENABLE]))
    cg.add(var.set_uvp_threshold(config[CONF_UVP_THRESHOLD]))
    cg.add(var.set_ovp_offset(config[CONF_OVP_OFFSET]))
    cg.add(var.set_ocp_threshold(config[CONF_OCP_THRESHOLD]))
    cg.add(var.set_otp_threshold(config[CONF_OTP_THRESHOLD]))
    cg.add(var.set_derating_threshold(config[CONF_DR_THRESHOLD]))
    cg.add(var.set_request_current_limit(config[CONF_REQUEST_CURRENT_LIMIT]))
    cg.add(var.set_keep_alive_interval(config[CONF_KEEP_ALIVE_INTERVAL]))

    if target_profiles := config.get(CONF_TARGET_PROFILES):
        for profile in target_profiles:
            voltage = profile[_CONF_PROFILE_VOLTAGE]
            current = profile.get(_CONF_PROFILE_CURRENT, -1.0)
            cg.add(var.add_target_profile(voltage, current))

    for conf in config.get(CONF_ON_PD_NEGOTIATION_SUCCESS, []):
        await automation.build_automation(var.get_pd_negotiation_success_trigger(), [], conf)

    for conf in config.get(CONF_ON_PD_NEGOTIATION_FAILURE, []):
        await automation.build_automation(var.get_pd_negotiation_failure_trigger(), [], conf)

    for conf in config.get(CONF_ON_NEW_PDO, []):
        await automation.build_automation(var.get_on_new_pdo_trigger(), [], conf)


CONF_REQUEST_POWER_PROFILE = "request_power_profile"
PowerProfileRequestAction = ap33772s_ns.class_(
    "PowerProfileRequestAction", automation.Action
)

REQUEST_POWER_PROFILE_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(AP33772SComponent),
        cv.Required(CONF_VOLTAGE): cv.templatable(cv.positive_float),
        cv.Optional(CONF_CURRENT): cv.templatable(cv.positive_float),
    }
)


@automation.register_action(
    CONF_REQUEST_POWER_PROFILE,
    PowerProfileRequestAction,
    REQUEST_POWER_PROFILE_SCHEMA,
    synchronous=True,
)
async def request_power_profile_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_VOLTAGE], args, cg.float_)
    cg.add(var.set_voltage(template_))
    if CONF_CURRENT in config:
        template_ = await cg.templatable(config[CONF_CURRENT], args, cg.float_)
        cg.add(var.set_current(template_))
    return var


HardResetAction = ap33772s_ns.class_("HardResetAction", automation.Action)

HARD_RESET_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(AP33772SComponent),
    }
)


@automation.register_action(
    CONF_HARD_RESET,
    HardResetAction,
    HARD_RESET_SCHEMA,
    synchronous=True,
)
async def hard_reset_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)
