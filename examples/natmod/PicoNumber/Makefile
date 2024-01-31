# Location of top-level MicroPython directory
MPY_DIR = ../../..

# Name of module
MOD = PicoNumber_$(ARCH)

# Source files (.c or .py)
SRC = pico_num.c fix16.c fix16_exp.c fix16_sqrt.c fix16_str.c fix16_trig.c

# Architecture to build for (x86, x64, armv7m, xtensa, xtensawin)
ARCH = x64

# Include to get the rules for compiling and linking the module
include $(MPY_DIR)/py/dynruntime.mk
