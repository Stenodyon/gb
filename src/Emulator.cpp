#include "Emulator.hpp"

#include <SDL2/SDL.h>

//#define TRACE

namespace GB {

Emulator::Emulator(Cart* cart, SDL_Texture* screen)
    : m_mmu(*this, cart)
    , m_cpu(*this)
    , m_ppu(*this, screen)
{
    m_cpu.set_PC(0x100);
    m_cpu.set_SP(0xfffe);
}

void Emulator::exec_to_next_frame()
{
    m_frame_end = false;
    while (!m_frame_end) {
#ifdef TRACE
        auto saved_PC = m_cpu.PC();
#endif
        auto ins = Instruction::from_stream(&m_cpu);
#ifdef TRACE
        if (ins.has_sub_op()) {
            printf(
                    "\033[0;36m%#06x\033[0m: \033[0;33m0xcb %#04x\033[0m %s\n",
                    saved_PC,
                    ins.sub_op(),
                    ins.to_string().c_str()
                  );
        } else {
            printf(
                    "\033[0;36m%#06x\033[0m: \033[0;33m%#04x\033[0m      %s\n",
                    saved_PC,
                    ins.opcode(),
                    ins.to_string().c_str()
                  );
        }
#endif
        (m_cpu.*ins.handler())(ins);
    }
}

}
