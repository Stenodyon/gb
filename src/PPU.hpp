#pragma once

#include "Defs.hpp"

#include <SDL2/SDL.h>

namespace GB {

static const usize VRAM_SIZE = 8 * KB;

class Emulator;

class PPU {
    public:
        explicit PPU(Emulator&, SDL_Texture*);
        ~PPU();

        u8 read8(u32);
        void write8(u32, u8);

        void cycle();

        struct ModeFlag {
            enum Flag {
                HBLANK = 0,
                VBLANK = 1,
                OAM = 2,
                TRANSFER = 3,
            };
        };

        inline u8 control_reg() const { return m_control_reg; }
        inline u8 status_reg() const
        {
            static const u8 read_mask = 0x7f;
            u8 value = m_status_reg
                | (u8)m_mode;
            return value & read_mask;
        }
        inline void set_control(u8 value) { m_control_reg = value; }
        inline void set_status(u8 value)
        {
            static const u8 write_mask = 0x7c;
            m_status_reg = value & write_mask;
        }

        inline bool display_enabled() { return m_control_reg & 0x80; }
        inline bool window_display_enabled() { return m_control_reg & 0x20; }
        inline bool sprite_display_enabled() { return m_control_reg & 0x02; }
        inline u16 window_tilemap_base()
        {
            u16 address = (m_control_reg & 0x40) ? 0x9c00 : 0x9800;
            return address - 0x8000;
        }
        inline u16 bg_tilemap_base()
        {
            u16 address = (m_control_reg & 0x08) ? 0x9c00 : 0x9800;
            return address - 0x8000;
        }
        inline u16 tile_data_base()
        {
            u16 address = (m_control_reg & 0x10) ? 0x8000 : 0x8000;
            return address - 0x8000;
        }
        inline usize sprite_height()
        {
            return (m_control_reg & 0x04) ? 16 : 8;
        }

        struct TileData {
            u8 data[16];

            u8 color_at(usize x, usize y);
        };

        inline TileData* tile_data(u8 tile_index)
        {
            auto* raw_ptr = m_vram + tile_data_base() + tile_index * 16;
            return reinterpret_cast<TileData*>(raw_ptr);
        }

    private:
        Emulator& m_emulator;
        SDL_Texture* m_screen { nullptr };
        u8* m_vram { nullptr };
        ModeFlag::Flag m_mode { ModeFlag::OAM };
        usize m_dot_count { 0 };
        usize m_pixel_x { 0 };
        usize m_pixel_y { 0 };
        u8* m_pixels { nullptr };

        u8 m_control_reg { 0x91 };
        u8 m_status_reg { 0 };

        void render_pixel();
        void set_pixel(usize, usize, u8);
        void copy_pixels();
};

} // namespace GB
