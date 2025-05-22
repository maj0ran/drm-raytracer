#include "scene.h"
#include "color.h"
#include "draw.h"
#include "vector.h"
#include <math.h>

#define EPS 0.01

void scene_init(Scene *scene, size_t capacity) {
    scene->objects = malloc(sizeof(Element *) * capacity);
    scene->capacity = capacity;
    scene->size = 0;

    scene->lights = NULL;

    Element *sphere_red =
        sphere_create(&(Vec){.x = 700, .y = 400, .z = -100}, 100.0, &RED);
    Element *sphere_green =
        sphere_create(&(Vec){.x = 900, .y = 500, .z = -200}, 100.0, &GREEN);
    Element *sphere_blue =
        sphere_create(&(Vec){.x = 600, .y = 500, .z = -400}, 100.0, &BLUE);
    Element *sphere_yellow =
        sphere_create(&(Vec){.x = 1200, .y = 500, .z = -350}, 100.0, &YELLOW);
    Element *sphere_magenta =
        sphere_create(&(Vec){.x = 1600, .y = 500, .z = 100}, 100.0, &MAGENTA);

    Element *floor = plane_create(&(Vec){.x = 0, .y = 600, .z = 1000},
                                  &(Vec){.x = 0, .y = 1.0, .z = 0.0}, &GREY);

    Element *ceiling = plane_create(&(Vec){.x = 0, .y = 0, .z = 1000},
                                  &(Vec){.x = 0, .y = -1.0, .z = .2}, &GREY);
    Texture tex = gen_checkboard_texture(64);
    set_texture(floor, tex);
    set_texture(ceiling, tex);

    Surface surface_floor = (Surface){.type = Reflective, .reflectivity = 0.8};
    Surface surface_ceiling = (Surface){.type = Diffuse, .reflectivity = 0.3};
    Surface surface_refractive =
        (Surface){.type = Refractive, .index = 1.5, .transparency = 1.0};
    Surface surface_reflective =
        (Surface){.type = Reflective, .reflectivity = 0.4};

    sphere_red->surface = surface_reflective;
    sphere_green->surface = surface_refractive;
    sphere_blue->surface = surface_refractive;
    sphere_yellow->surface = surface_reflective;
    sphere_magenta->surface = surface_refractive;
    floor->surface = surface_floor;
    ceiling->surface = surface_ceiling;

    scene_add_object(scene, sphere_red);
    scene_add_object(scene, sphere_green);
    scene_add_object(scene, sphere_blue);
    scene_add_object(scene, sphere_yellow);
    scene_add_object(scene, sphere_magenta);
    scene_add_object(scene, floor);
    scene_add_object(scene, ceiling);

    Light *sunlight = directional_light_create(
        (Vec){.x = 0, .y = 0, .z = 1.0}, (Color){.r = 1, .g = 1, .b = 0.9}, 1.0);

    Light *point_1 =
        point_light_create((Vec){.x = 1260, .y = 500, .z = -600},
                           (Color){.r = 1, .g = 0.6, .b = 0.2}, 500000);
    Light *point_2 =
        point_light_create((Vec){.x = 700, .y = 500, .z = -700},
                           (Color){.r = 1, .g = 0.7, .b = 1.0}, 500000);
    Light *point_3 =
        point_light_create((Vec){.x = 700, .y = 300, .z = 700},
                           (Color){.r = 1, .g = 1.0, .b = .7}, 15000000);

    Light *point_4 =
        point_light_create((Vec){.x = 2900, .y = 500, .z = 1500},
                           (Color){.r = 1, .g = 1, .b = .8}, 12000000);
    scene_add_light(scene, sunlight);
    scene_add_light(scene, point_1);
    scene_add_light(scene, point_2);
    scene_add_light(scene, point_3);
    scene_add_light(scene, point_4);
}

int scene_add_object(Scene *scene, Element *obj) {
    if (scene->size == scene->capacity)
        return -1;

    scene->objects[scene->size] = obj;
    scene->size++;

    return 0;
}

int scene_add_light(Scene *scene, Light *light) {
    if (scene->lights == NULL) {
        scene->lights = malloc(sizeof(struct Light_list));
        scene->lights->light = light;
        scene->lights->next = NULL;
        return 0;
    }

    Light_list *iter = scene->lights;
    while (iter->next != NULL)
        iter = iter->next;

    iter->next = malloc(sizeof(struct Light_list));
    iter->next->light = light;
    iter->next->next = NULL;
    return 0;
}

