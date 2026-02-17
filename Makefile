SHELL := /bin/bash
.SHELLFLAGS = -e -c
.DEFAULT_GOAL := help
.ONESHELL:
.SILENT:
MAKEFLAGS += --no-print-directory

ifneq (,$(wildcard ./.env))
    include .env
    export
endif

UNAME_S := $(shell uname -s)

.PHONY: clean
clean:
	cmake --build build --target clean

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
configure: ## Configure CMake cache
	cmake -S . -B build \
		-DEXOKOMODO_DRAKON_BUILD_EXAMPLES=ON \
		-DEXOKOMODO_DRAKON_BUILD_TESTS=ON

.PHONY: build
build:
	cmake --build build \
		--target exokomodo.drakon

.PHONY: list/example
list/example:
	ls ./examples | grep -v '\.txt$$'

run/example/%: build/examples/exokomodo.drakon.examples.%
	./build/examples/exokomodo.drakon.examples.$*

# File pattern rule to build examples
build/examples/exokomodo.drakon.examples.%: examples/%/*.cpp
	cmake \
		--build build \
		--target exokomodo.drakon.examples.$*

.PHONY: test
test: test/all ## Build and run all tests

.PHONY: test/all
test/all: ## Build and run all tests
	cmake --build build
	cd build && ctest

.PHONY: test
test/%: build/tests/exokomodo.drakon.tests.%
	cd build && ctest -R exokomodo.drakon.tests.$*

build/tests/exokomodo.drakon.tests.%: tests/%.cpp
	cmake \
		--build build \
		--target exokomodo.drakon.tests.$*

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
