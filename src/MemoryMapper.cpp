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

u8 MemoryMapper::read8(u16 address)
{
    if (m_emulator.cpu().doing_dma() && (address < 0xff80 || address > 0xfffe)) {
        exit(-1);
        return 0xff;
    }

    return read8_bypass(address);
}

void MemoryMapper::write8(u16 address, u8 value)
{
    if (m_emulator.cpu().doing_dma() && (address < 0xff80 || address > 0xfffe))
        return;

    write8_bypass(address, value);
}

void MemoryMapper::dma_copy(u8 sector, u8 offset)
{
    u16 src_address = ((u16)sector << 8) | (u16)offset;

    u8 value = read8_bypass(src_address);
    m_emulator.ppu().write8OAM((u16)offset, value);
}

u8 MemoryMapper::read8_bypass(u16 address)
{
    if (address < 0x8000)
        return m_cart->read8_rom(address);

    if (address < 0xa000)
        return m_emulator.ppu().read8(address - 0x8000);

    if (address < 0xc000)
        return m_cart->read8_ram(address - 0xa000);

    if (address < 0xe000)
        return m_work_ram[address - 0xc000];

    if (address < 0xfe00) {
        return m_work_ram[address - 0xe000];
    }

    if (address < 0xfea0)
        return m_emulator.ppu().read8OAM(address - 0xfe00);

    if (address < 0xff00) // unusable
        return 0xff;

    if (address < 0xff80) {
        return read_io8(address & 0xff);
    }

    if (address < 0xffff)
        return m_high_ram[address - 0xff80];

    // address == 0xffff (IE - Interrupt Enable)
    return read_io8(0xff);
}

void MemoryMapper::write8_bypass(u16 address, u8 value)
{
    if (address < 0x8000) {
        m_cart->write8_rom(address, value);
        return;
    }

    if (address < 0xa000) {
        m_emulator.ppu().write8(address - 0x8000, value);
        return;
    }

    if (address < 0xc000) {
        m_cart->write8_ram(address - 0xa000, value);
        return;
    }

    if (address < 0xe000) {
        m_work_ram[address - 0xc000] = value;
        return;
    }

    if (address < 0xfe00) {
        m_work_ram[address - 0xe000] = value;
        return;
    }

    if (address < 0xfea0) {
        m_emulator.ppu().write8OAM(address - 0xfe00, value);
        return;
    }

    if (address < 0xff00) { // Unusable space
        fprintf(stderr, "WARNING: write to unusable space\n");
        return;
    }

    if (address < 0xff80) {
        write_io8(address & 0xff, value);
        return;
    }

    if (address < 0xffff) {
        m_high_ram[address - 0xff80] = value;
        return;
    }

    // address == 0xffff (IE - Interrupt Enable)
    write_io8(0xff, value);
}