static Ray create_reflection(Ray *ray, Point *hit_point, Vec *surface_normal) {

    Vec offset = v_mul_s(surface_normal, EPS);
    Point reflect_origin = v_add(hit_point, &offset);

    Vec reflect_direction =
        v_mul_s(surface_normal, v_dot(&ray->direction, surface_normal) * 2.0);
    reflect_direction = v_sub(&ray->direction, &reflect_direction);
    reflect_direction = v_normalize(&reflect_direction);
    Ray reflect_ray = {.origin = reflect_origin,
                       .direction = reflect_direction};
    return reflect_ray;
}

static bool create_transmission(Ray *ray, Point *hit_point, Vec *surface_normal,
                                float index, Ray *transmission_ray) {
    Vec n = *surface_normal;
    float eta_t = index;
    float eta_i = 1.0;
    float i_dot_n = v_dot(&ray->direction, surface_normal);
    if (i_dot_n < 0.0) {
        // outside the surface
        i_dot_n = -i_dot_n;
    } else {
        n = v_neg(surface_normal);
        eta_t = 1.0;
        eta_i = index;
    }

    float eta = eta_i / eta_t;
    float k = 1.0 - (eta * eta) * (1.0 - i_dot_n * i_dot_n);
    if (k < 0.0) {
        return false;
    } else {
        Vec offset = v_mul_s(&n, -EPS);
        transmission_ray->origin = v_add(hit_point, &offset);

        Vec direction = v_mul_s(&n, i_dot_n);
        direction = v_add(&ray->direction, &direction);
        direction = v_mul_s(&direction, eta);
        n = v_mul_s(&n, sqrtf(k));
        direction = v_sub(&direction, &n);
        direction = v_normalize(&direction);
        transmission_ray->direction = direction;
        return true;
    }
}

Color get_color(Scene *scene, Ray *ray, Intersection *i, uint8_t depth) {
    Point hit_point = v_mul_s(&ray->direction, i->distance);
    hit_point = v_add(&ray->origin, &hit_point);
    SurfaceType surface_type = i->object->surface.type;
    Vec normal = surface_normal(i->object, &hit_point);
    Color c;

    switch (surface_type) {
    case Diffuse:
        c = shadow_diffuse(scene, ray, i);
        break;
    case Reflective:
        c = shadow_diffuse(scene, ray, i);
        float reflectivity = i->object->surface.reflectivity;
        Ray reflection_ray = create_reflection(ray, &hit_point, &normal);

        c = c_mul_s(&c, reflectivity);
        Color i_reflection = cast_ray(scene, &reflection_ray, depth + 1);
        i_reflection = c_mul_s(&i_reflection, reflectivity);
        c = c_add(&c, &i_reflection);
        break;
    case Refractive: {
        Color refraction_color = BACKGROUND;
        float index = i->object->surface.index;
        Color surface_color = i->object->color;

        float kr = fresnel(ray, &normal, index);
        if (kr < 1.0) {
            Ray transmission_ray;
            create_transmission(ray, &hit_point, &normal, index,
                                &transmission_ray);
            refraction_color = cast_ray(scene, &transmission_ray, depth + 1);
        }
        Ray reflection_ray = create_reflection(ray, &hit_point, &normal);
        Color reflection_color = cast_ray(scene, &reflection_ray, depth + 1);
        reflection_color = c_mul_s(&reflection_color, kr);
        refraction_color = c_mul_s(&refraction_color, 1.0 - kr);
        c = c_add(&reflection_color, &refraction_color);
        c = c_mul(&c, &surface_color);
        c = c_mul_s(&c, i->object->surface.transparency);
    } break;
    }

    clamp(&c);

    return c;
}

Color cast_ray(Scene *scene, Ray *ray, uint8_t depth) {
    if (depth >= 4) {
        Color c = BACKGROUND;
        return c;
    }

    Intersection i = trace_ray(scene, ray);
    if (i.object != NULL) {
        return get_color(scene, ray, &i, depth);
    }

    Color c = BACKGROUND;
    return c;
}

void wrap(const Texture *tex, TextureCoords *coords) {
    coords->x %= tex->width;
    coords->y %= tex->height;
}

