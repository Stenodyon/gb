#pragma once

#include "Defs.hpp"

namespace GB {

class Emulator;

class Joypad {
    public:
        explicit Joypad(Emulator&);

        void cycle();

        inline bool buttons_selected() const
        {
            return (m_joypad_reg & 0x20) == 0;
        }
        inline bool directions_selected() const
        {
            return (m_joypad_reg & 0x10) == 0;
        }

        inline void set_register(u8 value)
        {
            m_joypad_reg &= ~0x30;
            m_joypad_reg |= value & 0x30;
        }
        inline u8 read_register() const { return m_joypad_reg; }

        struct Buttons {
            enum Button {
                DOWN,
                UP,
                LEFT,
                RIGHT,
                START,
                SELECT,
                A,
                B,
            };
        };

        void set_button_status(Buttons::Button, bool pressed);

    private:
        Emulator& m_emulator;

        bool m_down_pressed { false };
        bool m_up_pressed { false };
        bool m_left_pressed { false };
        bool m_right_pressed { false };
        bool m_start_pressed { false };
        bool m_select_pressed { false };
        bool m_A_pressed { false };
        bool m_B_pressed { false };

        u8 m_joypad_reg { 0x0f };
};

} // namespace GB
