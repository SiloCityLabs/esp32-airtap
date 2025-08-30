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
	pip install -U platformio

build: ## Build the firmware for the zigbee device
	source .venv/bin/activate && \
	pio run -e esp32c6

flash: ## Flash the firmware to the zigbee device
	source .venv/bin/activate && \
	pio run -e esp32c6 -t upload

monitor: ## Monitor the firmware on the zigbee device
	source .venv/bin/activate && \
	pio device monitor -b 115200