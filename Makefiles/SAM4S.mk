# RRFLibraries SAM4S (non-RTOS) Configuration Makefile

SAM4S_BUILD_DIR := SAM4S
SAM4S_TARGET := $(SAM4S_BUILD_DIR)/libRRFLibraries.a

SAM4S_SRC_DIR := src

SAM4S_CPP_SRCS := $(shell find $(SAM4S_SRC_DIR) -name '*.cpp' ! -path '*/RP2040/*')

SAM4S_INCLUDES := \
	-I$(SAM4S_SRC_DIR)

SAM4S_DEFINES := \
	-D__SAM4S8C__

SAM4S_CXXFLAGS := -c -std=gnu++17 \
	-mcpu=cortex-m4 \
	-mthumb \
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
	$(SAM4S_INCLUDES) \
	$(SAM4S_DEFINES)

# Add debug flags if DEBUG=1
ifeq ($(DEBUG),1)
	SAM4S_CXXFLAGS += -O0 -g3
else
	SAM4S_CXXFLAGS += -O2
endif

SAM4S_OBJS := $(SAM4S_CPP_SRCS:%.cpp=$(SAM4S_BUILD_DIR)/%.o)
SAM4S_DEPS := $(SAM4S_OBJS:.o=.d)

.PHONY: SAM4S
SAM4S: $(SAM4S_TARGET)

$(SAM4S_TARGET): $(SAM4S_OBJS)
	$(Q)echo "  AR      $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(AR) rcs $@ $^
	$(Q)echo "Built $@"

$(SAM4S_BUILD_DIR)/%.o: %.cpp
	$(Q)echo "  CXX     $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) $(SAM4S_CXXFLAGS) -MMD -MP -o $@ $<

-include $(SAM4S_DEPS)

.PHONY: clean-SAM4S
clean-SAM4S:
	$(Q)echo "  RM      $(SAM4S_BUILD_DIR)"
	$(Q)rm -rf $(SAM4S_BUILD_DIR)
