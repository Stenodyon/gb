#pragma once

#include "Cart.hpp"

namespace GB {

static const usize WORK_RAM_SIZE = 0x2000;
static const usize HIGH_RAM_SIZE = 0x007f;

class Emulator;

class MemoryMapper {
    public:
        MemoryMapper(Emulator&, Cart*);
        ~MemoryMapper();

        u8 read8(u16);
        void write8(u16, u8);
        void dma_copy(u8, u8);

    private:
        u8 read8_bypass(u16);
        void write8_bypass(u16, u8);

        u8 read_io8(u8 reg);
        void write_io8(u8 reg, u8 value);

        Emulator& m_emulator;
        Cart* m_cart;
        u8* m_work_ram;
        u8* m_high_ram;
        u8 m_serial_data;
};

}
