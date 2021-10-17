# env = Environment(tools=['mingw'])
Program("test", ["./src/stb_image.cpp", "./src/events/event.cpp", "./src/render/renderer.cpp", "./src/glad.c", "./src/test.cpp"],
        LIBS=['msvcrtd', 'libcmt', 'Gdi32', 'shell32', 'user32', 'opengl32', 'glfw3'], LIBPATH=['./libs'], CPPPATH=['./include'])
