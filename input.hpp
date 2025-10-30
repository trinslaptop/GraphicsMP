/*
 *  CSCI 441, Computer Graphics, Fall 2025
 *
 *  Project: A3
 *  File: input.hpp
 *
 *	Author: Trin Wasinger - Fall 2025
 *
 *  Description: A set of classes to abstract out key and mouse inputs and also support held states
 *  in a reliable manner
 */

#ifndef INPUT_HPP
#define INPUT_HPP

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include <optional>
#include <functional>
#include <bitset>

namespace input {
    // An update to a key's state, there is no hold state, this is only for changes
    enum class State {
        Release = GLFW_RELEASE,
        Press = GLFW_PRESS
    };

    // An event that can be listened for
    enum class Event {
        Release = GLFW_RELEASE,
        Press = GLFW_PRESS,
        Hold = GLFW_REPEAT,
    };

    // A type of vector input
    enum class AxisType {
        Cursor = 0,
        Scroll = 1
    };

    // A type of input, may be expanded later for controllers, etc...
    enum class InputType {
        Mouse = 0,
        Key = Mouse + GLFW_MOUSE_BUTTON_LAST + 1 // Shift past mouse button range
    };

    // Represents some input source of a given type effectively unifying handling of mouse and keyboard
    class Input final {
        private:
            int _value;
        public:
            Input(const InputType type, const int value);
            int value() const;
    };

    // Get the input for a given key, pass GLFW_KEY_WHATEVER
    const Input key(const int key);

    // Get the input for a given mouse button, pass GLFW_MOUSE_BUTTON_WHATEVER
    const Input mouse(const int button);

    namespace {
        // A mask large enough to store a boolean for each possible input
        static const int INPUT_SPACE_SIZE = GLFW_MOUSE_BUTTON_LAST + GLFW_KEY_LAST + 2;
        typedef std::bitset<INPUT_SPACE_SIZE> InputMask;
    }

    // An ordered collection of keys or mouse buttons that must trigger in sequence before the binding resets
    class StickyBinding final {
        private:
            // The ordered inputs to watch for, these must be pressed in order before being reset to trigger the binding (may contain repeats)
            const std::vector<int> _sequence;

            // The unique keys that compose the above sequence, when one is hit out of order, resets progress
            const InputMask _mask;

            // These keys must be held down to to trigger inputs, but are not themselves consumed
            const InputMask _modifiers;

            // The event to listen for
            const Event _event;

            // What to do when triggered
            const std::function<void(GLFWwindow *const, const float)> _callback;

            // Automatically clear progress after this many seconds (-1 for never), useless if sequence length is one
            const float _timeout;
            
            // Current progress through input sequence (index of next expected) 
            size_t _progress;

            // Accumulated time since last change in progress
            float _time;
        
        public:
            StickyBinding(const std::initializer_list<Input> sequence, const std::initializer_list<Input> modifiers, const std::function<void(GLFWwindow *const, const float)> callback, const Event event = Event::Press, const float timeout = -1);

            // Resets progress, automatically called for timeouts and bad keys, useless if sequence length is one
            void reset();

            // Notifies keybind of a change or event poll to which it can respond, should return true iff anything happened and update dirty flags
            bool poll(GLFWwindow *const window, const InputMask &down, InputMask &dirty, const InputMask &cause, const float deltaTime);
    };

    // A class to manage inputs for a program, create an instance and feed it events with cursor() or dispatch()
    class InputManager final {
        private:
            // Pressed keys and unhandled keys respectively
            InputMask _keys, _dirty;

            // The current mouse xy
            glm::vec2 _cursor;
            
            // The current scroll xy
            glm::vec2 _scroll;

            // Different key handlers
            std::vector<StickyBinding> _key_bindings;

            // Different axis handlers
            std::unordered_map<AxisType, std::vector<std::function<void(const glm::vec2)>>> _axis_bindings;

            // Polls each binding. May be called when a key is dispatched (the cause mask will list the key, and delta time will be zero)
            // or when the event loop polls (the cause will be zero and delta time will have the seconds since last tick) 
            void _poll(GLFWwindow *const window, const InputMask &cause, const float deltaTime);

        public:
            InputManager();

            // Non-Copyable
            InputManager(const InputManager&) = delete;
            InputManager& operator=(const InputManager&) = delete;

            // Update or read the cursor position
            glm::vec2 cursor(const std::optional<glm::vec2> pos = std::nullopt);

            // Update or read scroll offset
            glm::vec2 scroll(const std::optional<glm::vec2> offset = std::nullopt);


            // Call from an event loop to update held bindings and timeouts
            void poll(GLFWwindow *const window, const float deltaTime);

            // Trigger a change in a key or button state
            void dispatch(GLFWwindow *const window, const Input input, const State event);

            // Adds a keybinding
            // The inputs in sequence must all trigger the event in order while all modifier keys remain held down
            // Hitting a wrong key, releasing a modifier, or a timeout all reset progress.
            // When the sequence is a single input, this is effectively the same as a more primitive binding but also allows
            // complicated stateful inputs like the Konami Code
            void on(const std::initializer_list<Input> sequence, const std::initializer_list<Input> modifiers, const std::function<void(GLFWwindow *const, const float)> callback, const Event event = Event::Press, const float timeout = -1);

            // Adds an axis (cursor, scroll) binding
            // Callback parameters are x and y
            void on_axis(const AxisType axis, const std::function<void(const glm::vec2)> callback);

            // Trigger a change in an axis
            void dispatch_axis(const AxisType axis, const glm::vec2 value);
    
            // Check if a button or key is down
            bool is_down(const Input input) const;
    };
}
#endif