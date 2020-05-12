import logging
import async_timeout
import asyncio

from aiohttp import FormData

from homeassistant.config_entries import SOURCE_IMPORT
from homeassistant.const import ATTR_TEMPERATURE, TEMP_CELSIUS, TEMP_FAHRENHEIT
from homeassistant.helpers import storage
from homeassistant.helpers.aiohttp_client import async_get_clientsession
from homeassistant.components.climate import ClimateDevice
from homeassistant.components.climate.const import (
                                                    CURRENT_HVAC_OFF,
                                                    CURRENT_HVAC_DRY,
                                                    CURRENT_HVAC_HEAT,
                                                    HVAC_MODE_OFF,
                                                    HVAC_MODE_HEAT,
                                                    PRESET_AWAY,
                                                    PRESET_COMFORT, PRESET_ECO,
                                                    PRESET_NONE,
                                                    SUPPORT_TARGET_TEMPERATURE)

_LOGGER = logging.getLogger(__name__)
_TIMEOUT = 15  # seconds

async def async_setup_platform(hass, config, add_devices, discovery_info=None):
    add_devices([CustomClimateDevice()], True)
    return True

class CustomClimateDevice(ClimateDevice):
    def __init__(self):
        self._current_temperature = 12
        self._target_temperature = 12
        self._hvac_mode = HVAC_MODE_OFF
        self._hvac_action = CURRENT_HVAC_OFF

    @property
    def min_temp(self):
        return 12

    @property
    def max_temp(self):
        return 32

    @property
    def precision(self):
        return 1

    @property
    def current_temperature(self):
        return self._current_temperature

    @property
    def target_temperature(self):
        return self._target_temperature

    @property
    def temperature_unit(self):
        return TEMP_CELSIUS

    @property
    def hvac_action(self):
        return self._hvac_action

    @property
    def supported_features(self):
        return SUPPORT_TARGET_TEMPERATURE

    @property
    def unique_id(self):
        return "custom_pool_heater"

    @property
    def name(self):
        return "Pool"

    @property
    def hvac_modes(self):
        return [
            HVAC_MODE_OFF,
            HVAC_MODE_HEAT
        ]

    @property
    def hvac_mode(self):
        return self._hvac_mode

    async def async_set_temperature(self, **kwargs):
        """Set new target temperatures."""
        if kwargs.get(ATTR_TEMPERATURE) is None:
            return

        try:
            target_temperature = kwargs.get(ATTR_TEMPERATURE)
            _LOGGER.info("Swtting Temperature: %s", target_temperature)
            params = {}
            params["value"] = target_temperature
            websession = async_get_clientsession(self.hass)
            with async_timeout.timeout(_TIMEOUT):
                response = await websession.post('http://192.168.20.15/temperature', data=FormData(params))

            self._target_temperature = target_temperature
            # The status takes a while to set after the POST
            await asyncio.sleep(2)

        except Exception as error:
            _LOGGER.error("Error setting power mode: %s", error)

    async def async_set_hvac_mode(self, hvac_mode):
        try:
            powerOn = hvac_mode == HVAC_MODE_HEAT
            _LOGGER.info("Swtting Power Mode: %s", powerOn)
            params = {}
            params["value"] = powerOn
            websession = async_get_clientsession(self.hass)
            with async_timeout.timeout(_TIMEOUT):
                response = await websession.post('http://192.168.20.15/power', data=FormData(params))

            # The status takes a while to set after the POST
            await asyncio.sleep(2)

        except Exception as error:
            _LOGGER.error("Error setting power mode: %s", error)

    async def async_update(self):
        """Retrieve latest state."""
        try:
            websession = async_get_clientsession(self.hass)
            with async_timeout.timeout(_TIMEOUT):
                response = await websession.get('http://192.168.20.15/status')

            body = await response.json()
            _LOGGER.info("Status update power: %s", body["powerOn"])
            _LOGGER.info("Status update set point temp: %s", body["setPointTemperature"])
            _LOGGER.info("Status update water temp: %s", body["waterTemperature"])
            _LOGGER.info("Status update flow: %s", body["waterFlowing"])
            _LOGGER.info("Status update heating: %s", body["heatingOn"])

            if body["powerOn"]:
                self._hvac_mode = HVAC_MODE_HEAT
            else:
                self._hvac_mode = HVAC_MODE_OFF

            if body["waterFlowing"]:
                if body["heatingOn"]:
                    self._hvac_action = CURRENT_HVAC_HEAT
                else:
                    self._hvac_action = CURRENT_HVAC_DRY
            else:
                self._hvac_action = CURRENT_HVAC_OFF

            self._target_temperature = body["setPointTemperature"]
            self._current_temperature = body["waterTemperature"]

        except Exception as error:
            _LOGGER.error("Error getting status: %s", error)