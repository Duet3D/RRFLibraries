# RRFLibraries SAME51_RTOS Configuration Makefile

SAME51_RTOS_BUILD_DIR := SAME51_RTOS
SAME51_RTOS_TARGET := $(SAME51_RTOS_BUILD_DIR)/libRRFLibraries.a

SAME51_RTOS_SRC_DIR := src

SAME51_RTOS_CPP_SRCS := $(shell find $(SAME51_RTOS_SRC_DIR) -name '*.cpp' ! -path '*/RP2040/*')

SAME51_RTOS_INCLUDES := \
	-I$(SAME51_RTOS_SRC_DIR) \
	-I../FreeRTOS/src/include \
	-I../FreeRTOS/src/portable/GCC/ARM_CM4F

SAME51_RTOS_DEFINES := \
	-D__SAME51N19A__ \
	-DRTOS

SAME51_RTOS_CXXFLAGS := -c -std=c++20 \
	-mcpu=cortex-m4 \
	-mthumb \
	-fno-math-errno \
	-mfpu=fpv4-sp-d16 \
	-mfloat-abi=hard \
	-mfp16-format=ieee \
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
	-O2 \
	$(SAME51_RTOS_INCLUDES) \
	$(SAME51_RTOS_DEFINES)

SAME51_RTOS_CXXFLAGS += $(DEBUG_FLAGS)

SAME51_RTOS_OBJS := $(SAME51_RTOS_CPP_SRCS:%.cpp=$(SAME51_RTOS_BUILD_DIR)/%.o)
SAME51_RTOS_DEPS := $(SAME51_RTOS_OBJS:.o=.d)

.PHONY: SAME51_RTOS
SAME51_RTOS: $(SAME51_RTOS_TARGET)

$(SAME51_RTOS_TARGET): $(SAME51_RTOS_OBJS)
	$(Q)echo "  AR      $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(AR) rcs $@ $^

$(SAME51_RTOS_BUILD_DIR)/%.o: %.cpp
	$(Q)echo "  CXX     $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) $(SAME51_RTOS_CXXFLAGS) -MMD -MP -o $@ $<

-include $(SAME51_RTOS_DEPS)

.PHONY: clean-SAME51_RTOS
clean-SAME51_RTOS:
	$(Q)echo "  RM      $(SAME51_RTOS_BUILD_DIR)"
	$(Q)rm -rf $(SAME51_RTOS_BUILD_DIR)
