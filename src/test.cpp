#include "./common/common.h"
#include "./common/materials.h"
#include "./common/game_object.h"
#include "./events/event.h"
#include "./common/scene.h"

#include <iostream>
#include <fstream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

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
            object->TranslateLocal(glm::vec3(0, 0, -moveSpeed));
            // object->TranslateLocal(glm::vec3(0, moveSpeed, 0));
            break;
        case GLFW_KEY_A:
            object->TranslateLocal(glm::vec3(-moveSpeed, 0, 0));
            break;
        case GLFW_KEY_S:
            object->TranslateLocal(glm::vec3(0, 0, moveSpeed));
            // object->TranslateLocal(glm::vec3(0, -moveSpeed, 0));
            break;
        case GLFW_KEY_D:
            object->TranslateLocal(glm::vec3(moveSpeed, 0, 0));
            break;
        }
    }

    virtual void OnMouseMove(std::shared_ptr<common::ED_MouseMovement> desc)
    {
        float xoffset = desc->dx;
        float yoffset = desc->dy;
        xoffset *= rotateSpeed;
        yoffset *= rotateSpeed;
        object->RotateGlobal(-xoffset, glm::vec3(0.0f, 1.0f, 0.0f));
        object->RotateLocal(yoffset, glm::vec3(1.0f, 0.0f, 0.0f));
    }
};

std::shared_ptr<common::GameObject> MakeCamera(
    std::shared_ptr<renderer::Renderer> rder,
    glm::vec3 pos,
    glm::vec3 dir,
    float fov,
    float aspect,
    float near,
    float far)
{
    auto tpCam = std::make_shared<common::TransformParameter>(pos, dir);
    auto camObject = std::make_shared<common::GameObject>(tpCam, rder);
    auto cam = std::make_shared<builtin_components::Camera>(camObject, fov, aspect, near, far);
    camObject->AddComponent(cam);
    return camObject;
}

std::shared_ptr<common::GameObject> MakeLight(
    std::shared_ptr<renderer::Renderer> rder,
    renderer::LightType tp,
    glm::vec3 pos,
    glm::vec3 dir,
    glm::vec3 color,
    float intensity,
    float spotangle)
{
    auto tpLight = std::make_shared<common::TransformParameter>(pos, dir);
    auto inner_lp = renderer::InnerLightParameters(glm::vec4(glm::vec3(), intensity), glm::vec4(color, 0.0f), glm::vec4(glm::vec3(), spotangle));
    auto light_prop = std::make_shared<renderer::LightParameters>(tp, false, inner_lp);
    auto lightObject = std::make_shared<common::GameObject>(tpLight, rder);
    auto light = std::make_shared<builtin_components::Light>(lightObject, light_prop);
    lightObject->AddComponent(light);
    return lightObject;
}

std::shared_ptr<common::Scene> LoadScene(std::shared_ptr<renderer::Renderer> rder, resources::ResourceManager *manager, std::string pth)
{
    auto ret = std::make_shared<common::Scene>(rder);
    std::ifstream infile(pth);
    std::stringstream buffer;
    buffer << infile.rdbuf();
    std::string s(buffer.str());
    infile.close();
    ret->UnserializeJSON(s, manager);
    return ret;
}

void SaveScene(common::Scene &scene, std::string pth)
{
    std::string s = scene.SerializeJSON();
    std::ofstream outjson(pth);
    outjson << scene.SerializeJSON();
    outjson.close();
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SimpleEngine", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    float camSpeed = 0.01;

    auto rManager = resources::ResourceManager();

    auto skybox = rManager.LoadMeta<renderer::SkyBox>("./assets/skybox/skybox.json");

    auto rder = std::make_shared<renderer::Renderer>(glm::vec4(0.3f, 0.3f, 0.3f, 0.0f), skybox);
    auto scene = LoadScene(rder, &rManager, "./assets/scene/scene.json");
    scene->OnStart();

    // auto lightObject1 = MakeLight(rder, renderer::DIRECTIONAL_LIGHT,
    //                               glm::vec3(), glm::vec3(glm::radians(90.0f), 0.0f, 0.0f),
    //                               glm::vec3(1.0f, 1.0f, 0.7f),
    //                               2.7f, 0.0f);
    // lightObject1->OnStart();
    // scene.objects.push_back(lightObject1);

    // auto lightObject2 = MakeLight(rder, renderer::SPOT_LIGHT,
    //                               glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(glm::radians(150.0f), 0.0f, 0.0f),
    //                               glm::vec3(1.0f, 0.0f, 0.0f),
    //                               2.5f, glm::cos(glm::radians(30.5f)));
    // lightObject2->OnStart();
    // scene.objects.push_back(lightObject2);

    // auto lightObject3 = MakeLight(rder, renderer::POINT_LIGHT,
    //                               glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(),
    //                               glm::vec3(0.0f, 0.0f, 1.0f),
    //                               2.6f, 0.0f);
    // lightObject3->OnStart();
    // scene.objects.push_back(lightObject3);

    // auto camObject = MakeCamera(
    //     rder,
    //     glm::vec3(0.0f, 0.0f, 2.0f),
    //     glm::vec3(),
    //     glm::radians(45.0f),
    //     SCR_WIDTH * 1.0f / SCR_HEIGHT,
    //     0.1f,
    //     100.0f);
    // camObject->OnStart();
    // scene.objects.push_back(camObject);

    // auto tp = std::make_shared<common::TransformParameter>(glm::vec3(), glm::vec3());
    // auto tp1 = std::make_shared<common::TransformParameter>(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3());
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // auto mesh = std::make_shared<common::ModelMesh>("./assets/models/test2.txt");
    // auto mesh1 = std::make_shared<common::ModelMesh>("./assets/models/test3.txt");

    // auto mat_args = std::make_shared<common::RenderArguments>();
    // auto mat_args1 = std::make_shared<common::RenderArguments>();

    // unsigned int id = rder->GetMaterialID(renderer::OPAQUE);
    // unsigned int id1 = rder->GetMaterialID(renderer::OPAQUE);
    // auto material = rManager.LoadMeta<builtin_materials::CustomMaterial>("./assets/materials/pbr.json");
    // material->material_id = id;
    // auto material1 = rManager.LoadMeta<builtin_materials::CustomMaterial>("./assets/materials/parallax_pbr.json");
    // material1->material_id = id1;
    // rder->RegisterMaterial(id, material, renderer::OPAQUE);
    // rder->RegisterMaterial(id1, material1, renderer::OPAQUE);
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // auto object = std::make_shared<common::GameObject>(tp, rder);
    // auto object1 = std::make_shared<common::GameObject>(tp1, rder);

    // auto render_component = std::make_shared<builtin_components::RenderableObject>(
    //     object,
    //     "./assets/materials/parallax_pbr.json",
    //     "./assets/models/test2.txt",
    //     &rManager);
    // object->AddComponent(render_component);
    // object->OnStart();
    // scene.objects.push_back(object);

    // auto render_component1 = std::make_shared<builtin_components::RenderableObject>(
    //     object1,
    //     "./assets/materials/pbr.json",
    //     "./assets/models/test3.txt",
    //     &rManager);
    // object1->AddComponent(render_component1);
    // object1->OnStart();
    // scene.objects.push_back(object1);

    // SaveScene(scene,"./assets/scene/scene.json");

    TestController controller(scene->objects[3], camSpeed, 0.0005f);
    // TestController controller(object1, camSpeed, 0.0005f);
    // TestController controller(lightObject3, camSpeed, 0.0005f);
    //  render loop
    //  -----------
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