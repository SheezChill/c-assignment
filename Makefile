CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
BUILD_DIR=./build
BIN_DIR=./bin
SRC_DIR=src
LIB_DIR=./lib
OBJECTS=$(BUILD_DIR)/cJSON.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/fort.o $(BUILD_DIR)/asprintf.o $(BUILD_DIR)/login.o $(BUILD_DIR)/administrator.o $(BUILD_DIR)/staff.o $(BUILD_DIR)/customer.o $(BUILD_DIR)/guest.o
EXECUTABLES=$(BIN_DIR)/staff $(BIN_DIR)/admin $(BIN_DIR)/customer $(BIN_DIR)/guest $(BIN_DIR)/login

.PHONY: all staff admin customer guest login post_build clean

all: $(EXECUTABLES)

staff: $(BIN_DIR)/staff

admin: $(BIN_DIR)/admin

customer: $(BIN_DIR)/customer

guest: $(BIN_DIR)/guest

login: $(BIN_DIR)/login

# Executable targets
$(BIN_DIR)/staff: $(BUILD_DIR)/staff.o $(OBJECTS) | $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/admin: $(BUILD_DIR)/administrator.o $(OBJECTS) | $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/customer: $(BUILD_DIR)/customer.o $(OBJECTS) | $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/guest: $(BUILD_DIR)/guest.o $(OBJECTS) | $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/login: $(BUILD_DIR)/login.o $(OBJECTS) | $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

# Object file targets
$(BUILD_DIR)/fort.o: $(LIB_DIR)/fort.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/cJSON.o: $(LIB_DIR)/cJSON.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/asprintf.o: $(LIB_DIR)/asprintf.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/utils.o: $(SRC_DIR)/utils.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/administrator.o: $(SRC_DIR)/menus/administrator.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/staff.o: $(SRC_DIR)/menus/staff.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/customer.o: $(SRC_DIR)/menus/customer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/guest.o: $(SRC_DIR)/menus/guest.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/login.o: $(SRC_DIR)/login.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Post build target to clean object files
post_build:
	rm -rf $(BUILD_DIR)/*.o

# Clean target to remove all build and bin directories
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Ensure directories exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

