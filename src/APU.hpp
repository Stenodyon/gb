#pragma once

#include "Defs.hpp"
#include <SDL2/SDL.h>

namespace GB {

static const usize AUDIO_SAMPLES_COUNT = 735;
static const usize CYCLES_PER_SAMPLE = (usize)(4194304.0 * 1000.0 / 44100.0);
static const float BASE_VOLUME = 0.10;

static const u8 DUTIES[4][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
};

class APU;

class Channel1 {
    public:
        void cycle();
        float sample();

    private:
        u8 m_NR10 { 0 };
        u8 m_NR11 { 0 };
        u8 m_NR12 { 0 };
        u8 m_NR13 { 0 };
        u8 m_NR14 { 0 };
};

class Channel2 {
    public:
        explicit Channel2(APU&);

        void cycle();
        float sample();
        inline bool stopped() const { return m_stopped; }
        inline void stop() { m_stopped = true; }

        inline u8 NR21() const { return m_NR21 & 0xc0; }
        inline void set_NR21(u8 value)
        {
            m_NR21 = value;
            reset_length_counter();
        }
        inline u8 NR22() const { return m_NR22; }
        inline void set_NR22(u8 value) { m_NR22 = value; }
        inline void set_NR23(u8 value) { m_NR23 = value; }
        inline u8 NR24() const { return m_NR24 & 0x40; }
        inline void set_NR24(u8 value)
        {
            m_NR24 = value;
            if (m_NR24 & 0x80)
                restart();
        }

        inline u16 period() const
        {
            u16 internal_repr = ((u16)(m_NR24 & 0b111) << 8) | (u16)m_NR23;
            return (2048 - internal_repr) * 4;
        }
        inline u8 duty() const { return (m_NR21 & 0xc0) >> 6; }
        inline bool stop_after_length() const { return m_NR24 & 0x40; }
        inline u8 length_counter_base() const { return 64 - (m_NR21 & 0x1f); }
        inline u8 envelope_period() const { return m_NR22 & 0x07; }
        inline bool envelope_increases() const { return m_NR22 & 0x08; }
        inline u8 envelope_base_volume() const { return (m_NR22 & 0xf0) >> 4; }

    private:
        APU& m_apu;

        u8 m_NR21 { 0 };
        u8 m_NR22 { 0 };
        u8 m_NR23 { 0 };
        u8 m_NR24 { 0 };

        bool m_stopped { true };
        u16 m_duty_timer { 0 };
        usize m_frequency_timer { 0 };
        usize m_length_timer { 0 };
        u8 m_length_counter { 0 };
        usize m_envelope_timer { 0 };
        usize m_envelope_counter { 0 };
        u8 m_envelope_volume { 0 };

        void cycle_frequency();
        void cycle_length();
        void cycle_envelope();
        void restart();
        inline void reset_length_counter()
        {
            m_length_counter = length_counter_base();
        }
        inline void reset_envelope()
        {
            m_envelope_volume = envelope_base_volume();
        }
};

class Channel3 {
    public:
    private:
        u8 m_NR30 { 0 };
        u8 m_NR31 { 0 };
        u8 m_NR32 { 0 };
        u8 m_NR33 { 0 };
        u8 m_NR34 { 0 };
        u8 m_wave_pattern[0x10];
};

class Channel4 {
    public:
    private:
        u8 m_NR41 { 0 };
        u8 m_NR42 { 0 };
        u8 m_NR43 { 0 };
        u8 m_NR44 { 0 };
};

class Emulator;

class APU {
    public:
        explicit APU(Emulator&);
        ~APU();

        void cycle();
        void pause();
        void unpause();
        inline u8 silence() const { return m_audio_spec.silence; }

        void callback(u8*,int);

        inline u8 NR50() const { return m_NR50; }
        inline void set_NR50(u8 value) { m_NR50 = value; }
        inline u8 NR51() const { return m_NR51; }
        inline void set_NR51(u8 value) { m_NR51 = value; }
        inline u8 NR52() const
        {
            return m_NR52
                | (m_channel2.stopped() ? 0 : 0x02)
                | 0x70;
        }
        inline void set_NR52(u8 value) { m_NR52 = (m_NR52 & 0x7f) | (value & 0x80); }

        inline Channel1& channel1() { return m_channel1; }
        inline Channel2& channel2() { return m_channel2; }
        inline Channel3& channel3() { return m_channel3; }
        inline Channel4& channel4() { return m_channel4; }

        inline bool sound_enabled() const { return m_NR52 & 0x80; }
        inline float left_volume() const
        {
            return (float)((m_NR50 & 0x70) >> 4) / 7.0;
        }
        inline float right_volume() const
        {
            return (float)(m_NR50 & 0x07) / 7.0;
        }
        inline bool channel1_to_left() const { return m_NR51 & 0x10; }
        inline bool channel2_to_left() const { return m_NR51 & 0x20; }
        inline bool channel3_to_left() const { return m_NR51 & 0x40; }
        inline bool channel4_to_left() const { return m_NR51 & 0x80; }
        inline bool channel1_to_right() const { return m_NR51 & 0x01; }
        inline bool channel2_to_right() const { return m_NR51 & 0x02; }
        inline bool channel3_to_right() const { return m_NR51 & 0x04; }
        inline bool channel4_to_right() const { return m_NR51 & 0x08; }

        inline u8 NR21() const { return m_channel2.NR21(); }
        inline void set_NR21(u8 value) { m_channel2.set_NR21(value); }
        inline u8 NR22() const { return m_channel2.NR22(); }
        inline void set_NR22(u8 value) { m_channel2.set_NR22(value); }
        inline void set_NR23(u8 value) { m_channel2.set_NR23(value); }
        inline u8 NR24() const { return m_channel2.NR24(); }
        inline void set_NR24(u8 value) { m_channel2.set_NR24(value); }

    private:
        Emulator& m_emulator;

        SDL_AudioDeviceID m_device_id;
        SDL_AudioSpec m_audio_spec;

        SDL_sem* m_front_buffer_empty { nullptr };
        u8* m_front_buffer { nullptr };
        u8* m_back_buffer { nullptr };
        usize m_buffer_pos { 0 };
        usize m_cycle_counter { 0 };

        void sample_audio();
        void add_sample(u8 left, u8 right);

        u8 m_NR50 { 0 };
        u8 m_NR51 { 0 };
        u8 m_NR52 { 0 };
        Channel1 m_channel1;
        Channel2 m_channel2;
        Channel3 m_channel3;
        Channel4 m_channel4;
};

} // namespace GB
