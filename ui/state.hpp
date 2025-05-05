#pragma once

#include <cstdint>
#include <memory>

namespace ui
{

    class State {
    public:
        State() = default;
        virtual ~State() = default;

        virtual void update(int delta_ms);
        virtual void draw() = 0;

        virtual void pause();
        virtual void resume();

    protected:
        bool active = false;
        uint32_t time_ms = 0;
    };

    using StatePtr = std::shared_ptr<State>;

    template<typename T, typename... Args>
    std::shared_ptr<T> make_state(Args&& ... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

}
