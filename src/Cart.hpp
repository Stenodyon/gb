#pragma once

#include <string>

#include "Defs.hpp"

namespace GB {

static const usize ROM_BANK_SIZE = 16 * KB;
static const usize RAM_BANK_SIZE = 8 * KB;

struct MBCTypes {
    enum Type {
        NO_BANKING,
        MBC1,
        MBC2,
        MBC3,
        MBC5,
        MBC6,
        MBC7,
        MMM01,

        HuC1,
        HuC3,
    };
};

const char* MBCTypeName(MBCTypes::Type mbc_type);

class CartHeader {
    public:
        explicit CartHeader(const u8* data);

        std::string title() const;
        usize rom_size() const { return rom_bank_count() * ROM_BANK_SIZE; }
        usize ram_size() const;
        MBCTypes::Type mbc_type() const;

        usize rom_bank_count() const;
        usize rom_bank_mask() const;
        usize ram_bank_count() const;
        bool has_ram() const;
        bool checksum_header() const;

    private:
        static const u16 BASE_ADDRESS;

        struct RawHeader {
            u32 entry_point;
            u8 nintendo_logo[48];
            u8 title[16];
            u8 licensee_code[2];
            u8 sgb_flag;
            u8 cart_type;
            u8 rom_size;
            u8 ram_size;
            u8 destination_code;
            u8 old_licensee_code;
            u8 version_number;
            u8 header_checksum;
            u16 global_checksum;
        };

        const RawHeader* m_header;
};

class MemoryBankController {
    public:
        static MemoryBankController* create(const u8*);

        virtual ~MemoryBankController() {}

        virtual u8 read8_rom(u16) = 0;
        virtual u8 read8_ram(u16) = 0;
        virtual void write8_rom(u16, u8) = 0;
        virtual void write8_ram(u16, u8) = 0;
};

class NoBanking : public MemoryBankController {
    public:
        explicit NoBanking(const u8*);
        ~NoBanking();

        virtual u8 read8_rom(u16) override;
        virtual u8 read8_ram(u16) override;
        virtual void write8_rom(u16, u8) override;
        virtual void write8_ram(u16, u8) override;

    protected:
        const u8* m_rom;
        usize m_rom_size { 0 };
        usize m_ram_size { 0 };
        u8* m_ram { nullptr };
};

class MBC1 : public NoBanking {
    public:
        explicit MBC1(const u8*);
        ~MBC1();

        virtual u8 read8_rom(u16) override;
        virtual u8 read8_ram(u16) override;
        virtual void write8_rom(u16, u8) override;
        virtual void write8_ram(u16, u8) override;

    protected:
        inline usize rom_bank_base() { return m_rom_bank * ROM_BANK_SIZE; }
        inline usize ram_bank_base() { return m_ram_bank * RAM_BANK_SIZE; }

        enum BankingMode {
            ROM_BANKING,
            RAM_BANKING,
        };

        bool m_ram_enabled { false };
        BankingMode m_banking_mode { ROM_BANKING };
        usize m_rom_bank_mask { 0xffff };
        u8 m_rom_bank { 1 };
        u8 m_ram_bank { 0 };
};

class MBC2 : public NoBanking {
    public:
        explicit MBC2(const u8*);
        ~MBC2();

        virtual u8 read8_rom(u16) override;
        virtual u8 read8_ram(u16) override;
        virtual void write8_rom(u16, u8) override;
        virtual void write8_ram(u16, u8) override;

    private:
        static const usize RAM_SIZE = 512;
        inline usize rom_bank_base() { return m_rom_bank * ROM_BANK_SIZE; }

        bool m_ram_enabled { false };
        usize m_rom_bank_mask { 0xffff };
        usize m_rom_bank { 1 };
};

class MBC3 : public MBC1 {
    public:
        explicit MBC3(const u8*);
        ~MBC3();

        virtual u8 read8_ram(u16) override;
        virtual void write8_rom(u16, u8) override;
        virtual void write8_ram(u16, u8) override;

    private:
};

class Cart {
    public:
        explicit Cart(const u8* data);
        ~Cart();

        const u8* data() const { return m_data; }

        u8 read8_rom(u16 address) { return m_mbc->read8_rom(address); }
        void write8_rom(u16 address, u8 value) { m_mbc->write8_rom(address, value); }
        u8 read8_ram(u16 address) { return m_mbc->read8_ram(address); }
        void write8_ram(u16 address, u8 value) { m_mbc->write8_ram(address, value); }

        const CartHeader& header() const { return m_header; }

        //const u8* data() const { return m_head

    private:
        const u8* m_data;
        CartHeader m_header;
        MemoryBankController* m_mbc;
};

}
