#include <stdio.h>
#include "drm-common.h"
#include "renderer.h"

int main() {
    // 1) DRM/KMS init (connector, crtc, mode)
    struct drm drm = {0};
    if (init_drm(&drm, "/dev/dri/card0")) {
        fprintf(stderr, "init_drm failed\n");
        return -1;
    }

    // Call the rendering function
    if (render_frame(&drm)) {
        fprintf(stderr, "render_frame failed\n");
        return -1;
    }

    // Cleanup
    close(drm.fd);
    return 0;
}