u8 MemoryMapper::read_io8(u8 reg)
{
    switch (reg) {
        case 0x00: // JOYP - Joypad
            return m_emulator.joypad().read_register();

        case 0x01: // SB - Serial data
            return m_serial_data;
        case 0x02: // SC - Serial control
            return 0x01;

        case 0x04: // DIV - Divider Register
            return m_emulator.timer().divider();
        case 0x05: // TIMA - Timer Counter
            return m_emulator.timer().counter();
        case 0x07: // TAC - Timer Control
            return m_emulator.timer().control();

        case 0x0f: // IF - Interrupt Flag
            return m_emulator.cpu().interrupt_flag();

        case 0x10:
            return m_emulator.apu().NR10();
        case 0x11:
            return m_emulator.apu().NR11();
        case 0x12:
            return m_emulator.apu().NR12();
        case 0x13: // = 0xff
            return m_emulator.apu().NR13();
        case 0x14:
            return m_emulator.apu().NR14();
        case 0x15: // = 0xff
            return m_emulator.apu().NR20();
        case 0x16:
            return m_emulator.apu().NR21();
        case 0x17:
            return m_emulator.apu().NR22();
        case 0x18: // = 0xff
            return m_emulator.apu().NR23();
        case 0x19:
            return m_emulator.apu().NR24();
        case 0x1a:
            return m_emulator.apu().NR30();
        case 0x1b: // = 0xff
            return m_emulator.apu().NR31();
        case 0x1c:
            return m_emulator.apu().NR32();
        case 0x1d: // = 0xff
            return m_emulator.apu().NR33();
        case 0x1e:
            return m_emulator.apu().NR34();
        case 0x1f:
            return m_emulator.apu().NR40();
        case 0x20:
            return m_emulator.apu().NR41();
        case 0x21:
            return m_emulator.apu().NR42();
        case 0x22:
            return m_emulator.apu().NR43();
        case 0x23:
            return m_emulator.apu().NR44();
        case 0x24:
            return m_emulator.apu().NR50();
        case 0x25:
            return m_emulator.apu().NR51();
        case 0x26:
            return m_emulator.apu().NR52();

        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2a:
        case 0x2b:
        case 0x2c:
        case 0x2d:
        case 0x2e:
        case 0x2f:
            fprintf(stderr, "WARNING: reading from unused io register 0x%02x\n", reg);
            return 0xff;

        case 0x30: // Wave Pattern RAM
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3a:
        case 0x3b:
        case 0x3c:
        case 0x3d:
        case 0x3e:
        case 0x3f:
            return m_emulator.apu().read_wave_pattern(reg & 0x0f);

        case 0x40: // LCD Control
            return m_emulator.ppu().control_reg();
        case 0x41: // STAT - LCDC Status
            return m_emulator.ppu().status_reg();
        case 0x42: // SCY - Scroll Y
            return m_emulator.ppu().scroll_y_reg();
        case 0x43: // SCX - Scroll X
            return m_emulator.ppu().scroll_x_reg();
        case 0x44: // LY - LCDC Y-Coordinate
            return m_emulator.ppu().line_y();
        case 0x45: // LYC - LY Compare
            return m_emulator.ppu().ly_compare();
        case 0x47: // BGP - Background Palette Data
            return m_emulator.ppu().bg_palette_reg();
        case 0x48: // OBP0 - Object Palette 0
            return m_emulator.ppu().obj_palette0_reg();
        case 0x49: // OBP1 - Object Palette 1
            return m_emulator.ppu().obj_palette1_reg();
        case 0x4a: // WY - Window Y
            return m_emulator.ppu().window_y();
        case 0x4b: // WX - Window X
            return m_emulator.ppu().window_x();

            // CGB-Only
        case 0x4f:
        case 0x4d:
        case 0x55:
        case 0x70:
            //fprintf(stderr, "WARNING: reading from CGB-only register\n");
            return 0xff;

        case 0xff: // IE - Interrupt Enable
            return m_emulator.cpu().interrupt_enable();

        default:
            fprintf(stderr, "unimplemented io register read %#04x\n", reg);
            assert(false); // TODO
    }
}

