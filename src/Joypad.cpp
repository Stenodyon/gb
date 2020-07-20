#include "Joypad.hpp"

#include "Emulator.hpp"

namespace GB {

Joypad::Joypad(Emulator& emulator)
    : m_emulator(emulator)
{
}

void Joypad::cycle()
{
    u8 prev_value = m_joypad_reg;

    m_joypad_reg &= 0x30;
    m_joypad_reg |=
        (buttons_selected() && m_A_pressed)
        || (directions_selected() && m_right_pressed) ? 0 : 0x1;
    m_joypad_reg |=
        (buttons_selected() && m_B_pressed)
        || (directions_selected() && m_left_pressed) ? 0 : 0x2;
    m_joypad_reg |=
        (buttons_selected() && m_select_pressed)
        || (directions_selected() && m_up_pressed) ? 0 : 0x4;
    m_joypad_reg |=
        (buttons_selected() && m_start_pressed)
        || (directions_selected() && m_down_pressed) ? 0 : 0x8;

    if ((prev_value & 0x0f) != (m_joypad_reg & 0x0f)) {
        printf("Joypad register change: %02x\n", m_joypad_reg);
        m_emulator.cpu().request_joypad_interrupt();
    }
}

void Joypad::set_button_status(Buttons::Button button, bool pressed)
{
    switch (button) {
        case Buttons::DOWN:
            m_down_pressed = pressed;
            break;
        case Buttons::UP:
            m_up_pressed = pressed;
            break;
        case Buttons::LEFT:
            m_left_pressed = pressed;
            break;
        case Buttons::RIGHT:
            m_right_pressed = pressed;
            break;

        case Buttons::START:
            m_start_pressed = pressed;
            break;
        case Buttons::SELECT:
            m_select_pressed = pressed;
            break;
        case Buttons::A:
            m_A_pressed = pressed;
            break;
        case Buttons::B:
            m_B_pressed = pressed;
            break;
    }
}

} // namespace GB
