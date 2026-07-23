.PHONY: help
SHELL := /bin/bash
 
# The default target will display help
help:
	@echo "Available targets:"
	@echo
	@grep -E '^[a-zA-Z0-9_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo

setup: ## Setup the environment
	rm -rf .venv && \
	python3 -m venv .venv && \
	source .venv/bin/activate && \
	pip install esphome

# config: ## Generate the configuration
# 	source .venv/bin/activate && \
# 	esphome config esphome-4btn-rev1.yaml

build: build-three build-six ## Build the firmware for both versions

build-three: ## Build the firmware
	source .venv/bin/activate && \
	esphome compile Airtap-Tx/Gen-1/esphome-3btn-rev2.yaml && \
	cp Airtap-Tx/Gen-1/.esphome/build/airtap-esp32-3btn/.pioenvs/airtap-esp32-3btn/firmware.factory.bin firmware.3btn.bin && \
	esphome compile Airtap-Tx/Gen-2/esphome-4btn-rev1.yaml && \
	cp Airtap-Tx/Gen-2/.esphome/build/airtap-esp32-4btn/.pioenvs/airtap-esp32-4btn/firmware.factory.bin firmware.4btn.bin

build-six: ## Build the C6 Versions
	source .venv/bin/activate && \
	esphome compile Airtap-Tx/Gen-1/esphome-3btn-rev3.yaml && \
	cp Airtap-Tx/Gen-1/.esphome/build/airtap-esp32-3btn/.pioenvs/airtap-esp32-3btn/firmware.factory.bin firmware.3btn.bin && \
	esphome compile Airtap-Tx/Gen-2/esphome-4btn-rev2.yaml && \
	cp Airtap-Tx/Gen-2/.esphome/build/airtap-esp32-4btn/.pioenvs/airtap-esp32-4btn/firmware.factory.bin firmware.4btn.bin


flash-4: ## Flash the firmware 4 button device
	source .venv/bin/activate && \
	esphome upload Airtap-Tx/Gen-2/esphome-4btn-rev1.yaml --device /dev/ttyACM0

flash-3: ## Flash the firmware 3 button device
	source .venv/bin/activate && \
	esphome upload Airtap-Tx/Gen-1/esphome-3btn-rev2.yaml --device /dev/ttyACM0

flash4-c6: ## Flash the firmware 4 button device
	source .venv/bin/activate && \
	esphome upload Airtap-Tx/Gen-2/esphome-4btn-rev2.yaml --device /dev/ttyACM0

flash3-c6: ## Flash the firmware 3 button device
	source .venv/bin/activate && \
	esphome upload Airtap-Tx/Gen-1/esphome-3btn-rev3.yaml --device /dev/ttyACM0

flash-wait-3-c6: ## Keeps flashing until killed as soon as device is detected for 3-c6
	@source .venv/bin/activate && \
	PORT=/dev/ttyACM0; \
	YAML=Airtap-Tx/Gen-1/esphome-3btn-rev3.yaml; \
	echo "Waiting for $$PORT (Ctrl+C to stop)..."; \
	while true; do \
		while [[ ! -e $$PORT ]]; do sleep 0.5; done; \
		echo "[$$(date '+%H:%M:%S')] Device detected — flashing 3-btn C6..."; \
		if esphome upload $$YAML --device $$PORT; then \
			echo "[$$(date '+%H:%M:%S')] Flash OK — unplug device for next unit"; \
		else \
			echo "[$$(date '+%H:%M:%S')] Flash FAILED — check device / retry"; \
		fi; \
		while [[ -e $$PORT ]]; do sleep 0.5; done; \
		echo "[$$(date '+%H:%M:%S')] Disconnected — waiting for next device..."; \
	done

flash-wait-4-c6: ## Keeps flashing until killed as soon as device is detected for 4-c6
	@source .venv/bin/activate && \
	PORT=/dev/ttyACM0; \
	YAML=Airtap-Tx/Gen-2/esphome-4btn-rev2.yaml; \
	echo "Waiting for $$PORT (Ctrl+C to stop)..."; \
	while true; do \
		while [[ ! -e $$PORT ]]; do sleep 0.5; done; \
		echo "[$$(date '+%H:%M:%S')] Device detected — flashing 4-btn C6..."; \
		if esphome upload $$YAML --device $$PORT; then \
			echo "[$$(date '+%H:%M:%S')] Flash OK — unplug device for next unit"; \
		else \
			echo "[$$(date '+%H:%M:%S')] Flash FAILED — check device / retry"; \
		fi; \
		while [[ -e $$PORT ]]; do sleep 0.5; done; \
		echo "[$$(date '+%H:%M:%S')] Disconnected — waiting for next device..."; \
	done