Color shadow_diffuse(Scene *scene, Ray *ray, Intersection *i) {
    Color c = BLACK;

    Point hit_point = v_mul_s(&ray->direction, i->distance);
    hit_point = v_add(&ray->origin, &hit_point);
    Vec normal = surface_normal(i->object, &hit_point);

    Color *surface_color;
    if (i->object->is_textured) {
        Texture *tex = &i->object->texture;
        TextureCoords coords = texture_coords(i->object, &hit_point);
        wrap(tex, &coords);
        surface_color = tex->data + (tex->width * coords.y + coords.x);
    } else {
        surface_color = &i->object->color;
    }

    // Spotlights
    Light_list *iter = scene->lights;
    Ray shadow_ray;
    Intersection shadow_intersect;
    while (iter) {
        Light *light = iter->light;
        Vec dir_to_light = direction(light, &hit_point);
        dir_to_light = v_neg(&dir_to_light);
        dir_to_light = v_normalize(&dir_to_light);
        shadow_ray.origin = v_mul_s(&normal, EPS);
        shadow_ray.origin = v_add(&hit_point, &shadow_ray.origin);
        shadow_ray.direction = dir_to_light;

        shadow_intersect = trace_ray(scene, &shadow_ray);

        bool in_light = (shadow_intersect.object == NULL);
        float light_intensity;
        float light_power;
        Color light_color;
        if (!in_light) {
            float light_distance = distance(light, &hit_point);
            in_light = (shadow_intersect.distance > light_distance);
        }
        if (in_light) {
            light_intensity = intensity(light, &hit_point);
        } else {
            light_intensity = 0.0;
        }
        float light_reflected = i->object->surface.reflectivity;
        light_power =
            fmax(v_dot(&normal, &dir_to_light), 0.0) * light_intensity;
        light_color = c_mul_s(&light->color, light_power);
        light_color = c_mul_s(&light_color, light_reflected);
        light_color = c_mul(surface_color, &light_color);
        c = c_add(&c, &light_color);
        iter = iter->next;
    }
    return c;
}

float fresnel(Ray *ray, Vec *surface_normal, float index) {
    Vec incident = v_normalize(&ray->direction);
    Vec n = v_normalize(surface_normal);
    float i_dot_n = v_dot(&incident, &n);
    float eta_i = 1.0;
    float eta_t = index;
    if (i_dot_n > 0.0) {
        eta_i = eta_t;
        eta_t = 1.0;
    }

    float sin_t = eta_i / eta_t * sqrtf(fmaxf((1.0 - i_dot_n * i_dot_n), 0.0));
    if (sin_t > 1.0) {
        // Total internal reflection
        return 1.0;
    } else {
        float cos_t = sqrtf(fmaxf(1.0 - sin_t * sin_t, 0.0));
        float cos_i = fabsf(cos_t);
        float r_s = ((eta_t * cos_i) - (eta_i * cos_t)) /
                    ((eta_t * cos_i) + (eta_i * cos_t));
        float r_p = ((eta_i * cos_i) - (eta_t * cos_t)) /
                    ((eta_i * cos_i) + (eta_t * cos_t));
        return (r_s * r_s + r_p * r_p) / 2.0;
    }
}

Intersection trace_ray(Scene *scene, Ray *ray) {
    float distance;
    float nearest_intersection = INFINITY;
    Element *nearest_object = NULL;
    Intersection i;
    for (size_t obj = 0; obj < scene->size; obj++) {
        Element *o = scene->objects[obj];
        if (intersect(o, ray, &distance)) {
            if (distance < nearest_intersection) {
                nearest_intersection = distance;
                nearest_object = o;
            }
        }
    }

    i.distance = nearest_intersection;
    i.object = nearest_object;
    return i;
}

void render(Scene *scene, struct drm_dev *dev) {
    // starting point of the rays
    // this is where the eye/camera is
    Vec origin = {.x = 960, .y = 300, .z = -1000};
    // One Ray through each pixel
    for (int i = 0; i < 1920; i++) {
        for (int j = 0; j < 1080; j++) {
            Vec target = {.x = i, .y = j, .z = 0};
            Vec direction = v_sub(&target, &origin);
            direction = v_normalize(&direction);
            Ray ray = {origin, direction};

            Color c = BACKGROUND;
            Intersection intersection = trace_ray(scene, &ray);
            if (intersection.object != NULL) {
                c = cast_ray(scene, &ray, 0);
            }
            plot(dev, i, j, &c);
        }
    }
}
