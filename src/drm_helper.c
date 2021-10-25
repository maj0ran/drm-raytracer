#include "drm_helper.h"
#include "drm.h"
#include "drm_mode.h"
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <sys/mman.h>
#include <xf86drmMode.h>

static int drm_setup_dev(struct drm_manager *drm, struct drm_dev *dev,
                         drmModeConnector *conn);
static int drm_find_crtc(struct drm_manager *drm, struct drm_dev *dev,
                         drmModeConnector *conn);
static int drm_create_fb(struct drm_dev *dev, struct drm_buf *buf);

int drm_open(struct drm_manager *drm, const char *path) {
  int fd, flags;
  uint64_t has_dumb;

  fd = open(path, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "cannot open '%s'\n", path);
    return -errno;
  }

  /* set FD_CLOEXEC flag */
  if ((flags = fcntl(fd, F_GETFD)) < 0 ||
      fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
    fprintf(stderr, "fcntl FD_CLOEXEC failed");
    return -errno;
  }

  /* check if the DRM device is capable dumb buffers */
  if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb) {
    fprintf(stderr, "drm device '%s' does not support dumb buffers\n", path);
    return -ENOTSUP;
  }
  drm->dri_fd = fd;
  return fd;
}

void add_conn(conn_list *list, drmModeConnector *conn) {
  conn_list *current = list;
  while (current->next != NULL) {
    current = current->next;
  }
  current->next->conn = conn;
  current->next->next = NULL;
}

void drm_manager_init(struct drm_manager *drm) {
  drm->dev_list = NULL;
  drm->res = NULL;
}

int registerConnectors(struct drm_manager *drm) {
  drmModeConnector *conn;
  struct drm_dev *dev;
  int ret;

  /* retrieve resources */
  drm->res = drmModeGetResources(drm->dri_fd);
  drmModeRes *res = drm->res;
  if (!res) {
    fprintf(stderr, "cannot retrieve DRM resources (%d): %m\n", errno);
    return -errno;
  }

  /* iterate all connectors */
  for (int i = 0; i < res->count_connectors; i++) {
    conn = drmModeGetConnector(drm->dri_fd, res->connectors[i]);
    if (!conn) {
      fprintf(stderr, "cannot retrieve DRM connector %u:%u (%d): %m\n", i,
              res->connectors[i], errno);
      continue;
    }
    /* for each valid connector, create a new DRM device structure */
    dev = malloc(sizeof(*dev));
    memset(dev, 0, sizeof(*dev));
    dev->fd = drm->dri_fd;
    dev->conn_id = conn->connector_id;
    ret = drm_setup_dev(drm, dev, conn);
    // error
    if (ret) {
      if (ret != -ENOENT) {
        errno = -ret;
        fprintf(stderr, "cannot setup device for connector %u, %u (%d): %m", i,
                res->connectors[i], errno);
      }
      free(dev);
      drmModeFreeConnector(conn);
      continue;
    }
    // success
    // free the connector data we retrieved from the kernel as we copied it now
    // to our userspace. Link the drm device to our list
    drmModeFreeConnector(conn);
    dev->next = drm->dev_list;
    drm->dev_list = dev;
  }

  return 0;
}

static int drm_setup_dev(struct drm_manager *drm, struct drm_dev *dev,
                         drmModeConnector *conn) {
  int ret;
  /* check if a monitor is connected */
  if (conn->connection != DRM_MODE_CONNECTED) {
    fprintf(stderr, "ignoring connector %u: not connected to a monitor\n",
            conn->connector_id);
    return -ENOENT;
  }
  // check if there at least one valid mode (that is resolution, etc.)
  if (conn->count_modes == 0) {
    fprintf(stderr, "no valid modes for connector %u\n", conn->connector_id);
    return -EFAULT;
  }

  // now we can do the actual device setup. We copy the mode-settings from the
  // connector to our personal structure and link it with a valid CRTC. We also
  // setup a framebuffer for the image data

  // TODO: Only use the first mode for now, which should be the
  // native/recommended mode. But in reality, we could fetch all modes and make
  // some way to select one.
  memcpy(&dev->mode, &conn->modes[0], sizeof(dev->mode));
  dev->buf.width = conn->modes[0].hdisplay;
  dev->buf.height = conn->modes[0].vdisplay;
  dev->buf.bpp = 32;
  fprintf(stdout, "mode for connector %u is %ux%u\n", conn->connector_id,
          dev->buf.width, dev->buf.height);
  // find a CRTC for this connector
  ret = drm_find_crtc(drm, dev, conn);
  if (ret) {
    fprintf(stderr, "no valid CRTC found for connector %u\n",
            conn->connector_id);
    return ret;
  }

  // create a framebuffer for this CRTC
  ret = drm_create_fb(dev, &dev->buf);
  if (ret) {
    fprintf(stderr, "cannot create framebuffer for connector %u\n",
            conn->connector_id);
    return ret;
  }
  return 0;
}

