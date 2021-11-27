# env = Environment(tools=['mingw'])
Program("test",
        ["./src/stb_image.cpp",
         "./src/events/event.cpp",
         "./src/render/renderer.cpp",
         "./src/render/skybox.cpp",
         "./src/common/common.cpp",
         "./src/glad.c",
         "./src/test.cpp"],
        LIBS=['msvcrtd', 'libcmt', 'Gdi32', 'shell32', 'user32', 'opengl32', 'glfw3'], LIBPATH=['./libs'], CPPPATH=['./include'])

Program("gen_six",
        ["./tools/irr_gen/gen_six.cpp",
         "./src/common/common.cpp",
         "./src/stb_image.cpp",
         "./src/glad.c", ],
        LIBS=['msvcrtd', 'libcmt', 'Gdi32', 'shell32', 'user32', 'opengl32', 'glfw3'], LIBPATH=['./libs'], CPPPATH=['./include'])

Program("gen_one",
        ["./tools/irr_gen/gen_one.cpp",
         "./src/common/common.cpp",
         "./src/stb_image.cpp",
         "./src/glad.c", ],
        LIBS=['msvcrtd', 'libcmt', 'Gdi32', 'shell32', 'user32', 'opengl32', 'glfw3'], LIBPATH=['./libs'], CPPPATH=['./include'])


Program("texture_conv",
        ["./tools/texture_conv/texture_conv.cpp",
         "./src/common/common.cpp",
         "./src/stb_image.cpp",
         "./src/glad.c", ],
        LIBS=['msvcrtd', 'libcmt', 'Gdi32', 'shell32', 'user32', 'opengl32', 'glfw3'], LIBPATH=['./libs'], CPPPATH=['./include'])
