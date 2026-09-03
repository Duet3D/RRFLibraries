# RRFLibraries STM32H7_RTOS Configuration Makefile

STM32H7_RTOS_BUILD_DIR := STM32H7_RTOS
STM32H7_RTOS_TARGET := $(STM32H7_RTOS_BUILD_DIR)/libRRFLibraries.a

STM32H7_RTOS_SRC_DIR := src

STM32H7_RTOS_CPP_SRCS := $(shell find $(STM32H7_RTOS_SRC_DIR) -name '*.cpp' ! -path '*/RP2040/*')

STM32H7_RTOS_INCLUDES := \
	-I$(STM32H7_RTOS_SRC_DIR) \
	-I../FreeRTOS/src/include \
	-I../FreeRTOS/src/portable/GCC/ARM_CM7/r0p1

STM32H7_RTOS_DEFINES := \
	-DSTM32H743xx \
	-DRTOS

STM32H7_RTOS_CXXFLAGS := -c -std=c++20 \
	-mcpu=cortex-m7 \
	-mthumb \
	-fno-math-errno \
	-mfpu=fpv5-d16 \
	-mfloat-abi=hard \
	-mfp16-format=ieee \
	-mno-unaligned-access \
	-ffunction-sections \
	-fdata-sections \
	-fno-threadsafe-statics \
	-fno-rtti \
	-fno-exceptions \
	-nostdlib \
	-Wundef \
	-Wdouble-promotion \
	-Werror -Wnoexcept -Wshadow -Wsign-promo \
	-fsingle-precision-constant \
	-fstack-usage \
	-fdump-rtl-expand \
	-Wall \
	$(STM32H7_RTOS_INCLUDES) \
	$(STM32H7_RTOS_DEFINES)

STM32H7_RTOS_CXXFLAGS += $(DEBUG_FLAGS)

STM32H7_RTOS_OBJS := $(STM32H7_RTOS_CPP_SRCS:%.cpp=$(STM32H7_RTOS_BUILD_DIR)/%.o)
STM32H7_RTOS_DEPS := $(STM32H7_RTOS_OBJS:.o=.d)

.PHONY: STM32H7_RTOS
STM32H7_RTOS: $(STM32H7_RTOS_TARGET)

$(STM32H7_RTOS_TARGET): $(STM32H7_RTOS_OBJS)
	$(Q)echo "  AR      $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(AR) rcs $@ $^

$(STM32H7_RTOS_BUILD_DIR)/%.o: %.cpp
	$(Q)echo "  CXX     $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) $(STM32H7_RTOS_CXXFLAGS) -MMD -MP -o $@ $<

-include $(STM32H7_RTOS_DEPS)

.PHONY: clean-STM32H7_RTOS
clean-STM32H7_RTOS:
	$(Q)echo "  RM      $(STM32H7_RTOS_BUILD_DIR)"
	$(Q)rm -rf $(STM32H7_RTOS_BUILD_DIR)