/* find a CRTC for a given connector */
static int drm_find_crtc(struct drm_manager *drm, struct drm_dev *dev,
                         drmModeConnector *conn) {
  drmModeEncoder *enc = NULL;
  int32_t crtc;
  struct drm_dev *iter;

  // first try the currently connected encoder+crtc
  if (conn->encoder_id)
    enc = drmModeGetEncoder(dev->fd, conn->encoder_id);

  if (enc) {
    if (enc->crtc_id) {
      crtc = enc->crtc_id;
      for (iter = drm->dev_list; iter; iter = iter->next) {
        if (iter->crtc_id == crtc) {
          crtc = -1;
          break;
        }
      }
      if (crtc >= 0) {
        drmModeFreeEncoder(enc);
        dev->crtc_id = crtc;
        return 0;
      }
    }

    drmModeFreeEncoder(enc);
  }

  /* If the connector is not currently bound to an encoder or if the
   * encoder+crtc is already used by another connector, iterate all other
   * available encoders to find a matching CRTC. */

  for (int i = 0; i < conn->count_encoders; i++) {
    enc = drmModeGetEncoder(dev->fd, conn->encoders[i]);
    if (!enc) {
      fprintf(stderr, "cannot retrieve encoder %u:%u (%d): %m\n", i,
              conn->encoders[i], errno);
      continue;
    }

    drmModeRes *res = drm->res;
    // iterate all CRTCs
    for (int j = 0; j < res->count_crtcs; j++) {
      // check whether the CRTC works with the encoder
      if (!(enc->possible_crtcs & (1 << j)))
        continue;
      // check that no other device already uses this CRTC
      crtc = res->crtcs[j];
      for (iter = drm->dev_list; iter; iter = iter->next) {
        if (iter->crtc_id == crtc) {
          crtc = -1;
          break;
        }
      }

      // now we have foud a valid CRTC, save it and return
      if (crtc >= 0) {
        drmModeFreeEncoder(enc);
        dev->crtc_id = crtc;
        return 0;
      }
    }
    drmModeFreeEncoder(enc);
  }
  fprintf(stderr, "cannot find a suitable CRTC for connector %u\n",
          conn->connector_id);
  return -ENOENT;
}

static int drm_create_fb(struct drm_dev *dev, struct drm_buf *buf) {
  struct drm_mode_create_dumb creq;
  struct drm_mode_destroy_dumb dreq;
  struct drm_mode_map_dumb mreq;
  int ret;

  // create dumf buffer
  memset(&creq, 0, sizeof(creq));
  creq.width = buf->width;
  creq.height = buf->height;
  creq.bpp = buf->bpp;
  ret = drmIoctl(dev->fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
  if (ret < 0) {
    fprintf(stderr, "cannot create dumb buffer (%d): %m\n", errno);
    return -errno;
  }
  buf->stride = creq.pitch;
  buf->size = creq.size;
  buf->handle = creq.handle;

  // create framebuffer object for the dumb buffer
  ret = drmModeAddFB(dev->fd, buf->width, buf->height, 24, buf->bpp,
                     buf->stride, buf->handle, &buf->fb_id);

  if (ret) {
    fprintf(stderr, "cannot create framebuffer (%d): %m\n", errno);
    ret = -errno;
    goto err_destroy;
  }

  // prepare buffer for memory mapping
  memset(&mreq, 0, sizeof(mreq));
  mreq.handle = buf->handle;
  ret = drmIoctl(dev->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
  if (ret) {
    fprintf(stderr, "cannot map dumb buffer (%d): %m\n", errno);
    ret = -errno;
    goto err_fb;
  }

  // perform actual memory mapping
  buf->map = mmap(0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd,
                  mreq.offset);
  if (buf->map == MAP_FAILED) {
    fprintf(stderr, "cannot mmap dumb buffer (%d): %m\n", errno);
    ret = -errno;
    goto err_fb;
  }

  // clear framebuffer
  memset(buf->map, 0, buf->size);

  return 0;

err_fb:
  drmModeRmFB(dev->fd, buf->fb_id);
err_destroy:
  memset(&dreq, 0, sizeof(dreq));
  dreq.handle = buf->handle;
  drmIoctl(dev->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
  return ret;
}

void drm_manager_mode_set(struct drm_manager *drm) {
  struct drm_dev *iter;
  struct drm_buf *buf;

  for (iter = drm->dev_list; iter; iter = iter->next) {
    // save the current DRM setting so we can restore at program exit
    printf("%s: %d\n", __func__, __LINE__);
    iter->saved_crtc = drmModeGetCrtc(drm->dri_fd, iter->crtc_id);

    printf("%s: %d\n", __func__, __LINE__);
    buf = &iter->buf;
    if (drmModeSetCrtc(drm->dri_fd, iter->crtc_id, buf->fb_id, 0, 0,
                       &iter->conn_id, 1, &iter->mode)) {
      fprintf(stderr, "cannot set CRTC for connector %u (%d): %m\n",
              iter->conn_id, errno);
    }
  }
}

void drm_manager_cleanup(struct drm_manager *drm) {
  struct drm_dev *iter;
  struct drm_mode_destroy_dumb dreq;

  while (drm->dev_list) {
    iter = drm->dev_list;
    drm->dev_list = iter->next;

    // restore saved CRTC configuration
    drmModeSetCrtc(drm->dri_fd, iter->saved_crtc->crtc_id,
                  iter->saved_crtc->buffer_id, iter->saved_crtc->x,
                  iter->saved_crtc->y, &iter->conn_id, 1,
                  &iter->saved_crtc->mode);
    drmModeFreeCrtc(iter->saved_crtc);

    // unmap buffer
    munmap(iter->buf.map, iter->buf.size);

    // delete framebuffer
    drmModeRmFB(drm->dri_fd, iter->buf.fb_id);

    // delete dumf buffer
    memset(&dreq, 0, sizeof(dreq));
    dreq.handle = iter->buf.handle;
    drmIoctl(drm->dri_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);

    free(iter);
  }
  drmModeFreeResources(drm->res);
}
