# RRFLibraries STM32H5 Configuration Makefile

STM32H5_BUILD_DIR := STM32H5
STM32H5_TARGET := $(STM32H5_BUILD_DIR)/libRRFLibraries.a

STM32H5_SRC_DIR := src

STM32H5_CPP_SRCS := $(filter-out $(STM32H5_SRC_DIR)/RP2040/%,$(call rwildcard,$(STM32H5_SRC_DIR),*.cpp))

STM32H5_INCLUDES := \
	-I$(STM32H5_SRC_DIR)

STM32H5_DEFINES := \
	-DSTM32H523xx \
	-DNO_PRINTF_FLOAT

STM32H5_CXXFLAGS := -c -std=c++20 \
	-mcpu=cortex-m33 \
	-mthumb \
	-fno-math-errno \
	-mfpu=fpv5-sp-d16 \
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
	-Os \
	$(STM32H5_INCLUDES) \
	$(STM32H5_DEFINES)

STM32H5_CXXFLAGS += $(DEBUG_FLAGS)

STM32H5_OBJS := $(STM32H5_CPP_SRCS:%.cpp=$(STM32H5_BUILD_DIR)/%.o)
STM32H5_DEPS := $(STM32H5_OBJS:.o=.d)

.PHONY: STM32H5
STM32H5: $(STM32H5_TARGET)

$(STM32H5_TARGET): $(STM32H5_OBJS)
	$(Q)echo "  AR      $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(AR) rcs $@ $^

$(STM32H5_BUILD_DIR)/%.o: %.cpp
	$(Q)echo "  CXX     $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) $(STM32H5_CXXFLAGS) -MMD -MP -o $@ $<

-include $(STM32H5_DEPS)

.PHONY: clean-STM32H5
clean-STM32H5:
	$(Q)echo "  RM      $(STM32H5_BUILD_DIR)"
	$(Q)rm -rf $(STM32H5_BUILD_DIR)
