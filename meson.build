project('drm-raytracer', 'c',
  version : '0.1',
  default_options : ['warning_level=2', 'werror=true'])

inc_dir = include_directories('include') 

cc = meson.get_compiler('c')

libdrm = dependency('libdrm')
m_dep = cc.find_library('m', required : false)

src = [ 'main.c', 
        'src/elements.c',
        'src/vector.c',
        'src/drm_helper.c',
        'src/draw.c',
        'src/scene.c',
        'src/color.c',
        'src/texture.c',
        'src/light.c',
      ]
exe = executable('drm_raytracer',
                 src,
                 include_directories: inc_dir,
                 dependencies: [libdrm, m_dep],
                 install : true)
