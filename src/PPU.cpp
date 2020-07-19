#include "PPU.hpp"
#include "Emulator.hpp"

#include <cassert>
#include <cstdlib>
#include <cstdio>

namespace GB {

const static u8 COLORS[4][3] = {
    { 15, 56, 15 },
    { 48, 98, 48 },
    { 139, 172, 15 },
    { 155, 188, 15 },
};

PPU::PPU(Emulator& emulator, SDL_Texture* screen)
    : m_emulator(emulator)
    , m_screen(screen)
{
    m_vram = (u8*)malloc(VRAM_SIZE);
    m_pixels = (u8*)malloc(160 * 144 * 3);
}

PPU::~PPU()
{
    free(m_pixels);
    free(m_vram);
}

u8 PPU::read8(u32 offset)
{
    return m_vram[offset];
}

void PPU::write8(u32 offset, u8 value)
{
    m_vram[offset] = value;
}

void PPU::cycle()
{
    ++m_dot_count;
    switch (m_mode) {
        case ModeFlag::OAM:
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
            }
            break;

        case ModeFlag::HBLANK:
            if (m_dot_count == 208) {
                m_dot_count = 0;
                ++m_pixel_y;
                if (m_pixel_y == 144) {
                    m_mode = ModeFlag::VBLANK;
                    copy_pixels();
                    m_emulator.notify_frame_end();
                } else {
                    m_mode = ModeFlag::OAM;
                }
            }
            break;

        case ModeFlag::VBLANK:
            if (m_dot_count % 456 == 0) {
                ++m_pixel_y;
            }
            if (m_dot_count == 4560) {
                m_dot_count = 0;
                m_pixel_y = 0;
                m_mode = ModeFlag::OAM;
            }
            break;

        default:
            assert(false); // unreachable
    }
}

u8 PPU::TileData::color_at(usize x, usize y)
{
    auto index = x + y * 8;
    auto byte_index = index / 4;
    auto byte = data[byte_index];
    auto bit_offset = 6 - (index % 4) * 2;

    return (byte & (0b11 << bit_offset)) >> bit_offset;
}

void PPU::render_pixel()
{
    // Background
    auto tile_x = m_pixel_x / 8;
    auto tile_y = m_pixel_y / 8;
    auto tile_offset = tile_x + tile_y * 32;
    auto tile_index = m_vram[bg_tilemap_base() + tile_offset];

    auto* data = tile_data(tile_index);
    auto color = data->color_at(m_pixel_x % 8, m_pixel_y % 8);
    set_pixel(m_pixel_x, m_pixel_y, color);
}

void PPU::set_pixel(usize x, usize y, u8 color)
{
    auto index = (x + y * 144) * 3;
    m_pixels[index + 0] = COLORS[color][0];
    m_pixels[index + 1] = COLORS[color][1];
    m_pixels[index + 2] = COLORS[color][2];
}

void PPU::copy_pixels()
{
    u8* pixels;
    int ignored_pitch;
    if (SDL_LockTexture(m_screen, NULL, (void**)&pixels, &ignored_pitch) != 0)
    {
        fprintf(stderr, "could not lock texture: %s\n", SDL_GetError());
        exit(-1);
    }

    for (usize index = 0; index < 160 * 144; ++index) {
        pixels[index * 4 + 1] = m_pixels[index * 3 + 0];
        pixels[index * 4 + 2] = m_pixels[index * 3 + 1];
        pixels[index * 4 + 3] = m_pixels[index * 3 + 2];
    }

    SDL_UnlockTexture(m_screen);
}

} // namespace GB
