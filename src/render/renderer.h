#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "../common/common.h"
#include "../events/event.h"

#include <vector>
#include <list>
#include <map>

namespace renderer
{

    class Camera : public common::EventListener
    {
    public:
        //TODO: bind camera movement to a game object
        float cameraSpeed;
        float yaw;
        float pitch;
        glm::vec3 cameraPos;
        glm::vec3 cameraFront;
        glm::vec3 cameraUp;
        glm::mat4 view;
        glm::mat4 projection;

        Camera() {}
        Camera(
            float speed,
            glm::vec3 pos,
            glm::vec3 up,
            glm::mat4 projection) : cameraSpeed(speed), cameraPos(pos), cameraUp(up), projection(projection)
        {
            yaw = 270;
            pitch = 0;
            calcFront();
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

            common::EventTransmitter::GetInstance()->SubscribeEvent(
                common::EventType::EVENT_WINDOW_RESIZE,
                std::static_pointer_cast<common::EventListener>(std::shared_ptr<Camera>(this)));

            common::EventTransmitter::GetInstance()->SubscribeEvent(
                common::EventType::EVENT_KEYBOARD_PRESS,
                std::static_pointer_cast<common::EventListener>(std::shared_ptr<Camera>(this)));

            common::EventTransmitter::GetInstance()->SubscribeEvent(
                common::EventType::EVENT_MOUSE_MOVEMENT,
                std::static_pointer_cast<common::EventListener>(std::shared_ptr<Camera>(this)));
        }

        virtual void OnWindowResize(std::shared_ptr<common::ED_WindowResize> desc)
        {
            projection = glm::perspective(glm::radians(45.0f), desc->width * 1.0f / desc->height, 0.1f, 100.0f);
        }

        virtual void OnKeyBoardPress(std::shared_ptr<common::ED_KeyboardPress> desc)
        {
            switch (desc->keycode)
            {
            case GLFW_KEY_W:
                cameraPos += cameraSpeed * cameraFront;
                break;
            case GLFW_KEY_A:
                cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
                break;
            case GLFW_KEY_S:
                cameraPos -= cameraSpeed * cameraFront;
                break;
            case GLFW_KEY_D:
                cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
                break;
            }
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        }

        virtual void OnMouseMove(std::shared_ptr<common::ED_MouseMovement> desc)
        {
            float sensitivity = 0.05;
            float xoffset = desc->dx;
            float yoffset = desc->dy;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            yaw += xoffset;
            pitch += yoffset;

            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
            calcFront();
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        }

    private:
        void calcFront()
        {
            glm::vec3 front;
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraFront = glm::normalize(front);
        }
    };

    enum RenderMode
    {
        OPAQUE,
        OPAQUE_SHADOW,
        TRANSPARENT,
        TRANSPARENT_SHADOW
    };

    struct RenderQueueItem
    {
        std::shared_ptr<common::Material> material;
        std::shared_ptr<common::ModelMesh> mesh;
        unsigned int vertex_cnt;

        virtual void Draw(glm::mat4 view, glm::mat4 projection)
        {
            material->UpdateV(view);
            material->UpdateP(projection);
            material->PrepareForDraw();
            mesh->PrepareForDraw();
            glDrawElements(GL_TRIANGLES, vertex_cnt, GL_UNSIGNED_INT, 0);
        }

        RenderQueueItem() {}
        RenderQueueItem(std::shared_ptr<common::Material> material,
                        std::shared_ptr<common::ModelMesh> mesh,
                        unsigned int vertex_cnt) : material(material), mesh(mesh), vertex_cnt(vertex_cnt) {}
    };

    typedef unsigned long long render_id;

    struct RenderQueue
    {
        std::map<unsigned int, RenderQueueItem> queue;
        render_id maxid;

        render_id GetRenderID() { return ++maxid; }

        void InsertItem(render_id id, RenderQueueItem item)
        {
            queue[id] = item;
        }

        void RemoveItem(render_id id)
        {
            if (queue.find(id) == queue.end())
                return;
            queue.erase(queue.find(id));
        }
    };

    class Renderer
    {
    public:
        Renderer() {}
        Renderer(std::shared_ptr<Camera> main_camera) : main_camera(main_camera)
        {
            glEnable(GL_DEPTH_TEST);
        }
        void Render();
        render_id GetRenderID(RenderMode mode);
        void InsertToQueue(render_id id, RenderQueueItem item, RenderMode mode);
        void RemoveFromQueue(render_id id, RenderMode mode);

    private:
        RenderQueue opaque_queue;
        RenderQueue transparent_queue;
        RenderQueue opaque_shadow_queue;
        RenderQueue transparent_shadow_queue;
        std::shared_ptr<Camera> main_camera;
    };
}

#endif