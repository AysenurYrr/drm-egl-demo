CC = gcc
CFLAGS = -Wall -I/usr/include/libdrm -Isrc/common -Isrc/drm -Isrc/app -Isrc/app/* -Isrc/common/shader_utils -I/usr/include/EGL -I/usr/include/GLES2
LDFLAGS = -lEGL -lGLESv2 -lgbm -ldrm -lm

TARGET = main
SRC_DIR = src
BUILD_DIR = build

all: $(TARGET)

$(TARGET): $(BUILD_DIR)/main.o $(BUILD_DIR)/renderer.o $(BUILD_DIR)/drm-common.o $(BUILD_DIR)/common.o $(BUILD_DIR)/triangle.o $(BUILD_DIR)/shader_utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/renderer.o: $(SRC_DIR)/renderer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/drm-common.o: $(SRC_DIR)/drm/drm-common.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/common.o: $(SRC_DIR)/common/common.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/triangle.o: $(SRC_DIR)/app/03_split_screen_y_axis_moving_triangle/triangle.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/shader_utils.o: $(SRC_DIR)/common/shader_utils/shader_utils.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
