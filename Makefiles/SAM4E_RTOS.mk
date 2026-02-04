# RRFLibraries SAM4E_RTOS Configuration Makefile

BUILD_DIR := SAM4E_RTOS
TARGET := $(BUILD_DIR)/libRRFLibraries.a

# Source files (adjust based on actual source structure)
SRC_DIR := src

# Find all C++ source files in src directory
SAM4E_RTOS_CPP_SRCS := $(shell find $(SRC_DIR) -name '*.cpp' ! -path '*/SAME5x_C21/*' ! -path '*/RP2040/*')

SAM4E_RTOS_INCLUDES := \
	-I$(SRC_DIR) \
	-I../FreeRTOS/src/include \
	-I../FreeRTOS/src/portable/GCC/ARM_CM4F

SAM4E_RTOS_DEFINES := \
	-D__SAM4E8E__ \
	-DRTOS

SAM4E_RTOS_CXXFLAGS := -c -std=gnu++17 \
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
	-Wfloat-conversion \
	-Werror=return-type \
	-Wsuggest-override \
	-fsingle-precision-constant \
	-fstack-usage \
	-O2 \
	-Wall \
	-Werror \
	-Wnoexcept \
	-Wshadow \
	-Wsign-promo \
	$(SAM4E_RTOS_INCLUDES) \
	$(SAM4E_RTOS_DEFINES)

# Add debug flags if DEBUG=1
ifeq ($(DEBUG),1)
SAM4E_RTOS_CXXFLAGS += -O0 -g3
else
SAM4E_RTOS_CXXFLAGS += -O2
endif

SAM4E_RTOS_OBJS := $(SAM4E_RTOS_CPP_SRCS:%.cpp=$(BUILD_DIR)/%.o)
SAM4E_RTOS_DEPS := $(SAM4E_RTOS_OBJS:.o=.d)

.PHONY: SAM4E_RTOS
SAM4E_RTOS: $(TARGET)

$(TARGET): $(SAM4E_RTOS_OBJS)
	$(Q)echo "  AR      $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: %.cpp
	$(Q)echo "  CXX     $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) $(SAM4E_RTOS_CXXFLAGS) -MMD -MP -o $@ $<

-include $(SAM4E_RTOS_DEPS)

.PHONY: clean-SAM4E_RTOS
clean-SAM4E_RTOS:
	$(Q)echo "  RM      $(BUILD_DIR)"
	$(Q)rm -rf $(BUILD_DIR)
