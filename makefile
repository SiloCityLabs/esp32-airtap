.PHONY: help
SHELL := /bin/bash
 
# The default target will display help
help:
	@echo "Available targets:"
	@echo
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo

setup: ## Setup the environment
	rm -rf .venv && \
	python3 -m venv .venv && \
	source .venv/bin/activate && \
	pip install esphome

config: ## Generate the configuration
	source .venv/bin/activate && \
	esphome config esphome-4btn-rev1.yaml

build: ## Build the firmware
	source .venv/bin/activate && \
	esphome compile Airtap-Tx/Gen-1/esphome-3btn-rev2.yaml && \
	cp Airtap-Tx/Gen-1/.esphome/build/airtap-esp32-3btn/.pioenvs/airtap-esp32-3btn/firmware.factory.bin firmware.3btn.bin && \
	esphome compile Airtap-Tx/Gen-2/esphome-4btn-rev1.yaml && \
	cp Airtap-Tx/Gen-2/.esphome/build/airtap-esp32-4btn/.pioenvs/airtap-esp32-4btn/firmware.factory.bin firmware.4btn.bin

flashfour: ## Flash the firmware 4 button device
	source .venv/bin/activate && \
	esphome upload Airtap-Tx/Gen-2/esphome-4btn-rev1.yaml --device /dev/ttyACM0

flashthree: ## Flash the firmware 3 button device
	source .venv/bin/activate && \
	esphome upload Airtap-Tx/Gen-1/esphome-3btn-rev2.yaml --device /dev/ttyACM0


# pip install -U platformio
# pio project init --board esp32-c6-devkitc-1 --project-option "framework=espidf" --project-dir zigbee-4btn-rev3


build-zigbee: ## Build the firmware for the zigbee device
	source .venv/bin/activate && \
	cd Airtap-Tx/Gen-2/zigbee-4btn-rev3 && \
	pio run -e esp32c6

# First time: fetch SDK as component
# git submodule update --init --recursive


# # Build
# pio run -e esp32c6

# # Flash (replace with your serial port)
# pio run -e esp32c6 -t upload

# # Monitor
# pio device monitor -b 115200