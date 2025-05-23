#include "draw.h"
#include "drm_helper.h"
#include "scene.h"
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <float.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct drm_manager drmm;

void sigint_handler(__attribute__((unused)) int signo) {
  drm_manager_cleanup(&drmm);
  exit(0);
}

int main(int argc, const char **argv) {
  signal(SIGINT, sigint_handler);

  struct drm_dev *iter;
  int dri_fd;
  int ret;
  Scene scene;

  /* Setup the DRM Device */
  const char *card = argc > 1 ? argv[1] : "/dev/dri/card0";
  fprintf(stdout, "using card '%s'\n", card);

  drm_manager_init(&drmm);

  dri_fd = drm_open(&drmm, card);
  if (!dri_fd)
    return 0;

  ret = registerConnectors(&drmm);
  if (ret)
    return -1;

  drm_manager_mode_set(&drmm);
  for (iter = drmm.dev_list; iter; iter = iter->next)
    clear(iter);

  /* setup and render the scene */
  scene_init(&scene, 20);
  for (iter = drmm.dev_list; iter; iter = iter->next) {
    render(&scene, iter);
  }

  /* sleep 1 minute or interrupt with CTRL-C */
  sleep(60);
  drm_manager_cleanup(&drmm);
  return 0;
}
