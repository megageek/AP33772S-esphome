ESPHOME ?= .venv/bin/esphome
UV_CACHE_DIR ?= /tmp/ap33772s-uv-cache
PLATFORMIO_CORE_DIR ?= $(CURDIR)/.platformio
DEVICE ?= /dev/ttyUSB0

EXAMPLES := examples/ap33772s-component.yaml examples/ap33772s-d1-mini.yaml
D1_MINI_CONFIG := examples/ap33772s-d1-mini.yaml

.PHONY: config compile compile-d1-mini hardware-run hardware-logs

config:
	@for config in $(EXAMPLES); do \
		echo "==> Validating $$config"; \
		$(ESPHOME) config "$$config"; \
	done

compile:
	@for config in $(EXAMPLES); do \
		echo "==> Compiling $$config"; \
		UV_CACHE_DIR="$(UV_CACHE_DIR)" PLATFORMIO_CORE_DIR="$(PLATFORMIO_CORE_DIR)" \
			$(ESPHOME) compile "$$config"; \
	done

compile-d1-mini:
	UV_CACHE_DIR="$(UV_CACHE_DIR)" PLATFORMIO_CORE_DIR="$(PLATFORMIO_CORE_DIR)" \
		$(ESPHOME) compile "$(D1_MINI_CONFIG)"

hardware-run:
	UV_CACHE_DIR="$(UV_CACHE_DIR)" PLATFORMIO_CORE_DIR="$(PLATFORMIO_CORE_DIR)" \
		$(ESPHOME) run "$(D1_MINI_CONFIG)" --device "$(DEVICE)" --reset

hardware-logs:
	UV_CACHE_DIR="$(UV_CACHE_DIR)" PLATFORMIO_CORE_DIR="$(PLATFORMIO_CORE_DIR)" \
		$(ESPHOME) logs "$(D1_MINI_CONFIG)" --device "$(DEVICE)" --reset
