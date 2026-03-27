# RRFLibraries SAME70 (non-RTOS) Configuration Makefile

SAME70_BUILD_DIR := SAME70
SAME70_TARGET := $(SAME70_BUILD_DIR)/libRRFLibraries.a

SAME70_SRC_DIR := src

SAME70_CPP_SRCS := $(shell find $(SAME70_SRC_DIR) -name '*.cpp' ! -path '*/RP2040/*')

SAME70_INCLUDES := \
	-I$(SAME70_SRC_DIR)

SAME70_DEFINES := \
	-D__SAME70Q21__

SAME70_CXXFLAGS := -c -std=gnu++17 \
	-mcpu=cortex-m7 \
	-mthumb \
	-mfpu=fpv5-d16 \
	-mfloat-abi=hard \
	-mno-unaligned-access \
	-fno-math-errno \
	-mfp16-format=ieee \
	-ffunction-sections \
	-fdata-sections \
	-fno-threadsafe-statics \
	-fno-rtti \
	-fno-exceptions \
	-nostdlib \
	-Wundef \
	-Wdouble-promotion \
	-fsingle-precision-constant \
	$(SAME70_INCLUDES) \
	$(SAME70_DEFINES)

# Add debug flags if DEBUG=1
ifeq ($(DEBUG),1)
	SAME70_CXXFLAGS += -O0 -g3
else
	SAME70_CXXFLAGS += -Os
endif

SAME70_OBJS := $(SAME70_CPP_SRCS:%.cpp=$(SAME70_BUILD_DIR)/%.o)
SAME70_DEPS := $(SAME70_OBJS:.o=.d)

.PHONY: SAME70
SAME70: $(SAME70_TARGET)

$(SAME70_TARGET): $(SAME70_OBJS)
	$(Q)echo "  AR      $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(AR) rcs $@ $^
	$(Q)echo "Built $@"

$(SAME70_BUILD_DIR)/%.o: %.cpp
	$(Q)echo "  CXX     $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) $(SAME70_CXXFLAGS) -MMD -MP -o $@ $<

-include $(SAME70_DEPS)

.PHONY: clean-SAME70
clean-SAME70:
	$(Q)echo "  RM      $(SAME70_BUILD_DIR)"
	$(Q)rm -rf $(SAME70_BUILD_DIR)