void MemoryMapper::write_io8(u8 reg, u8 value)
{
    switch (reg) {
        case 0x00: // JOYP - Joypad
            m_emulator.joypad().set_register(value);
            break;

        case 0x01: // SB - Serial data
            m_serial_data = value;
            break;
        case 0x02: // SC - Serial control
            if (value & 0x80)
                printf("%c", m_serial_data);
            break;

        case 0x04:
            m_emulator.timer().set_divider(value);
            break;
        case 0x05: // TIMA - Timer Counter
            m_emulator.timer().set_counter(value);
            break;
        case 0x06: // TMA - Timer Modulo
            m_emulator.timer().set_modulo(value);
            break;
        case 0x07: // TAC - Timer Control
            m_emulator.timer().set_control(value);
            break;

        case 0x0f: // IF - Interrupt Flag
            m_emulator.cpu().set_interrupt_flag(value);
            break;

        case 0x10:
            m_emulator.apu().set_NR10(value);
            break;
        case 0x11:
            m_emulator.apu().set_NR11(value);
            break;
        case 0x12:
            m_emulator.apu().set_NR12(value);
            break;
        case 0x13:
            m_emulator.apu().set_NR13(value);
            break;
        case 0x14:
            m_emulator.apu().set_NR14(value);
            break;
        case 0x15:
            fprintf(stderr, "WARNING: writing to unused io register 0x%02x\n", reg);
            break;
        case 0x16:
            m_emulator.apu().set_NR21(value);
            break;
        case 0x17:
            m_emulator.apu().set_NR22(value);
            break;
        case 0x18:
            m_emulator.apu().set_NR23(value);
            break;
        case 0x19:
            m_emulator.apu().set_NR24(value);
            break;
        case 0x1a:
            m_emulator.apu().set_NR30(value);
            break;
        case 0x1b:
            m_emulator.apu().set_NR31(value);
            break;
        case 0x1c:
            m_emulator.apu().set_NR32(value);
            break;
        case 0x1d:
            m_emulator.apu().set_NR33(value);
            break;
        case 0x1e:
            m_emulator.apu().set_NR34(value);
            break;
        case 0x1f:
            fprintf(stderr, "WARNING: writing to unused io register 0x%02x\n", reg);
            break;
        case 0x20:
            m_emulator.apu().set_NR41(value);
            break;
        case 0x21:
            m_emulator.apu().set_NR42(value);
            break;
        case 0x22:
            m_emulator.apu().set_NR43(value);
            break;
        case 0x23:
            m_emulator.apu().set_NR44(value);
            break;
        case 0x24:
            m_emulator.apu().set_NR50(value);
            break;
        case 0x25:
            m_emulator.apu().set_NR51(value);
            break;
        case 0x26:
            m_emulator.apu().set_NR52(value);
            break;

        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2a:
        case 0x2b:
        case 0x2c:
        case 0x2d:
        case 0x2e:
        case 0x2f:
            fprintf(stderr, "WARNING: writing to unused io register 0x%02x\n", reg);
            break;

        case 0x30: // Wave Pattern RAM
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3a:
        case 0x3b:
        case 0x3c:
        case 0x3d:
        case 0x3e:
        case 0x3f:
            m_emulator.apu().set_wave_pattern(value, reg & 0x0f);
            break;

        case 0x40: // LCD Control
            m_emulator.ppu().set_control(value);
            break;
        case 0x41: // STAT - LCDC Status
            m_emulator.ppu().set_status(value);
            break;
        case 0x42: // SCY - Scroll Y
            m_emulator.ppu().set_scroll_y(value);
            break;
        case 0x43: // SCX - Scroll X
            m_emulator.ppu().set_scroll_x(value);
            break;
        case 0x44: // LY - LCDC Y-Coordinate
            fprintf(stderr, "WARNING: Writing to read-only io register FF44\n");
            break;
        case 0x45: // LYC - LY Compare
            m_emulator.ppu().set_ly_compare(value);
            break;
        case 0x46: // DMA
            m_emulator.cpu().start_dma(value);
            break;
        case 0x47: // BGP - Background Palette Data
            m_emulator.ppu().set_bg_palette(value);
            break;
        case 0x48: // OBP0 - Object Palette 0
            m_emulator.ppu().set_object_palette0(value);
            break;
        case 0x49: // OBP1 - Object Palette 1
            m_emulator.ppu().set_object_palette1(value);
            break;
        case 0x4a: // WY - Window Y
            m_emulator.ppu().set_window_y(value);
            break;
        case 0x4b: // WX - Window X
            m_emulator.ppu().set_window_x(value);
            break;

            // CGB-Only
        case 0x4d:
        case 0x4f:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x68:
        case 0x69:
        case 0x70:
            //fprintf(stderr, "WARNING: writing to CGB-only register\n");
            break;

        case 0x4e:
        case 0x50:
        case 0x7f:
            fprintf(
                    stderr,
                    "WARNING: writing to undefined register %#04x\n",
                    reg
                   );
            break;

        case 0xff: // IE - Interrupt Enable
            m_emulator.cpu().set_interrupt_enable(value);
            break;

        default:
            fprintf(stderr, "unimplemented io register write %#04x\n", reg);
            assert(false); // TODO
    }
}

} // namespace GB
