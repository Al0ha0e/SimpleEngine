#ifndef EVENT_H
#define EVENT_H

#include <map>
#include <vector>
#include <memory>

namespace common
{
    typedef long long hdlid_t;

    enum EventType
    {
        EVENT_WINDOW_RESIZE,
        EVENT_KEYBOARD_PRESS,
        EVENT_MOUSE_MOVEMENT,
        EVENT_CUSTOM
    };

    struct EventDescriptor
    {
    };

    struct ED_WindowResize : public EventDescriptor
    {
        int width;
        int height;
    };

    struct ED_KeyboardPress : public EventDescriptor
    {
        unsigned int keycode;
    };

    struct ED_MouseMovement : public EventDescriptor
    {
        float x, y, dx, dy;
    };

    class EventListener
    {
    public:
        virtual void OnWindowResize(std::shared_ptr<ED_WindowResize> desc) {}
        virtual void OnKeyBoardPress(std::shared_ptr<ED_KeyboardPress> desc) {}
        virtual void OnMouseMove(std::shared_ptr<ED_MouseMovement> desc) {}
    };

    class EventTransmitter
    {

    private:
        EventTransmitter() {}
        ~EventTransmitter() {}
        EventTransmitter(const EventTransmitter &);
        EventTransmitter &operator=(const EventTransmitter &);

        static EventTransmitter *instance;

        std::map<EventType, std::map<hdlid_t, std::shared_ptr<EventListener>>> handlers;
        hdlid_t handlerid;

        class Deletor
        {
        public:
            ~Deletor()
            {
                if (EventTransmitter::instance)
                    delete EventTransmitter::instance;
            }
        };
        static Deletor deletor;

    public:
        static EventTransmitter *GetInstance()
        {
            if (instance == nullptr)
            {
                instance = new EventTransmitter();
            }
            return instance;
        }

        int SubscribeEvent(EventType tp, std::shared_ptr<EventListener> handler)
        {
            auto it = handlers.find(tp);
            hdlid_t id = ++handlerid;
            if (it == handlers.end())
            {
                handlers.insert(std::pair<EventType, std::map<hdlid_t, std::shared_ptr<EventListener>>>(tp, std::map<hdlid_t, std::shared_ptr<EventListener>>()));
                it = handlers.find(tp);
            }
            it->second.insert(std::pair<hdlid_t, std::shared_ptr<EventListener>>(id, handler));
            return id;
        }

        void PublishEvent(EventType tp, std::shared_ptr<EventDescriptor> desc)
        {
            auto it = handlers.find(tp);
            if (it != handlers.end())
            {
                switch (tp)
                {
                case EventType::EVENT_WINDOW_RESIZE:
                    for (auto &handler : it->second)
                        handler.second->OnWindowResize(std::static_pointer_cast<ED_WindowResize>(desc));
                    break;
                case EventType::EVENT_KEYBOARD_PRESS:
                    for (auto &handler : it->second)
                        handler.second->OnKeyBoardPress(std::static_pointer_cast<ED_KeyboardPress>(desc));
                    break;
                case EventType::EVENT_MOUSE_MOVEMENT:
                    for (auto &handler : it->second)
                        handler.second->OnMouseMove(std::static_pointer_cast<ED_MouseMovement>(desc));
                    break;
                }
            }
        }
    };
    //EventTransmitter *EventTransmitter::instance;
}

#endif