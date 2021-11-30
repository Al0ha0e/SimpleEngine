#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../src/common/common.h"

static float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f};

inline int getP2(int x)
{
    int i;
    for (i = 31; i >= 0; --i)
    {
        if (x & (1 << i))
            break;
    }
    if (x ^ (1 << i))
        return 1 << (i + 1);
    return x;
}

//mode hdrpath outpath outsz
int main(int argc, char *argv[])
{
    std::string mode(argv[1]);
    char *hdrpath = argv[2];
    std::string outpath(argv[3]);
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(512, 512, "GenIrr", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float *data = stbi_loadf(hdrpath, &width, &height, &nrComponents, 0);
    unsigned int hdrTexture;
    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
        return -1;
    }
    width = getP2(width);
    height = getP2(height);

    // configure global opengl state
    // -----------------------------

    std::shared_ptr<common::ShaderProgram> shader;

    if (mode == "skybox")
    {
        shader = std::make_shared<common::ShaderProgram>(
            common::Shader("./tools/irr_gen/gen1.vs", common::VERTEX_SHADER),
            common::Shader("./tools/irr_gen/gen1.fs", common::FRAGMENT_SHADER));
    }
    else if (mode == "irr")
    {
        shader = std::make_shared<common::ShaderProgram>(
            common::Shader("./tools/irr_gen/gen1.vs", common::VERTEX_SHADER),
            common::Shader("./tools/irr_gen/gen_irr1.fs", common::FRAGMENT_SHADER));
        width >>= 3;
        height >>= 3;
    }

    glUseProgram(shader->shader);
    glUniform1i(glGetUniformLocation(shader->shader, "equirectangularMap"), 0);
    // glUniform1f(glGetUniformLocation(shader->shader, "width"), width);

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);

    unsigned int pbo;
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(float) * width * height, NULL, GL_STREAM_COPY);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, captureRBO);

    glUseProgram(shader->shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glBindVertexArray(vao);

    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    void *scrdata = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE);
    stbi_write_png((outpath + mode + ".png").c_str(), width, height, 3, scrdata, 0);
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    std::ofstream outjson(outpath + mode + ".json");
    outjson << "{\n";
    outjson << "\"wraps\":" << GL_CLAMP_TO_EDGE << ",\n";
    outjson << "\"wrapt\":" << GL_CLAMP_TO_EDGE << ",\n";
    outjson << "\"minfilter\":" << GL_LINEAR << ",\n";
    outjson << "\"magfilter\":" << GL_LINEAR << ",\n";
    outjson << "\"miplevel\":1,\n";
    outjson << "\"automip\":false,\n";
    outjson << "\"paths\":[\"" << outpath + mode + ".png"
            << "\"]\n";
    outjson << "}\n";
    outjson.close();

    return 0;
}