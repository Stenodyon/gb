#include <cassert>

#include "Cart.hpp"

namespace GB {

const u32 CartHeader::BASE_ADDRESS = 0x100;

const char* MBCTypeName(MBCTypes::Type mbc_type)
{
    switch (mbc_type) {
        case MBCTypes::NO_BANKING:
            return "NO_BANKING";
        case MBCTypes::MBC1:
            return "MBC1";
        case MBCTypes::MBC2:
            return "MBC2";
        case MBCTypes::MBC3:
            return "MBC3";
        case MBCTypes::MBC5:
            return "MBC5";
        case MBCTypes::MBC6:
            return "MBC6";
        case MBCTypes::MBC7:
            return "MBC7";
        case MBCTypes::MMM01:
            return "MMM01";
        case MBCTypes::HuC1:
            return "HuC1";
        case MBCTypes::HuC3:
            return "HuC3";

        default:
            fprintf(stderr, "reached unreachable!\n");
            exit(-1);
    }
}

CartHeader::CartHeader(const u8* data)
    : m_header(reinterpret_cast<const RawHeader*>(data + BASE_ADDRESS))
{
}

std::string CartHeader::title() const
{
    return std::string((const char*)m_header->title, 16);
}

usize CartHeader::rom_bank_count() const
{
    switch (m_header->rom_size) {
        case 0x00:
            return 2;
        case 0x01:
            return 4;
        case 0x02:
            return 8;
        case 0x03:
            return 16;
        case 0x04:
            return 32;
        case 0x05:
            return 64;
        case 0x06:
            return 128;
        case 0x07:
            return 256;
        case 0x08:
            return 512;

        case 0x52:
            return 72;
        case 0x53:
            return 80;
        case 0x54:
            return 96;

        default:
            fprintf(stderr, "unknown rom size: %#02x\n", m_header->rom_size);
            exit(-1);
    }
}

usize CartHeader::ram_bank_count() const
{
    switch (m_header->ram_size) {
        case 0x00:
            return 0;
        case 0x01:
        case 0x02:
            return 1;
        case 0x03:
            return 4;
        case 0x04:
            return 16;
        case 0x05:
            return 8;

        default:
            fprintf(stderr, "unknown ram size: %#02x\n", m_header->ram_size);
            exit(-1);
    }
}

usize CartHeader::ram_size() const
{
    // special case: 1 bank but not the full 8 KB
    if (m_header->ram_size == 0x01)
        return 2 * KB;

    return ram_bank_count() * RAM_BANK_SIZE;
}

MBCTypes::Type CartHeader::mbc_type() const
{
    switch (m_header->cart_type) {
        case 0x00:
        case 0x08:
        case 0x09:
            return MBCTypes::NO_BANKING;
        case 0x01:
        case 0x02:
        case 0x03:
            return MBCTypes::MBC1;
        case 0x05:
        case 0x06:
            return MBCTypes::MBC2;
        case 0x0b:
        case 0x0c:
        case 0x0d:
            return MBCTypes::MMM01;
        case 0x0f:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            return MBCTypes::MBC3;
        case 0x19:
        case 0x1a:
        case 0x1b:
        case 0x1c:
        case 0x1d:
        case 0x1e:
            return MBCTypes::MBC5;
        case 0x20:
            return MBCTypes::MBC6;
        case 0x22:
            return MBCTypes::MBC7;
        case 0xfe:
            return MBCTypes::HuC3;
        case 0xff:
            return MBCTypes::HuC1;

        default:
            fprintf(
                    stderr,
                    "unknown memory bank controller: %#02x\n",
                    m_header->cart_type
                   );
            exit(-1);
    }
}

bool CartHeader::has_ram() const
{
    switch (m_header->cart_type) {
        case 0x00:
        case 0x01:
        case 0x05:
        case 0x06:
        case 0x0b:
        case 0x0f:
        case 0x11:
        case 0x19:
        case 0x1c:
        case 0x20:
        case 0xfe:
            return false;

        case 0x02:
        case 0x03:
        case 0x08:
        case 0x09:
        case 0x0c:
        case 0x0d:
        case 0x10:
        case 0x12:
        case 0x13:
        case 0x1a:
        case 0x1b:
        case 0x1d:
        case 0x1e:
        case 0x22:
        case 0xff:
            return true;

        default:
            fprintf(
                    stderr,
                    "unknown memory bank controller: %#02x\n",
                    m_header->cart_type
                   );
            exit(-1);
    }
}

bool CartHeader::checksum_header() const
{
    u8 checksum = 0;
    const u8* data = reinterpret_cast<const u8*>(m_header);

    for (usize index = 0x34; index <= 0x4c; ++index) {
        checksum -= data[index] + 1;
    }

    return checksum == m_header->header_checksum;
}

Cart::Cart(const u8* data)
    : m_data(data)
    , m_header(data)
{
    m_mbc = MemoryBankController::create(data);

    printf("Loaded cartridge [%s]\n", m_header.title().c_str());

    if (m_header.checksum_header()) {
        printf("Checksum valid! :)\n");
    } else {
        printf("Checksum invalid! :(\n");
    }
}

Cart::~Cart()
{
    delete m_mbc;
}

MemoryBankController* MemoryBankController::create(const u8* data) {
    auto mbc_type = CartHeader(data).mbc_type();

    switch (mbc_type) {
        case MBCTypes::NO_BANKING:
            return new NoBanking(data);
        case MBCTypes::MBC1:
            return new MBC1(data);

        default:
            fprintf(
                    stderr,
                    "unimplemented rom bank controller %s\n",
                    MBCTypeName(mbc_type)
                   );
            exit(-1);
    }
}

NoBanking::NoBanking(const u8* data)
    : m_rom(data)
{
    CartHeader header(data);

    if (header.has_ram()) {
        m_ram_size = header.ram_size();
        assert(m_ram_size > 0);
        m_ram = (u8*)malloc(m_ram_size);
    }
}

NoBanking::~NoBanking()
{
    if (m_ram)
        free(m_ram);
}

u8 NoBanking::read8(u32 address) {
    if (address < 0x8000)
        return read8_rom(address);

    if (address < 0xa000)
        assert(false);

    if (address < 0xc000) {
        auto offset_address = address - 0xa000;
        if (offset_address >= m_ram_size)
            return 0xff;
        return read8_ram(offset_address);
    }

    assert(false);
}

void NoBanking::write8(u32 address, u8 value) {
    if (address < 0x8000) {
        write8_rom(address, value);
        return;
    }

    if (address < 0xa000)
        assert(false);

    if (address < 0xc000) {
        auto offset_address = address - 0xa000;
        if (offset_address >= m_ram_size)
            return;
        write8_ram(offset_address, value);
    }

    assert(false);
}

u8 NoBanking::read8_rom(u32 address)
{
    return m_rom[address];
}

u8 NoBanking::read8_ram(u32 offset)
{
    return m_ram[offset];
}

void NoBanking::write8_rom(u32 address, u8 value)
{
    fprintf(
            stderr,
            "tried writing %#02x at no-banking ROM address %#04x\n",
            value,
            address
           );
}

void NoBanking::write8_ram(u32 offset, u8 value)
{
    m_ram[offset] = value;
}

MBC1::MBC1(const u8* data)
    : NoBanking(data)
{
}

MBC1::~MBC1()
{
}

u8 MBC1::read8_rom(u32 address)
{
    if (address < 0x4000)
        return m_rom[address];
    return m_rom[rom_bank_base() + address - 0x4000];
}

u8 MBC1::read8_ram(u32 offset)
{
    if (!m_ram_enabled)
        return 0xff;
    return m_ram[ram_bank_base() + offset];
}

void MBC1::write8_rom(u32 address, u8 value)
{
    if (address < 0x2000) { // RAM Enable
        m_ram_enabled = (value & 0x0a) == 0xa;
        return;
    }

    if (address < 0x4000) { // ROM Bank lower 5 bits
        u8 bank = value & 0x1f;
        if (bank == 0)
            bank = 1;
        m_rom_bank = (m_rom_bank & 0xe0) | bank;
        return;
    }

    if (address < 0x6000) { // RAM/ROM Bank
        value &= 0x03;
        switch (m_banking_mode) {
            case ROM_BANKING:
                m_rom_bank = (m_rom_bank & 0x1f) | (value << 5);
                break;
            case RAM_BANKING:
                m_ram_bank = value;
                break;

            default:
                assert(false); // unreachable
        }
        return;
    }

    if (address < 0x8000) { // Banking Mode
        m_banking_mode = (value & 0x01) ? RAM_BANKING : ROM_BANKING;
        return;
    }

    assert(false); // unreachable
}

void MBC1::write8_ram(u32 offset, u8 value)
{
    if (!m_ram_enabled)
        return;
    m_ram[ram_bank_base() + offset] = value;
}

} // namespace GB
