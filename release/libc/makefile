include $(TOP_DIR)/config.mk

SOURCES   = string.c stdio.c vsprintf.c
BUILD_DIR = build

OBJECTS=$(SOURCES:%.c=$(BUILD_DIR)/%.o)

TARGET=libc.a

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJECTS)
	$(TRACE_AR)
	$(Q) $(AR) rcs $@ $^

$(BUILD_DIR)/%.o: %.c
	$(TRACE_CC)
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	$(TRACE_MKDIR)
	$(Q) mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(TARGET).map