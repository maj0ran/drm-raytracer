#include "draw.h"
#include "drm_helper.h"
#include "object.h"
#include "plane.h"
#include "ray.h"
#include "scene.h"
#include "sphere.h"
#include "sunlight.h"
#include "vector.h"
#include <errno.h>
#include <float.h>
#include <stdio.h>
#include <unistd.h>

Color red = {.r = 1.0, .g = 0, .b = 0};
Color green = {.r = 0, .g = 1.0, .b = 0};
Color blue = {.r = 0, .g = 0, .b = 1.0};

int main(int argc, const char **argv) {
    struct drm_manager drmm;
    struct drm_dev *iter;
    int dri_fd;
    int ret;
    Scene scene;

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
    // clear all screens
  //  for (iter = drmm.dev_list; iter; iter = iter->next)
  //      clear(iter);

    // setting up the scene
    scene_init(&scene, 10);
    Vec s1_pos = {.x = 900, .y = 100, .z = 100};
    Vec s2_pos = {.x = 100, .y = 300, .z = 200};
    Vec s3_pos = {.x = 700, .y = 500, .z = 100};

    Vec floor_pos = {.x = 0, .y = 800, .z = 0};
    Vec floor_normal = {.x = 0, .y = 1, .z = 0};
    Color floor_color = {.r = 0.4, .g = 0.4, .b = 0.4 };

    Object *sphere_red = sphere_create(&s1_pos, 200.0, &red);
    Object *sphere_green = sphere_create(&s2_pos, 200.0, &green);
    Object *sphere_blue = sphere_create(&s3_pos, 200.0, &blue);
    Object *floor = plane_create(&floor_pos, &floor_normal, &floor_color);
    
    Surface surface;
    surface.type = Reflective;
    surface.reflectivity = 0.5;
    sphere_red->surface = surface;
    sphere_green->surface = surface;
    sphere_blue->surface = surface;
    floor->surface = surface;

    scene_add_object(&scene, sphere_red);
    scene_add_object(&scene, sphere_green);
    scene_add_object(&scene, sphere_blue);
    scene_add_object(&scene, floor);

    Vec sun_direction = { .x = 1.0, .y = 1.0, .z = 1.0 };
    Sunlight sunlight = { .direction = sun_direction, .intensity = 1.0 };
    scene.sunlight = sunlight;


    // render scene
    for (iter = drmm.dev_list; iter; iter = iter->next) {
        render(&scene, iter);
    }

    sleep(2);

    drm_manager_cleanup(&drmm);
    return 0;
}
