# PS-to-VNC clean reconstruction — Issue #39 Stage 2 linked build.
#
# Extend the qualified Issue #38 controller/input chassis with the keyboard,
# local foreground, historical OSK, clean visual renderer, and local
# presentation feature family without overwriting earlier qualified artifacts.

BUILD_DIR ?= build/reconstruction/issue39
EE_BIN ?= $(BUILD_DIR)/PS-to-VNC-Issue39.ELF

EXTRA_EE_OBJS := \
	$(BUILD_DIR)/input.o \
	$(BUILD_DIR)/mouse.o \
	$(BUILD_DIR)/input_runtime.o \
	$(BUILD_DIR)/keyboard.o \
	$(BUILD_DIR)/local_ui.o \
	$(BUILD_DIR)/osk.o \
	$(BUILD_DIR)/osk_render.o \
	$(BUILD_DIR)/local_ui_presentation.o \
	$(BUILD_DIR)/local_controller.o

include mk/issue7-clean.mk

$(BUILD_DIR)/local_controller.o: \
	src/ui/local_controller.c \
	src/ui/local_controller.h \
	src/input/controller.h \
	src/ui/local_ui.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
