#pragma once

#include <stdint.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

typedef struct conn_list {
    drmModeConnector *conn;
    struct conn_list *next;
} conn_list;

struct drm_manager {
    struct drm_dev *dev_list;
    conn_list *conns;
    drmModeConnector *activeConn;
    int dri_fd;
    drmModeRes *res;
};

struct drm_buf {
    uint32_t fb_id;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint32_t stride;
    uint32_t size;
    uint32_t pitch;
    uint32_t handle;
    uint8_t *map;
};

struct drm_dev {
    struct drm_dev *next;
    int fd;
    uint32_t conn_id;
    uint32_t enc_id;
    int32_t crtc_id;

    drmModeModeInfo mode;
    drmModeEncoder *encoder;
    int crtc;
    unsigned int fb_id;
    struct timeval start;

    struct drm_buf buf;
    int swap_count;
    // save the configuration before we take over the DRM device here,
    // so we can restore the configuration at program exit
    drmModeCrtc *saved_crtc;
};

void drm_manager_init(struct drm_manager *drm);
int drm_open(struct drm_manager *drm, const char *path);
int registerConnectors(struct drm_manager *drm);
void drm_manager_mode_set(struct drm_manager *drm);
void drm_manager_cleanup(struct drm_manager *drm);

void add_conn(conn_list *list, drmModeConnector *conn);
