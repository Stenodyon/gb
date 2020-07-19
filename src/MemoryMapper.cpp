#include <cassert>

#include "MemoryMapper.hpp"
#include "Emulator.hpp"

namespace GB {

MemoryMapper::MemoryMapper(Emulator& emulator, Cart* cart)
    : m_emulator(emulator)
    , m_cart(cart)
{
    m_work_ram = (u8*)malloc(WORK_RAM_SIZE);
    m_high_ram = (u8*)malloc(HIGH_RAM_SIZE);
}

MemoryMapper::~MemoryMapper()
{
    free(m_high_ram);
    free(m_work_ram);
}

u8 MemoryMapper::read8(u32 address)
{
    if (address < 0x8000)
        return m_cart->read8(address);

    if (address < 0xa000)
        return m_emulator.ppu().read8(address - 0x8000);

    if (address < 0xc000)
        return m_cart->read8(address - 0xa000);

    if (address < 0xe000)
        return m_work_ram[address - 0xc000];

    if (address < 0xfeff) {
        return m_work_ram[address - 0xe000];
    }

    if (address < 0xff00) {
        fprintf(stderr, "unimplemented memory read at %#06x\n", address);
        assert(false); // TODO
    }

    if (address < 0xff80)
        return read_io8(address & 0xff);

    if (address < 0xffff)
        return m_high_ram[address - 0xff80];

    // address == 0xffff (IE - Interrupt Enable)
    return read_io8(0xff);
}

void MemoryMapper::write8(u32 address, u8 value)
{
    if (address < 0x8000) {
        m_cart->write8(address, value);
        return;
    }

    if (address < 0xa000) {
        m_emulator.ppu().write8(address - 0x8000, value);
        return;
    }

    if (address < 0xc000) {
        m_cart->write8(address, value);
        return;
    }

    if (address < 0xe000) {
        m_work_ram[address - 0xc000] = value;
        return;
    }

    if (address < 0xff00) {
        fprintf(stderr, "unimplemented memory write at %#06x\n", address);
        assert(false); // TODO
    }

    if (address < 0xff80) {
        write_io8(address & 0xff, value);
        return;
    }

    if (address < 0xfff) {
        m_high_ram[address - 0xff80] = value;
        return;
    }

    // address == 0xffff (IE - Interrupt Enable)
    write_io8(0xff, value);
}

u8 MemoryMapper::read_io8(u8 reg)
{
    switch (reg) {
        case 0x44: // LY - LCDC Y-Coordinate
            fprintf(stderr, "FIXME: implement LCDC Y-Coordinate read\n");
            return 0x00;

        default:
            fprintf(stderr, "unimplemented io register read %#04x\n", reg);
            assert(false); // TODO
    }
}

void MemoryMapper::write_io8(u8 reg, u8 value)
{
    switch (reg) {
        case 0x01: // SB - Serial data
            m_serial_data = value;
            break;
        case 0x02: // SC - Serial control
            if (value & 0x80)
                printf("%c", m_serial_data);
            break;

        case 0x07: // TAC - Timer Control
            fprintf(stderr, "FIXME: implement Timer Control write\n");
            break;

        case 0x0f: // IF - Interrupt Flag
            fprintf(stderr, "FIXME: implement Interrupt Flag write\n");
            break;

        case 0x24:
            fprintf(stderr, "FIXME: implement NR50 write\n");
            break;
        case 0x25:
            fprintf(stderr, "FIXME: implement NR51 write\n");
            break;
        case 0x26: // NR52
            fprintf(stderr, "FIXME: implement NR52 write\n");
            break;

        case 0x40: // LCD Control
            fprintf(stderr, "FIXME: implement LCD Control write\n");
            break;
        case 0x41: // STAT - LCDC Status
            fprintf(stderr, "FIXME: implement LCDC Status write\n");
            break;
        case 0x42: // SCY - Scroll Y
            fprintf(stderr, "FIXME: implement Scroll Y write\n");
            break;
        case 0x43: // SCX - Scroll X
            fprintf(stderr, "FIXME: implement Scroll X write\n");
            break;
        case 0x47: // BGP - Background Palette Data
            fprintf(stderr, "FIXME: implement Background Palette Data write\n");
            break;

        case 0x4f: // CGB-Only
            fprintf(stderr, "WARNING: writing to CGB-only register\n");
            break;
        case 0x68: // CGB-Only
            fprintf(stderr, "WARNING: writing to CGB-only register\n");
            break;
        case 0x69: // CGB-Only
            fprintf(stderr, "WARNING: writing to CGB-only register\n");
            break;

        case 0xff: // IE - Interrupt Enable
            fprintf(stderr, "FIXME: implement Interrupt Enable write\n");
            break;
        default:
            fprintf(stderr, "unimplemented io register write %#04x\n", reg);
            assert(false); // TODO
    }
}

} // namespace GB
