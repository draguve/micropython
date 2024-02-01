CEXAMPLE_MOD_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD += $(CEXAMPLE_MOD_DIR)/pico_num.c
SRC_USERMOD += $(CEXAMPLE_MOD_DIR)/q.c
SRC_USERMOD += $(CEXAMPLE_MOD_DIR)/m_string.c

CFLAGS_USERMOD += -fwrapv

# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(CEXAMPLE_MOD_DIR)
