#include "Timer.hpp"

#include "Emulator.hpp"

namespace GB {

Timer::Timer(Emulator& emulator)
    : m_emulator(emulator)
{
}

void Timer::cycle()
{
    bool timer_inc_bit = (m_divider & clock_select_mask()) != 0;
    ++m_divider;
    bool new_timer_inc_bit = (m_divider & clock_select_mask()) != 0;
    bool timer_inc_bit_changed = timer_inc_bit && !new_timer_inc_bit;

    if (timer_enabled() && timer_inc_bit_changed) {
        if (m_timer_counter == 0xff) {
            m_timer_counter = m_timer_modulo;
            m_emulator.cpu().request_timer_interrupt();
        } else {
            ++m_timer_counter;
        }
    }
}

u16 Timer::clock_select_mask()
{
    switch (clock_select()) {
        case 0:
            return 1 << 9;
        case 1:
            return 1 << 3;
        case 2:
            return 1 << 5;
        case 3:
            return 1 << 7;

        default:
            assert(false); // unreachable
    }
}

} // namespace GB
