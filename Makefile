# RRFLibraries Master Makefile
# Builds RRFLibraries for various MCU configurations

# Cross-compiler toolchain (relative to project root)
#CROSS_COMPILE ?= ../arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-
CROSS_COMPILE ?= ../arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-
export CROSS_COMPILE

# Toolchain commands
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AS = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

# Quiet build support (Linux kernel style)
# Use V=1 for verbose output
ifeq ($(V),1)
	Q :=
else
	Q := @
endif
export Q

# Available build configurations
CONFIGS := SAME51_RTOS SAME70_RTOS SAMC21_RTOS SAMC21 SAME51 SAME70 RP2040_RTOS STM32H5_RTOS STM32H7_RTOS

# Default target
.DEFAULT_GOAL := SAME70_RTOS

# Print available targets
.PHONY: help
help:
	@echo "RRFLibraries Build System"
	@echo "Available targets:"
	@for config in $(CONFIGS); do echo "  make $$config"; done
	@echo ""
	@echo "Other targets:"
	@echo "  make all          - Build all configurations"
	@echo "  make clean        - Clean all build outputs"
	@echo "  make clean-<config> - Clean specific configuration"
	@echo ""
	@echo "Options:"
	@echo "  V=1               - Verbose build output"
	@echo "  DEBUG=1           - Build with debug symbols and no optimization"
	@echo "  CROSS_COMPILE=$(CROSS_COMPILE)"

# Build all configurations
.PHONY: all
all:
	$(Q)$(MAKE) SAME51_RTOS
	$(Q)$(MAKE) SAME70_RTOS
	$(Q)$(MAKE) SAMC21_RTOS
	$(Q)$(MAKE) STM32H5_RTOS
	$(Q)$(MAKE) STM32H7_RTOS

# Include configuration-specific makefiles only when building that specific config
ifeq ($(MAKECMDGOALS),SAME51_RTOS)
-include Makefiles/SAME51_RTOS.mk
endif
ifeq ($(MAKECMDGOALS),SAME70_RTOS)
-include Makefiles/SAME70_RTOS.mk
endif
ifeq ($(MAKECMDGOALS),SAMC21_RTOS)
-include Makefiles/SAMC21_RTOS.mk
endif
ifeq ($(MAKECMDGOALS),SAMC21)
-include Makefiles/SAMC21.mk
endif
ifeq ($(MAKECMDGOALS),SAME51)
-include Makefiles/SAME51.mk
endif
ifeq ($(MAKECMDGOALS),SAME70)
-include Makefiles/SAME70.mk
endif
ifeq ($(MAKECMDGOALS),RP2040_RTOS)
-include Makefiles/RP2040_RTOS.mk
endif
ifeq ($(MAKECMDGOALS),STM32H5_RTOS)
-include Makefiles/STM32H5_RTOS.mk
endif
ifeq ($(MAKECMDGOALS),STM32H7_RTOS)
-include Makefiles/STM32H7_RTOS.mk
endif

# Generic clean target
.PHONY: clean
clean:
	@echo "Cleaning all RRFLibraries build outputs..."
	@for config in $(CONFIGS); do \
		if [ -d "$$config" ]; then \
			echo "  Cleaning $$config..."; \
			rm -rf "$$config"; \
		fi; \
	done

# Configuration-specific clean targets are defined in each config makefile
