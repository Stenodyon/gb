#include "PPU.hpp"
#include "Emulator.hpp"

#include <cassert>
#include <cstdlib>
#include <cstdio>

namespace GB {

const static u8 COLORS[4][3] = {
    { 0xc4, 0xf0, 0xc2 },
    { 0x5a, 0xb9, 0xa8 },
    { 0x1e, 0x60, 0x6e },
    { 0x2d, 0x1b, 0x00 },
};

const static usize PIXEL_BUFFER_SIZE = 160 * 144 * 3;

PPU::PPU(Emulator& emulator, SDL_Texture* screen)
    : m_emulator(emulator)
    , m_screen(screen)
{
    m_vram = (u8*)malloc(VRAM_SIZE);
    m_oam = (u8*)malloc(OAM_SIZE);
    m_pixels = (u8*)malloc(PIXEL_BUFFER_SIZE);

    //assert(m_screen);
    assert(m_vram);
    assert(m_pixels);
}

PPU::~PPU()
{
    free(m_pixels);
    free(m_oam);
    free(m_vram);
}

u8 PPU::read8(u32 offset)
{
    assert(offset < VRAM_SIZE);

    return m_vram[offset];
}

void PPU::write8(u32 offset, u8 value)
{
    assert(offset < VRAM_SIZE);

    m_vram[offset] = value;
}

u8 PPU::read8OAM(u16 offset)
{
    assert(offset < OAM_SIZE);

    return m_oam[offset];
}

void PPU::write8OAM(u16 offset, u8 value)
{
    assert(offset < OAM_SIZE);

    m_oam[offset] = value;
}

void PPU::cycle()
{
    ++m_dot_count;
    switch (m_mode) {
        case ModeFlag::OAM:
            if (m_dot_count == 1) {
                if (oam_interrupt_enabled())
                    m_emulator.cpu().request_LCD_interrupt();

                gather_sprites();
            }

            if (m_dot_count == 80) {
                m_dot_count = 0;
                m_mode = ModeFlag::TRANSFER;
            }
            break;

        case ModeFlag::TRANSFER:
            render_pixel();
            ++m_pixel_x;
            if (m_dot_count == 160) {
                m_dot_count = 0;
                m_mode = ModeFlag::HBLANK;
                m_pixel_x = 0;
            }
            break;

        case ModeFlag::HBLANK:
            if (m_dot_count == 1) {
                if (hblank_interrupt_enabled())
                    m_emulator.cpu().request_LCD_interrupt();
            }

            if (m_dot_count == 208) {
                m_dot_count = 0;
                set_line_y(m_pixel_y + 1);
                m_mode = (m_pixel_y == 144) ? ModeFlag::VBLANK : ModeFlag::OAM;
            }
            break;

        case ModeFlag::VBLANK:
            if (m_dot_count == 1) {
                if (vblank_interrupt_enabled())
                    m_emulator.cpu().request_LCD_interrupt();

                copy_pixels();
                m_emulator.cpu().request_vblank_interrupt();
                m_emulator.notify_frame_end();
            }

            if (m_dot_count % 456 == 0) {
                set_line_y(m_pixel_y + 1);
            }
            if (m_dot_count == 4560) {
                m_dot_count = 0;
                set_line_y(0);
                m_mode = ModeFlag::OAM;
            }
            break;

        default:
            assert(false); // unreachable
    }
}

void PPU::set_line_y(u8 value)
{
    m_pixel_y = value;

    if (coincidence_interrupt_enabled() && m_pixel_y == m_ly_compare)
        m_emulator.cpu().request_LCD_interrupt();
}

u8 PPU::TileData::color_at(usize x, usize y)
{
    assert(x < 8);
    assert(y < 8);

    auto byte_index = y * 2;
    auto bit_offset = 7 - x;

    bool low = (data[byte_index] & (1 << bit_offset)) > 0;
    bool high = (data[byte_index + 1] & (1 << bit_offset)) > 0;

    return (low ? 1 : 0) | (high ? 2 : 0);
}

u8 PPU::background_color_at(u8 x, u8 y)
{
    auto tile_x = x >> 3;
    auto tile_y = y >> 3;
    auto tile_offset = tile_x + tile_y * 32;
    u8 tile_index = m_vram[bg_tilemap_base() + tile_offset];

    if (tiled_data_signed_addressing())
        tile_index += 128;

    auto* data = bg_tile_data(tile_index);
    auto color_index = data->color_at(x % 8, y % 8);
    return color_index;
}

u8 PPU::window_color_at(u8 x, u8 y)
{
    auto tile_x = x >> 3;
    auto tile_y = y >> 3;
    auto tile_offset = tile_x + tile_y * 32;
    u8 tile_index = m_vram[window_tilemap_base() + tile_offset];

    if (tiled_data_signed_addressing())
        tile_index += 128;

    auto* data = bg_tile_data(tile_index);
    auto color_index = data->color_at(x % 8, y % 8);
    return color_index;
}

bool PPU::inside_window(u8 x, u8 y)
{
    return ((i32)x >= ((i32)m_window_x - 7) && (i32)y >= (i32)m_window_y);
}

void PPU::render_pixel()
{
    if (m_emulator.cpu().stopped()) {
        set_pixel(m_pixel_x, m_pixel_y, 0);
        return;
    }

    u8 bg_color_index = 0;
    u8 color = 0;

    if (!display_enabled()) {
        set_pixel(m_pixel_x, m_pixel_y, color);
        return;
    }

    if (bg_display_enabled()) {
        if (window_display_enabled() && inside_window(m_pixel_x, m_pixel_y)) {
            bg_color_index = window_color_at(
                    m_pixel_x + 7 - m_window_x,
                    m_pixel_y - m_window_y
                    );
            color = bg_palette()->color_for(bg_color_index);
        } else {
            bg_color_index = background_color_at(
                    m_pixel_x + m_scroll_x,
                    m_pixel_y + m_scroll_y
                    );
            color = bg_palette()->color_for(bg_color_index);
        }
    }

    if (!sprite_display_enabled()) {
        set_pixel(m_pixel_x, m_pixel_y, color);
        return;
    }

    // Sprites
    for (usize index = 0; index < m_scanline_sprite_count; ++index) {
        auto* sprite = m_scanline_sprites[index];

        if ((i32)m_pixel_x < sprite->x() || (i32)m_pixel_x >= (sprite->x() + 8))
            continue;

        auto coord_x = m_pixel_x - sprite->x();
        auto coord_y = m_pixel_y - sprite->y();

        if (sprite->x_flip())
            coord_x = 7 - coord_x;

        if (sprite->y_flip())
            coord_y = sprite_height() - coord_y - 1;

        auto sprite_tile_index = sprite->tile_index;
        if (sprite_double_height()) {
            sprite_tile_index = (sprite_tile_index & 0xfe) | (coord_y >> 3);
        }

        auto* sprite_tile_data = obj_tile_data(sprite_tile_index);
        auto color_index = sprite_tile_data->color_at(coord_x, coord_y & 0x07);

        if (color_index == 0)
            continue;

        auto* palette = obj_palette(sprite->palette_number());

        if (sprite->behind_bg()) {
            if (bg_color_index == 0)
                color = palette->color_for(color_index);
        } else if (color_index != 0) {
            color = palette->color_for(color_index);
        }
        break;
    }

    set_pixel(m_pixel_x, m_pixel_y, color);
}

void PPU::set_pixel(usize x, usize y, u8 color)
{
    auto index = (x + y * 160) * 3;

    assert(index < PIXEL_BUFFER_SIZE);

    m_pixels[index + 0] = COLORS[color][0];
    m_pixels[index + 1] = COLORS[color][1];
    m_pixels[index + 2] = COLORS[color][2];
}

void PPU::copy_pixels()
{
    u8* pixels { nullptr };
    int pitch;
    if (SDL_LockTexture(m_screen, NULL, (void**)&pixels, &pitch) != 0)
    {
        fprintf(stderr, "could not lock texture: %s\n", SDL_GetError());
        exit(-1);
    }

    for (usize y = 0; y < 144; ++y) {
        for (usize x = 0; x < 160; ++x) {
            usize texture_index = x * 4 + y * pitch;
            usize pixel_index = (x + y * 160) * 3;

            pixels[texture_index + 0] = m_pixels[pixel_index + 0];
            pixels[texture_index + 1] = m_pixels[pixel_index + 1];
            pixels[texture_index + 2] = m_pixels[pixel_index + 2];
        }
    }

    SDL_UnlockTexture(m_screen);
}

void PPU::gather_sprites()
{
    auto* sprites = reinterpret_cast<Sprite*>(m_oam);
    m_scanline_sprite_count = 0;

    for (usize index = 0; index < 40; ++index) {
        auto* sprite = &sprites[index];

        if ((i32)m_pixel_y >= sprite->y()
         && (i32)m_pixel_y < (sprite->y() + (i32)sprite_height())) {

            // Insertion sort
            usize reverse_index = m_scanline_sprite_count;
            while (reverse_index > 0
                    && m_scanline_sprites[reverse_index - 1]->x_pos > sprite->x_pos)
            {
                m_scanline_sprites[reverse_index] = m_scanline_sprites[reverse_index - 1];
                --reverse_index;
            }
            m_scanline_sprites[reverse_index] = sprite;

            ++m_scanline_sprite_count;
            if (m_scanline_sprite_count >= 10)
                break;
        }
    }
}

} // namespace GB
