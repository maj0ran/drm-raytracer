#include "color.h"
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
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <signal.h>

struct drm_manager drmm;

void intHandler(__attribute__((unused)) int signo) {
    drm_manager_cleanup(&drmm);
    exit(0);
}
Color red = {.r = 1.0, .g = 0.4, .b = 0.1};
Color green = {.r = 0.2, .g = 1.0, .b = 0.2};
Color blue = {.r = 0.3, .g = 0.7, .b = 1.0};

int main(int argc, const char **argv) {
    signal(SIGINT, intHandler);

    time_t seed = time(NULL);
    srand(seed);
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
    for (iter = drmm.dev_list; iter; iter = iter->next)
        clear(iter);

    // setting up the scene
    scene_init(&scene, 10);
    Vec s1_pos = {.x = 1400, .y = 600, .z = 500};
    Vec s2_pos = {.x = 200, .y = 800, .z = 500};
    Vec s3_pos = {.x = 800, .y = 900, .z = 600};

    Vec floor_pos = {.x = 0, .y = 1200, .z = 0};
    Vec floor_normal = {.x = 0, .y = 1, .z = 0.1};
    Vec ceiling_pos = {.x = 0, .y = 1000, .z = 0};
    Vec ceiling_normal = {.x = 0, .y = 1, .z = 1};
    Color floor_color = {.r = 0.4, .g = 0.4, .b = 0.4};
    Color sphere_color = generate_random_color();
    Element *sphere_red = sphere_create(&s1_pos, 200.0, &sphere_color);
    sphere_color = generate_random_color();
    Element *sphere_green = sphere_create(&s2_pos, 200.0, &sphere_color);
    sphere_color = generate_random_color();
    Element *sphere_blue = sphere_create(&s3_pos, 200.0, &sphere_color);
    Element *floor = plane_create(&floor_pos, &floor_normal, &floor_color);
    
    Texture tex = gen_checkboard_texture(256);
    set_texture(floor, tex);
    Surface surface_refractive;
    surface_refractive.type = Refractive;
    surface_refractive.index = 1.8;
    surface_refractive.transparency = 1.0;

    Surface surface_reflective;
    surface_reflective.type = Reflective;
    surface_reflective.reflectivity = 0.9;

    sphere_red->surface = surface_reflective;
    sphere_green->surface = surface_reflective;
    sphere_blue->surface = surface_refractive;
    floor->surface = surface_reflective;

    scene_add_object(&scene, sphere_red);
    scene_add_object(&scene, sphere_green);
    scene_add_object(&scene, sphere_blue);
    scene_add_object(&scene, floor);

    Vec sun_direction = {.x = 0, .y = 1.0, .z = 1.0};
    Sunlight sunlight = {.direction = sun_direction, .intensity = 1.0};
    scene.sunlight = sunlight;

    Point light1_pos = {.x = 700, .y = 750, .z = 100};
    Spotlight light1 = {.position = light1_pos, .intensity = 5000000};
    light1.color.r = 1.0;
    light1.color.g = 1.0;
    light1.color.b = 1.0;

    Point light2_pos = {.x = 700, .y = 500, .z = -250};
    Spotlight light2 = {.position = light2_pos, .intensity = 5000000};
    light2.color.r = 1.0;
    light2.color.g = 1.0;
    light2.color.b = 1.0;

    scene_add_spotlight(&scene, &light2);
    scene_add_spotlight(&scene, &light1);
    // render scene
    for (iter = drmm.dev_list; iter; iter = iter->next) {
        render(&scene, iter);
    }

    sleep(5);
    drm_manager_cleanup(&drmm);
    return 0;
}
