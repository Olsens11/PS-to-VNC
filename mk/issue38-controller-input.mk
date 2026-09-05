# PS-to-VNC clean reconstruction — Issue #38 Stage 1 linked build.
#
# Extend the qualified Issue #7 chassis with the controller/input feature
# family without overwriting the preserved Issue #7 ELF artifact.

BUILD_DIR ?= build/reconstruction/issue38
EE_BIN ?= $(BUILD_DIR)/PS-to-VNC-Issue38.ELF

EXTRA_EE_OBJS := \
	$(BUILD_DIR)/input.o \
	$(BUILD_DIR)/mouse.o \
	$(BUILD_DIR)/input_runtime.o

include mk/issue7-clean.mk
