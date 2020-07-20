#include "Emulator.hpp"

#include <SDL2/SDL.h>

namespace GB {

Emulator::Emulator(Cart* cart, SDL_Texture* screen)
    : m_mmu(*this, cart)
    , m_cpu(*this)
    , m_ppu(*this, screen)
    , m_joypad(*this)
    , m_timer(*this)
{
    m_cpu.setPC(0x100);
    m_cpu.setSP(0xfffe);

    m_mmu.write8(0xff05, 0x00);
    m_mmu.write8(0xff06, 0x00);
    m_mmu.write8(0xff07, 0x00);
    m_mmu.write8(0xff10, 0x80);
    m_mmu.write8(0xff11, 0xbf);
    m_mmu.write8(0xff12, 0xf3);
    m_mmu.write8(0xff14, 0xbf);
    m_mmu.write8(0xff16, 0x3f);
    m_mmu.write8(0xff17, 0x00);
    m_mmu.write8(0xff19, 0xbf);
    m_mmu.write8(0xff1a, 0x7f);
    m_mmu.write8(0xff1b, 0xff);
    m_mmu.write8(0xff1c, 0x9f);
    m_mmu.write8(0xff1e, 0xbf);
    m_mmu.write8(0xff20, 0xff);
    m_mmu.write8(0xff21, 0x00);
    m_mmu.write8(0xff22, 0x00);
    m_mmu.write8(0xff23, 0xbf);
    m_mmu.write8(0xff24, 0x77);
    m_mmu.write8(0xff25, 0xf3);
    m_mmu.write8(0xff26, 0xf1);
    m_mmu.write8(0xff40, 0x91);
    m_mmu.write8(0xff42, 0x00);
    m_mmu.write8(0xff43, 0x00);
    m_mmu.write8(0xff45, 0x00);
    m_mmu.write8(0xff47, 0xfc);
    m_mmu.write8(0xff48, 0xff);
    m_mmu.write8(0xff49, 0xff);
    m_mmu.write8(0xff4a, 0x00);
    m_mmu.write8(0xff4b, 0x00);
    m_mmu.write8(0xffff, 0x00);
}

void Emulator::step()
{
    m_cpu.cycle();
    m_cpu.dump_registers();
}

void Emulator::exec_to_next_frame()
{
    m_frame_end = false;
    while (!m_frame_end) {
        m_cpu.cycle();
    }
}

}
