# RRFLibraries SAME70_RTOS Configuration Makefile

SAME70_RTOS_BUILD_DIR := SAME70_RTOS
SAME70_RTOS_TARGET := $(SAME70_RTOS_BUILD_DIR)/libRRFLibraries.a

SAME70_RTOS_SRC_DIR := src

SAME70_RTOS_CPP_SRCS := $(shell find $(SAME70_RTOS_SRC_DIR) -name '*.cpp' ! -path '*/RP2040/*' ! -path '*/SAME5x_C21/*')

SAME70_RTOS_INCLUDES := \
	-I$(SAME70_RTOS_SRC_DIR) \
	-I../FreeRTOS/src/include \
	-I../FreeRTOS/src/portable/GCC/ARM_CM7/r0p1

SAME70_RTOS_DEFINES := \
	-D__SAME70Q21__ \
	-DRTOS

SAME70_RTOS_CXXFLAGS := -c -std=c++20 \
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
	-O2 \
	-Wall \
	$(SAME70_RTOS_INCLUDES) \
	$(SAME70_RTOS_DEFINES)

SAME70_RTOS_CXXFLAGS += $(DEBUG_FLAGS)

SAME70_RTOS_OBJS := $(SAME70_RTOS_CPP_SRCS:%.cpp=$(SAME70_RTOS_BUILD_DIR)/%.o)
SAME70_RTOS_DEPS := $(SAME70_RTOS_OBJS:.o=.d)

.PHONY: SAME70_RTOS
SAME70_RTOS: $(SAME70_RTOS_TARGET)

$(SAME70_RTOS_TARGET): $(SAME70_RTOS_OBJS)
	$(Q)echo "  AR      $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(AR) rcs $@ $^

$(SAME70_RTOS_BUILD_DIR)/%.o: %.cpp
	$(Q)echo "  CXX     $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) $(SAME70_RTOS_CXXFLAGS) -MMD -MP -o $@ $<

-include $(SAME70_RTOS_DEPS)

.PHONY: clean-SAME70_RTOS
clean-SAME70_RTOS:
	$(Q)echo "  RM      $(SAME70_RTOS_BUILD_DIR)"
	$(Q)rm -rf $(SAME70_RTOS_BUILD_DIR)
