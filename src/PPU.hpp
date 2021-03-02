#pragma once

#include <cassert>

#include <SDL2/SDL.h>

#include "Defs.hpp"

namespace GB {

static const usize VRAM_SIZE = 8 * KB;
static const usize OAM_SIZE = 0xa0;

class Emulator;

class PPU {
    public:
        explicit PPU(Emulator&, SDL_Texture*);
        ~PPU();

        u8 read8(u32);
        void write8(u32, u8);
        u8 read8OAM(u16);
        void write8OAM(u16, u8);

        void cycle();

        struct ModeFlag {
            enum Flag {
                HBLANK = 0,
                VBLANK = 1,
                OAM = 2,
                TRANSFER = 3,
            };
        };

        inline u8 line_y() const { return m_pixel_y; }
        void set_line_y(u8 value);
        inline u8 control_reg() const { return m_control_reg; }
        inline u8 status_reg() const
        {
            static const u8 read_mask = 0x7f;
            u8 value = m_status_reg
                | (u8)m_mode
                | (m_pixel_y == m_ly_compare ? 0x04 : 0);
            return value & read_mask;
        }
        inline void set_control(u8 value) { m_control_reg = value; }
        inline void set_status(u8 value)
        {
            static const u8 write_mask = 0x7c;
            m_status_reg = value & write_mask;
        }
        inline void set_scroll_x(u8 value) { m_scroll_x = value; }
        inline u8 scroll_x_reg() const { return m_scroll_x; }
        inline void set_scroll_y(u8 value) { m_scroll_y = value; }
        inline u8 scroll_y_reg() const { return m_scroll_y; }
        inline u8 window_x() const { return m_window_x; }
        inline u8 window_y() const { return m_window_y; }
        inline void set_window_x(u8 value) { m_window_x = value; }
        inline void set_window_y(u8 value) { m_window_y = value; }
        inline u8 ly_compare() const { return m_ly_compare; }
        inline void set_ly_compare(u8 value) { m_ly_compare = value; }

        inline u8 bg_palette_reg() const { return m_bg_palette; }
        inline void set_bg_palette(u8 value) { m_bg_palette = value; }
        inline u8 obj_palette0_reg() const { return m_object_palette0; }
        inline void set_object_palette0(u8 value) { m_object_palette0 = value; }
        inline u8 obj_palette1_reg() const { return m_object_palette1; }
        inline void set_object_palette1(u8 value) { m_object_palette1 = value; }

        inline bool display_enabled() const { return m_control_reg & 0x80; }
        inline bool window_display_enabled() const { return m_control_reg & 0x20; }
        inline bool sprite_display_enabled() const { return m_control_reg & 0x02; }
        inline bool bg_display_enabled() const { return m_control_reg & 0x01; }
        inline u16 window_tilemap_base() const
        {
            u16 address = (m_control_reg & 0x40) ? 0x9c00 : 0x9800;
            return address - 0x8000;
        }
        inline u16 bg_tilemap_base() const
        {
            u16 address = (m_control_reg & 0x08) ? 0x9c00 : 0x9800;
            return address - 0x8000;
        }
        inline bool tiled_data_signed_addressing() const
        {
            return (m_control_reg & 0x10) == 0;
        }
        inline u16 tile_data_base() const
        {
            u16 address = tiled_data_signed_addressing() ? 0x8800 : 0x8000;
            return address - 0x8000;
        }
        inline bool sprite_double_height() const
        {
            return (m_control_reg & 0x04) != 0;
        }
        inline usize sprite_height() const
        {
            return (m_control_reg & 0x04) ? 16 : 8;
        }
        inline bool coincidence_interrupt_enabled() const
        {
            return (m_status_reg & 0x40) != 0;
        }
        inline bool hblank_interrupt_enabled() const
        {
            return (m_status_reg & 0x08) != 0;
        }
        inline bool vblank_interrupt_enabled() const
        {
            return (m_status_reg & 0x10) != 0;
        }
        inline bool oam_interrupt_enabled() const
        {
            return (m_status_reg & 0x20) != 0;
        }

        struct TileData {
            u8 data[16];

            u8 color_at(usize x, usize y);
        };

        inline TileData* bg_tile_data(u8 tile_index)
        {
            auto* raw_ptr = m_vram + tile_data_base() + tile_index * 16;
            return reinterpret_cast<TileData*>(raw_ptr);
        }
        inline TileData* obj_tile_data(u8 tile_index)
        {
            auto* raw_ptr = m_vram + tile_index * 16;
            return reinterpret_cast<TileData*>(raw_ptr);
        }

        struct Palette {
            u8 data;

            inline u8 color_for(u8 color_index) const
            {
                assert(color_index < 4);

                u8 bit_offset = color_index * 2;
                return (data & (0b11 << bit_offset)) >> bit_offset;
            }
        };

        inline Palette* bg_palette()
        {
            return reinterpret_cast<Palette*>(&m_bg_palette);
        }
        inline Palette* obj_palette(u8 number)
        {
            auto* palette = (number == 0) ? &m_object_palette0 : &m_object_palette1;
            return reinterpret_cast<Palette*>(palette);
        }

        struct Sprite {
            u8 y_pos;
            u8 x_pos;
            u8 tile_index;
            u8 attributes;

            i32 x() const { return (i32)x_pos - 8; }
            i32 y() const { return (i32)y_pos - 16; }
            u8 palette_number() const { return (attributes & 0x10) >> 2; }
            bool x_flip() const { return (attributes & 0x20) != 0; }
            bool y_flip() const { return (attributes & 0x40) != 0; }
            bool behind_bg() const { return (attributes & 0x80) != 0; }
        };

        static_assert(sizeof(Sprite) == 4, "PPU::Sprite is not 4 bytes wide");

    private:
        Emulator& m_emulator;
        SDL_Texture* m_screen { nullptr };
        u8* m_vram { nullptr };
        u8* m_oam { nullptr };
        ModeFlag::Flag m_mode { ModeFlag::OAM };
        usize m_dot_count { 0 };
        usize m_pixel_x { 0 };
        usize m_pixel_y { 0 };
        usize m_window_line { 0 };
        bool m_was_window { false };
        u8 m_scroll_x { 0 };
        u8 m_scroll_y { 0 };
        u8 m_window_x { 0 };
        u8 m_window_y { 0 };
        u8* m_pixels { nullptr };
        usize m_scanline_sprite_count { 0 };
        Sprite* m_scanline_sprites[10];

        u8 m_bg_palette { 0 };
        u8 m_object_palette0 { 0 };
        u8 m_object_palette1 { 0 };

        u8 m_control_reg { 0x91 };
        u8 m_status_reg { 0 };
        u8 m_ly_compare { 0 };

        u8 background_color_at(u8, u8);
        u8 window_color_at(u8, u8);
        bool inside_window(u8, u8);
        void render_pixel();
        void set_pixel(usize, usize, u8);
        void copy_pixels();
        void gather_sprites();
};

} // namespace GB
