#pragma once

#include <cassert>

#include "Defs.hpp"

namespace GB {

class Emulator;

class Timer {
    public:
        explicit Timer(Emulator&);

        void cycle();

        inline u8 control() const { return m_timer_control; }
        inline void set_control(u8 value) { m_timer_control = value & 0x7; }
        inline u8 divider() const { return m_divider >> 8; }
        inline void set_divider(u8) { set_divider_internal(0); }
        inline u8 counter() const { return m_timer_counter; }
        inline void set_counter(u8 value) { m_timer_counter = value; }
        inline u8 modulo() const { return m_timer_modulo; }
        inline void set_modulo(u8 value) { m_timer_modulo = value; }

        bool timer_enabled() const { return (m_timer_control & 0x4) != 0; }
        u16 clock_select_mask() const;
        inline u8 clock_select() const { return m_timer_control & 0x03; }
        inline bool timer_trigger_bit() const
        {
            return (m_divider & clock_select_mask()) != 0;
        }

    private:
        Emulator& m_emulator;

        u16 m_divider { 0 };
        u8 m_timer_counter { 0 };
        u8 m_timer_modulo { 0 };
        u8 m_timer_control { 0 };

        void set_divider_internal(u16);
};

} // namespace GB
