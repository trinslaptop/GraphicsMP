/*
 * (c) Trin Wasinger 2025
 * See input.hpp
 */

#include "input.hpp"

namespace input {
    Input::Input(const InputType type, const int value) : _value(static_cast<int>(type) + value) {}

    int Input::value() const {
        return this->_value;
    }
    
    // Get the input for a given key, pass GLFW_KEY_WHATEVER
    const Input key(const int key) {
        return Input(InputType::Key, key);
    }

    // Get the input for a given mouse button, pass GLFW_MOUSE_BUTTON_WHATEVER
    const Input mouse(const int button) {
        return Input(InputType::Mouse, button);
    }

    namespace {
        // An all false mask
        static inline constexpr const InputMask none() {
            return InputMask();
        }

        // An all true mask
        static inline const InputMask all() {
            return InputMask().set();
        }

        // Convert initializer list of inputs to mask
        static inline const InputMask create_mask(const std::initializer_list<Input> &list) {
            InputMask mask;
            for(const auto key : list) mask.set(key.value());
            return mask;
        }

        // Convert initializer list of inputs to vector of mask positions
        static inline const std::vector<int> create_vector(const std::initializer_list<Input> &list) {
            std::vector<int> vec;
            vec.reserve(list.size());
            for(const auto key : list) vec.push_back(key.value());
            return vec;
        }
    }

    StickyBinding::StickyBinding(const std::initializer_list<Input> sequence, const std::initializer_list<Input> modifiers, const std::function<void(GLFWwindow *const, const float)> callback, const Event event, const float timeout)
        : _sequence(create_vector(sequence)), _mask(create_mask(sequence)), _modifiers(create_mask(modifiers)), _callback(callback), _event(event), _progress(0), _timeout(timeout), _time(0.0f)
    {}

    // Resets progress, automatically called for timeouts and bad keys
    void StickyBinding::reset() {
        this->_progress = 0;
        this->_time = 0;
    }

    // Notifies keybind of a change or event poll to which it can respond, should return true iff anything happened and update dirty flags
    bool StickyBinding::poll(GLFWwindow *const window, const InputMask &down, InputMask &dirty, const InputMask &cause, const float deltaTime) {        
        // If any modifier becomes unpressed, reset
        if((cause & this->_modifiers & ~down) != 0) {
            this->reset();
            return false;
        }

        // If a relevant key changed on the appropriate event or this is a poll for held keys, continue checks
        if((cause & this->_mask & (this->_event == Event::Release ? ~down : down)) != 0 || cause == 0) {
            // If all modifiers are down and the next input in the sequence is in the right state and is dirty
            // (dirty condition is ignored for hold events), consume input and step sequence.
            // Otherwise, if not a tick poll, reset
            if((this->_modifiers & down) == this->_modifiers && (
                (this->_event == Event::Release ? ~down : down)
                & (this->_event == Event::Hold ? all() : cause & dirty)
            ).test(this->_sequence[this->_progress])) {
                dirty &= ~cause;
                this->_progress++;
                this->_time = 0;
            } else if(cause != 0) {
                this->reset();
                return false;
            }
        }

        // If complete, run callback and reset
        if(this->_progress == this->_sequence.size()) {
            this->reset();
            this->_callback(window, deltaTime);
            return true;
        }

        // Update timeout if enabled
        if(this->_timeout != -1) {
            this->_time += deltaTime;
            if(this->_time >= this->_timeout) {
                this->reset();
            }
        }

        return false;
    }

    InputManager::InputManager() : _keys(), _dirty(), _cursor(0.0f, 0.0f), _scroll(0.0f, 0.0f), _key_bindings(), _axis_bindings() {}

    void InputManager::_poll(GLFWwindow *const window, const InputMask &cause, const float deltaTime) {
        for(StickyBinding& binding : this->_key_bindings) binding.poll(window, this->_keys, this->_dirty, cause, deltaTime);
    }

    // Update or read the cursor position
    glm::vec2 InputManager::cursor(const std::optional<glm::vec2> pos) {
        if(pos) {
            this->dispatch_axis(AxisType::Cursor, pos.value());
            this->_cursor.x = pos.value().x;
            this->_cursor.y = pos.value().y;
        }
        return this->_cursor;
    }

    // Update or read the scroll offset
    glm::vec2 InputManager::scroll(const std::optional<glm::vec2> offset) {
        if(offset) {
            this->dispatch_axis(AxisType::Scroll, offset.value());
            this->_scroll.x = offset.value().x;
            this->_scroll.y = offset.value().y;
        }
        return this->_scroll;
    }


    // Call from an event loop to update held bindings and timeouts
    void InputManager::poll(GLFWwindow *const window, const float deltaTime) {
        this->_poll(window, none(), deltaTime);
    }

    // Trigger a change in a key or button state
    void InputManager::dispatch(GLFWwindow *const window, const Input input, const State event) {
        _keys.set(input.value(), static_cast<int>(event));
        _dirty.set(input.value());
        this->_poll(window, create_mask({input}), 0.0f);
    }

    // Add a keybinding
    void InputManager::on(const std::initializer_list<Input> sequence, const std::initializer_list<Input> modifiers, const std::function<void(GLFWwindow *const, const float)> callback, const Event event, const float timeout) {
        this->_key_bindings.push_back(StickyBinding(sequence, modifiers, callback, event, timeout));
    }

    // Adds an axis (cursor, scroll) binding
    void InputManager::on_axis(const AxisType axis, const std::function<void(const glm::vec2)> callback) {
        this->_axis_bindings[axis].push_back(callback);
    }

    // Trigger a change in an axis
    void InputManager::dispatch_axis(const AxisType axis, const glm::vec2 value) {
        for(const auto& f : this->_axis_bindings[axis]) {
            f(value);
        }
    }

    // Check if a button or key is down
    bool InputManager::is_down(const Input input) const {
        return this->_keys[input.value()];
    }
}
