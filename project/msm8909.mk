# top level project rules for the msm8909 project
#
LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := msm8909

MODULES += app/aboot

ifeq ($(TARGET_BUILD_VARIANT),user)
DEBUG := 0
else
DEBUG := 1
endif

EMMC_BOOT := 1

#ENABLE_SMD_SUPPORT := 1
ENABLE_PWM_SUPPORT := true
#ENABLE_BOOT_CONFIG_SUPPORT := 1

#DEFINES += WITH_DEBUG_DCC=1
DEFINES += WITH_DEBUG_LOG_BUF=1
DEFINES += WITH_DEBUG_UART=1
#DEFINES += WITH_DEBUG_FBCON=1
DEFINES += DEVICE_TREE=1
#DEFINES += MMC_BOOT_BAM=1
#DEFINES += CRYPTO_BAM=1
DEFINES += SPMI_CORE_V2=1
DEFINES += ABOOT_IGNORE_BOOT_HEADER_ADDRS=1

DEFINES += ABOOT_FORCE_KERNEL_ADDR=0x80008000
DEFINES += ABOOT_FORCE_RAMDISK_ADDR=0x82000000
DEFINES += ABOOT_FORCE_TAGS_ADDR=0x81E00000
DEFINES += ABOOT_FORCE_KERNEL64_ADDR=0x00080000

DEFINES += BAM_V170=1

#Enable the feature of long press power on
DEFINES += LONG_PRESS_POWER_ON=1

#Disable thumb mode
ENABLE_THUMB := false

ENABLE_SDHCI_SUPPORT := 1

ifeq ($(ENABLE_SDHCI_SUPPORT),1)
DEFINES += MMC_SDHCI_SUPPORT=1
endif

#enable power on vibrator feature
ENABLE_PON_VIB_SUPPORT := true

ifeq ($(EMMC_BOOT),1)
DEFINES += _EMMC_BOOT=1
endif

ifeq ($(ENABLE_PON_VIB_SUPPORT),true)
DEFINES += PON_VIB_SUPPORT=1
endif

ifeq ($(ENABLE_SMD_SUPPORT),1)
DEFINES += SMD_SUPPORT=1
endif

ifeq ($(ENABLE_BOOT_CONFIG_SUPPORT),1)
DEFINES += BOOT_CONFIG_SUPPORT=1
endif
