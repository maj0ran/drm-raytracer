#include "scene.h"
#include "color.h"
#include "draw.h"
#include "object.h"
#include "sphere.h"
#include "vector.h"
#include <math.h>

#define EPS 0.01

void scene_init(Scene *scene, size_t capacity) {
    scene->objects = malloc(sizeof(Element *) * capacity);
    scene->capacity = capacity;
    scene->size = 0;

    scene->spotlights = NULL;
}

int scene_add_object(Scene *scene, Element *obj) {
    if (scene->size == scene->capacity)
        return -1;

    scene->objects[scene->size] = obj;
    scene->size++;

    return 0;
}

int scene_add_spotlight(Scene *scene, Spotlight *light) {
    if (scene->spotlights == NULL) {
        scene->spotlights = malloc(sizeof(struct Spotlight_list));
        scene->spotlights->light = *light;
        scene->spotlights->next = NULL;
        return 0;
    }

    Spotlight_list *iter = scene->spotlights;
    while (iter->next != NULL)
        iter = iter->next;

    iter->next = malloc(sizeof(struct Spotlight_list));
    iter->next->light = *light;
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
void set_sunlight(Scene *scene, Vec *sun_direction, float sun_intensity) {
    scene->sunlight.direction = *sun_direction;
    scene->sunlight.intensity = sun_intensity;
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
        Color refraction_color = {.r = 0.0, .g = 0.0, .b = 0.0};
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
        Color c = {.r = 0.0, .g = 0.0, .b = 0.0};
        return c;
    }

    Intersection i = trace_ray(scene, ray);
    if (i.object != NULL) {
        return get_color(scene, ray, &i, depth);
    }

    Color c = {.r = 0.0, .g = 0.0, .b = 0.0};
    return c;
}

void wrap(const Texture *tex, TextureCoords *coords) {
    coords->x %= tex->width;
    coords->y %= tex->height;
}

Color shadow_diffuse(Scene *scene, Ray *ray, Intersection *i) {
    Color c = {.r = 0.0, .g = 0.0, .b = 0.0};

    Point hit_point = v_mul_s(&ray->direction, i->distance);
    hit_point = v_add(&ray->origin, &hit_point);
    Vec normal = surface_normal(i->object, &hit_point);
    Vec offset = v_mul_s(&normal, EPS);

    // Sunlight
    Vec dir_to_light = v_neg(&scene->sunlight.direction);
    dir_to_light = v_normalize(&dir_to_light);
    float light_intensity;
    float light_power;
    Color light_color;

    Ray shadow_ray = {.origin = v_add(&hit_point, &offset),
                      .direction = dir_to_light};

    Intersection shadow_intersect = trace_ray(scene, &shadow_ray);
    bool in_light = (shadow_intersect.object == NULL);
    if (in_light) {
        light_intensity = scene->sunlight.intensity;
    } else {
        light_intensity = 0.0;
    }
    light_power = fmax(v_dot(&normal, &dir_to_light), 0.0) * light_intensity;
    Color *surface_color;
    if (i->object->is_textured) {
        Texture *tex = &i->object->texture;
        TextureCoords coords = texture_coords(i->object, &hit_point);
        wrap(tex, &coords);
        surface_color = tex->data + (tex->width * coords.y + coords.x);
    } else {
        surface_color = &i->object->color;
    }
    c = c_add(&c, surface_color);
    c = c_mul_s(&c, light_power);

    // Spotlights
    Spotlight_list *iter = scene->spotlights;
    while (iter) {
        Spotlight *spotlight = &iter->light;
        dir_to_light = v_sub(&spotlight->position, &hit_point);
        dir_to_light = v_normalize(&dir_to_light);
        shadow_ray.origin = v_mul_s(&normal, EPS);
        shadow_ray.origin = v_add(&hit_point, &shadow_ray.origin);
        shadow_ray.direction = dir_to_light;

        shadow_intersect = trace_ray(scene, &shadow_ray);
        in_light = (shadow_intersect.object == NULL);
        if (in_light) {
            Vec distance_vector = v_sub(&spotlight->position, &hit_point);
            float r2 = v_len2(&distance_vector);
            light_intensity = spotlight->intensity / (4.0 * 3.141592653 * r2);
        } else {
            light_intensity = 0.0;
        }

        light_power = v_dot(&normal, &dir_to_light) * light_intensity * 0.5;
        light_color = c_mul_s(&spotlight->color, light_power);
        light_color = c_mul(&i->object->color, &light_color);
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
    Vec origin = {.x = 960, .y = 540, .z = -600};
    // One Ray through each pixel
    for (int i = 0; i < 1920; i++) {
        for (int j = 0; j < 1080; j++) {
            Vec target = {.x = i, .y = j, .z = 0};
            Vec direction = v_sub(&target, &origin);
            direction = v_normalize(&direction);
            Ray ray = {origin, direction};

            Color c = {.r = 0.1, .g = 0.6, .b = 0.8};
            Intersection intersection = trace_ray(scene, &ray);
            if (intersection.object != NULL) {
                c = cast_ray(scene, &ray, 0);
            }
            plot(dev, i, j, &c);
        }
    }
}
