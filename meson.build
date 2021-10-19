project('drm-raytracer', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

inc_dir = include_directories('include') 

src = [ 'main.c', 
        'src/object.c',
        'src/sphere.c',
        'src/plane.c'
      ]
exe = executable('drm_raytracer', src, include_directories: inc_dir,
  install : true)
