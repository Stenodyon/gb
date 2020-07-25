#pragma once

#include "Defs.hpp"
#include "LR35902.hpp"
#include "Cart.hpp"
#include "MemoryMapper.hpp"
#include "PPU.hpp"
#include "Joypad.hpp"
#include "Timer.hpp"
#include "APU.hpp"

#include <SDL2/SDL.h>

namespace GB {

class Emulator {
    public:
        Emulator(Cart*, SDL_Texture*);

        void step();
        void exec_to_next_frame();

        inline LR35902& cpu() { return m_cpu; }
        inline MemoryMapper& mmu() { return m_mmu; }
        inline PPU& ppu() { return m_ppu; }
        inline APU& apu() { return m_apu; }
        inline Joypad& joypad() { return m_joypad; }
        inline Timer& timer() { return m_timer; }

        inline bool trace() const { return m_trace; }
        inline void enable_tracing(bool value) { m_trace = value; }

        void notify_frame_end() { m_frame_end = true; }

    private:
        MemoryMapper m_mmu;
        LR35902 m_cpu;
        PPU m_ppu;
        APU m_apu;
        Joypad m_joypad;
        Timer m_timer;

        bool m_frame_end { false };
        bool m_trace { false };
};

}
