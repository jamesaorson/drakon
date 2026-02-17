SHELL := /bin/bash
.SHELLFLAGS = -e -c
.DEFAULT_GOAL := help
.ONESHELL:
.SILENT:

ifneq (,$(wildcard ./.env))
    include .env
    export
endif

UNAME_S := $(shell uname -s)

.PHONY: clean
clean:
	echo "Cleaning build artifacts..."

.PHONY: setup
setup: setup/glfw setup/vulkan ## Setup development environment

.PHONY: setup/glfw
setup/glfw: ## Install GLFW library
ifeq ($(UNAME_S),Linux)
	sudo apt-get update
	sudo apt-get install -y libglfw3-dev
else ifeq ($(UNAME_S),Darwin)
	brew install glfw
else
	echo "Unsupported OS: $(UNAME_S)"
	exit 1
endif

.PHONY: setup/vulkan
setup/vulkan: ## Install Vulkan SDK
	echo "Installing Vulkan SDK..."

.PHONY: configure
configure: configure/build configure/test ## Configure CMake cache

.PHONY: configure/build
configure/build: ## Configure CMake cache for build
	cmake -S . -B build

.PHONY: configure/test
configure/test: ## Configure CMake cache for testing
	cmake -S . -B test

.PHONY: build
build:
	cmake --build build

list/example:
	ls -1 ./build/examples | grep -E '^exokomodo\.drakon\.examples\.[^\.]+$$' | sed 's/^exokomodo\.drakon\.examples\.//'

run/example/%:
	./build/examples/exokomodo.drakon.examples.$*

.PHONY: test
test:
	cmake --build test
	cd test && ctest

.PHONY: format
format: ## Format code
	find . -type f \( -name "*.h" -o -name "*.cpp" \) -print0 | xargs -0 clang-format -i

env-%: ## Check for env var
	if [ -z "$($*)" ]; then \
		echo "Error: Environment variable '$*' is not set."; \
		exit 1; \
	fi

.PHONY: help
help: ## Displays help info
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)
