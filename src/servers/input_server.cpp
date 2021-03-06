#include "input_server.h"

#include <locale>
#include <codecvt>

namespace Flint {
    std::string cpp11_codepoint_to_utf8(char32_t codepoint) {
        char utf8[4];
        char *end_of_utf8;

        char32_t const *from = &codepoint;

        std::mbstate_t mbs;
        std::codecvt_utf8<char32_t> ccv;

        if (ccv.out(mbs, from, from + 1, from, utf8, utf8 + 4, end_of_utf8)) {
            throw std::runtime_error("Bad codepoint-to-utf8 conversion!");
        }

        return {utf8, end_of_utf8};
    }

    // This should convert to whatever the system-wide character encoding
    // is for the platform (UTF-32/Linux - UCS-2/Windows)
    std::string ws_to_utf8(std::wstring const &s) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cnv;
        std::string utf8 = cnv.to_bytes(s);
        if (cnv.converted() < s.size()) {
            throw std::runtime_error("Incomplete wstring-to-utf8 conversion!");
        }
        return utf8;
    }

    std::wstring utf8_to_ws(std::string const &utf8) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cnv;
        std::wstring s = cnv.from_bytes(utf8);
        if (cnv.converted() < utf8.size()) {
            throw std::runtime_error("Incomplete utf8-to-wstring conversion!");
        }
        return s;
    }

    void InputEvent::consume() {
        consumed = true;
    }

    bool InputEvent::is_consumed() const {
        return consumed;
    }

    InputServer::InputServer() {
        // All remaining cursors are destroyed when glfwTerminate is called.
        arrow_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        ibeam_cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        crosshair_cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
        hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
        resize_cursor_h = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        resize_cursor_v = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    }

    void InputServer::init(GLFWwindow *window) {
        // GLFW input callbacks.
        current_window = window;

        // A lambda function that doesn't capture anything can be implicitly converted to a regular function pointer.
        auto cursor_position_callback = [](GLFWwindow *window, double x_pos, double y_pos) {
            InputEvent input_event{};
            input_event.type = InputEventType::MouseMotion;
            input_event.args.mouse_motion.position = {(float) x_pos, (float) y_pos};
            auto input_server = InputServer::get_singleton();
            input_server->last_cursor_position = input_server->cursor_position;
            input_server->cursor_position = {(float) x_pos, (float) y_pos};
            input_event.args.mouse_motion.relative = input_server->cursor_position - input_server->last_cursor_position;
            input_server->input_queue.push_back(input_event);
        };
        glfwSetCursorPosCallback(window, cursor_position_callback);

        auto cursor_button_callback = [](GLFWwindow *window, int button, int action, int mods) {
            InputEvent input_event{};
            input_event.type = InputEventType::MouseButton;
            input_event.args.mouse_button.button = button;
            input_event.args.mouse_button.pressed = action == GLFW_PRESS;
            auto input_server = InputServer::get_singleton();
            input_event.args.mouse_button.position = input_server->cursor_position;
            input_server->input_queue.push_back(input_event);
        };
        glfwSetMouseButtonCallback(window, cursor_button_callback);

        auto key_callback = [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            auto input_server = InputServer::get_singleton();

            InputEvent input_event{};
            input_event.type = InputEventType::Key;
            input_event.args.key.pressed = action == GLFW_PRESS;
            input_event.args.key.repeated = action == GLFW_REPEAT;
            input_event.args.key.key = KeyCode::UNKNOWN;

            switch (key) {
                case GLFW_KEY_BACKSPACE: {
                    input_event.args.key.key = KeyCode::BACKSPACE;
                }
                    break;
                case GLFW_KEY_LEFT: {
                    input_event.args.key.key = KeyCode::LEFT;
                }
                    break;
                case GLFW_KEY_RIGHT: {
                    input_event.args.key.key = KeyCode::RIGHT;
                }
                    break;
                case GLFW_KEY_UP: {
                    input_event.args.key.key = KeyCode::UP;
                }
                    break;
                case GLFW_KEY_DOWN: {
                    input_event.args.key.key = KeyCode::DOWN;
                }
                    break;
                default: {
                    input_event.args.key.key = KeyCode::UNKNOWN;
                }
            }

            input_server->input_queue.push_back(input_event);
        };
        glfwSetKeyCallback(window, key_callback);

        auto character_callback = [](GLFWwindow *window, unsigned int codepoint) {
            InputEvent input_event{};
            input_event.type = InputEventType::Text;
            input_event.args.text.codepoint = codepoint;
            auto input_server = InputServer::get_singleton();
            input_server->input_queue.push_back(input_event);
        };

        glfwSetCharCallback(window, character_callback);
    }

    void InputServer::collect_events() {
        input_queue.clear();

        glfwPollEvents();
    }

    void InputServer::set_cursor(CursorShape shape) {
        GLFWcursor *current_cursor{};

        switch (shape) {
            case CursorShape::ARROW: {
                current_cursor = arrow_cursor;
            }
                break;
            case CursorShape::IBEAM: {
                current_cursor = ibeam_cursor;
            }
                break;
            case CursorShape::CROSSHAIR: {
                current_cursor = crosshair_cursor;
            }
                break;
            case CursorShape::HAND: {
                current_cursor = hand_cursor;
            }
                break;
            case CursorShape::HRESIZE: {
                current_cursor = resize_cursor_h;
            }
                break;
            case CursorShape::VRESIZE: {
                current_cursor = resize_cursor_v;
            }
                break;
        }

        glfwSetCursor(current_window, current_cursor);
    }

    void InputServer::set_cursor_captured(bool captured) {
        glfwSetInputMode(current_window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    void InputServer::hide_cursor() {
        glfwSetInputMode(current_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    void InputServer::restore_cursor() {
        glfwSetInputMode(current_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}
