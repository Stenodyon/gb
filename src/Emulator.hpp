#pragma once

#include "Defs.hpp"
#include "LR35902.hpp"
#include "Cart.hpp"
#include "MemoryMapper.hpp"
#include "PPU.hpp"

#include <SDL2/SDL.h>

namespace GB {

class Emulator {
    public:
        Emulator(Cart*, SDL_Texture*);

        void exec_to_next_frame();
        MemoryMapper& mmu() { return m_mmu; }
        PPU& ppu() { return m_ppu; }

        void notify_frame_end() { m_frame_end = true; }

    private:
        MemoryMapper m_mmu;
        LR35902 m_cpu;
        PPU m_ppu;

        bool m_frame_end { false };
};

}
