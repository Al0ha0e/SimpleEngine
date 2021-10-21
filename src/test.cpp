#include "./common/common.h"
#include "./common/materials.h"
#include "./common/game_object.h"
#include "./events/event.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

class TestController : public common::EventListener
{
public:
    std::shared_ptr<common::GameObject> object;
    float moveSpeed;
    float rotateSpeed;

    TestController() {}
    TestController(std::shared_ptr<common::GameObject> object,
                   float moveSpeed,
                   float rotateSpeed)
        : object(object), moveSpeed(moveSpeed), rotateSpeed(rotateSpeed)
    {
        common::EventTransmitter::GetInstance()->SubscribeEvent(
            common::EventType::EVENT_KEYBOARD_PRESS,
            std::static_pointer_cast<common::EventListener>(std::shared_ptr<TestController>(this)));

        common::EventTransmitter::GetInstance()->SubscribeEvent(
            common::EventType::EVENT_MOUSE_MOVEMENT,
            std::static_pointer_cast<common::EventListener>(std::shared_ptr<TestController>(this)));
    }

    virtual void OnKeyBoardPress(std::shared_ptr<common::ED_KeyboardPress> desc)
    {
        switch (desc->keycode)
        {
        case GLFW_KEY_W:
            object->Transform(glm::vec3(0, moveSpeed, 0));
            break;
        case GLFW_KEY_A:
            object->Transform(glm::vec3(-moveSpeed, 0, 0));
            break;
        case GLFW_KEY_S:
            object->Transform(glm::vec3(0, -moveSpeed, 0));
            break;
        case GLFW_KEY_D:
            object->Transform(glm::vec3(moveSpeed, 0, 0));
            break;
        }
    }

    virtual void OnMouseMove(std::shared_ptr<common::ED_MouseMovement> desc)
    {
        float xoffset = desc->dx;
        float yoffset = desc->dy;
        xoffset *= rotateSpeed;
        yoffset *= rotateSpeed;
        object->Rotate(xoffset, yoffset);
    }
};

int main()
{

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SimpleEngine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glm::mat4 model(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.5f, 0.0f));

    glm::mat4 camModel;
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

    common::TransformParameter tp(model, glm::vec3());
    common::TransformParameter tpCam(camModel, cameraPos);

    auto camObject = std::make_shared<common::GameObject>(tpCam);

    glm::mat4 view;
    glm::mat4 projection(1.0f);
    projection = glm::perspective(glm::radians(45.0f), SCR_WIDTH * 1.0f / SCR_HEIGHT, 0.1f, 100.0f);

    float camSpeed = 0.01;

    auto rder = std::make_shared<renderer::Renderer>(view, projection);

    auto cam = std::make_shared<builtin_components::Camera>(camObject, rder, projection);
    camObject->AddComponent(cam);

    auto shader = std::make_shared<common::ShaderProgram>(common::Shader("./assets/shaders/v.txt", common::VERTEX_SHADER),
                                                          common::Shader("./assets/shaders/f.txt", common::FRAGMENT_SHADER));

    auto texture = std::make_shared<common::Texture>("./assets/textures/container.jpg");

    auto mesh = std::make_shared<common::ModelMesh>("./assets/models/test.txt");

    auto mat_args = std::make_shared<common::RenderArguments>(model, view, projection);

    auto material = std::make_shared<builtin_materials::NaiveMaterial>(shader, texture, mat_args);

    auto object = std::make_shared<common::GameObject>(tp);

    auto render_component = std::make_shared<builtin_components::RenderableObject>(object, rder, material, mesh);

    object->AddComponent(render_component);

    object->OnStart();

    TestController controller(camObject, camSpeed, 0.05f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);

        rder->Render();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    object->Dispose();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        auto desc = std::make_shared<common::ED_KeyboardPress>();
        desc->keycode = GLFW_KEY_A;
        common::EventTransmitter::GetInstance()->PublishEvent(
            common::EventType::EVENT_KEYBOARD_PRESS,
            std::static_pointer_cast<common::EventDescriptor>(desc));
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        auto desc = std::make_shared<common::ED_KeyboardPress>();
        desc->keycode = GLFW_KEY_W;
        common::EventTransmitter::GetInstance()->PublishEvent(
            common::EventType::EVENT_KEYBOARD_PRESS,
            std::static_pointer_cast<common::EventDescriptor>(desc));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        auto desc = std::make_shared<common::ED_KeyboardPress>();
        desc->keycode = GLFW_KEY_S;
        common::EventTransmitter::GetInstance()->PublishEvent(
            common::EventType::EVENT_KEYBOARD_PRESS,
            std::static_pointer_cast<common::EventDescriptor>(desc));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        auto desc = std::make_shared<common::ED_KeyboardPress>();
        desc->keycode = GLFW_KEY_D;
        common::EventTransmitter::GetInstance()->PublishEvent(
            common::EventType::EVENT_KEYBOARD_PRESS,
            std::static_pointer_cast<common::EventDescriptor>(desc));
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    auto desc = std::make_shared<common::ED_WindowResize>();
    desc->height = height;
    desc->width = width;
    common::EventTransmitter::GetInstance()->PublishEvent(
        common::EventType::EVENT_WINDOW_RESIZE,
        std::static_pointer_cast<common::EventDescriptor>(desc));
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    static bool firstMouse = true;
    static float lastX, lastY;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    auto desc = std::make_shared<common::ED_MouseMovement>();
    desc->dx = xoffset;
    desc->dy = yoffset;
    desc->x = xpos;
    desc->y = ypos;
    common::EventTransmitter::GetInstance()->PublishEvent(
        common::EventType::EVENT_MOUSE_MOVEMENT,
        std::static_pointer_cast<common::EventDescriptor>(desc));
}