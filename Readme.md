# EGL + OpenGL ES + GBM + DRM

Using OpenGL ES and EGL over GBM for direct rendering on Linux consoles

## Installation Instructions

Run the following commands to install required dependencies:
```
sudo apt update
sudo apt install libdrm-dev libgbm-dev libegl1-mesa-dev libgles2-mesa-dev
```

## Build Instructions

To choose which example you want to run in the Makefile, update the path in the following line to match your chosen example:
```
$(BUILD_DIR)/triangle.o: $(SRC_DIR)/app/<change_me>/triangle.c | $(BUILD_DIR)
    $(CC) $(CFLAGS) -c $< -o $@
```
Replace **<change_me>** with the name of your chosen example directory.


To build the project, run the following command:
```
make all
```

## Run Instructions

Switch to a TTY screen (e.g., CTRL+F3) and execute the program:
```
./main
```
