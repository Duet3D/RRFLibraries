# RRFLibraries SAM4E (non-RTOS) Configuration Makefile

SAM4E_BUILD_DIR := SAM4E
SAM4E_TARGET := $(SAM4E_BUILD_DIR)/libRRFLibraries.a

SAM4E_SRC_DIR := src

SAM4E_CPP_SRCS := $(shell find $(SAM4E_SRC_DIR) -name '*.cpp' ! -path '*/RP2040/*')

SAM4E_INCLUDES := \
	-I$(SAM4E_SRC_DIR)

SAM4E_DEFINES := \
	-D__SAM4E8E__

SAM4E_CXXFLAGS := -c -std=gnu++17 \
	-mcpu=cortex-m4 \
	-mthumb \
	-mfpu=fpv4-sp-d16 \
	-mfloat-abi=hard \
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
	$(SAM4E_INCLUDES) \
	$(SAM4E_DEFINES)

# Add debug flags if DEBUG=1
ifeq ($(DEBUG),1)
	SAM4E_CXXFLAGS += -O0 -g3
else
	SAM4E_CXXFLAGS += -Os
endif

SAM4E_OBJS := $(SAM4E_CPP_SRCS:%.cpp=$(SAM4E_BUILD_DIR)/%.o)
SAM4E_DEPS := $(SAM4E_OBJS:.o=.d)

.PHONY: SAM4E
SAM4E: $(SAM4E_TARGET)

$(SAM4E_TARGET): $(SAM4E_OBJS)
	$(Q)echo "  AR      $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(AR) rcs $@ $^
	$(Q)echo "Built $@"

$(SAM4E_BUILD_DIR)/%.o: %.cpp
	$(Q)echo "  CXX     $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) $(SAM4E_CXXFLAGS) -MMD -MP -o $@ $<

-include $(SAM4E_DEPS)

.PHONY: clean-SAM4E
clean-SAM4E:
	$(Q)echo "  RM      $(SAM4E_BUILD_DIR)"
	$(Q)rm -rf $(SAM4E_BUILD_DIR)
