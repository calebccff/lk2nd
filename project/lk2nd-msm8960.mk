LOCAL_DIR := $(GET_LOCAL_DIR)

include $(LOCAL_DIR)/msm8960.mk
include $(LOCAL_DIR)/lk2nd-common.mk

LK2ND_KEYMAP := 1

APPSBOOTHEADER: $(OUTBOOTIMG) $(OUTBOOTIMGADTB) $(OUTODINTAR)
ANDROID_BOOT_BASE := 0x80000000

DISPLAY_USE_CONTINUOUS_SPLASH := 0

# Memory usually reserved for RMTFS, should be fine for early SMP bring-up
#SMP_SPIN_TABLE_BASE := 0x86700000
