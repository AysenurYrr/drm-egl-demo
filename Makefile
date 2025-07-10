CC = gcc
CFLAGS = -Wall -I/usr/include/libdrm -Isrc/common -Isrc/drm
LDFLAGS = -lEGL -lGLESv2 -lgbm -ldrm -lm

TARGET = main
SRC_DIR = src
BUILD_DIR = build

all: $(TARGET)

$(TARGET): $(BUILD_DIR)/main.o $(BUILD_DIR)/renderer.o $(BUILD_DIR)/drm-common.o $(BUILD_DIR)/common.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/renderer.o: $(SRC_DIR)/renderer.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/drm-common.o: $(SRC_DIR)/drm/drm-common.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/common.o: $(SRC_DIR)/common/common.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)