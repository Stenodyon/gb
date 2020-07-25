#pragma once

#include "Defs.hpp"
#include <SDL2/SDL.h>

namespace GB {

static const usize AUDIO_SAMPLES_COUNT = 1024;
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
        inline bool stopped() const { return m_stopped; }
        inline void stop() { m_stopped = true; }

        inline u8 NR10() const { return m_NR10 | 0x80; }
        inline void set_NR10(u8 value) { m_NR10 = value; }
        inline u8 NR11() const { return m_NR11 | 0x3f; }
        inline void set_NR11(u8 value)
        {
            m_NR11 = value;
            reset_length_counter();
        }
        inline u8 NR12() const { return m_NR12; }
        inline void set_NR12(u8 value) { m_NR12 = value; }
        inline u8 NR13() const { return 0xff; }
        inline void set_NR13(u8 value) { m_NR13 = value; }
        inline u8 NR14() const { return m_NR14 | 0xbf; }
        inline void set_NR14(u8 value)
        {
            m_NR14 = value;
            if (m_NR14 & 0x80)
                restart();
        }

        inline u16 frequency() const
        {
            return ((u16)(m_NR14 & 0b111) << 8) | (u16)m_NR13;
        }
        inline void set_frequency(u16 value)
        {
            m_NR13 = (u8)(value & 0xff);
            m_NR14 = (m_NR14 & 0xf8) | (u8)(value >> 8);
        }
        inline u16 period() const { return (2048 - frequency()) * 4; }
        inline u8 duty() const { return (m_NR11 & 0xc0) >> 6; }
        inline u8 sweep_time() const { return (m_NR10 & 0x70) >> 4; }
        inline bool sweep_increases() const { return m_NR10 & 0x080; }
        inline u8 sweep_shifts() const { return m_NR10 & 0x07; }
        inline bool stop_after_length() const { return m_NR14 & 0x40; }
        inline u8 length_counter_base() const { return 64 - (m_NR11 & 0x1f); }
        inline u8 envelope_period() const { return m_NR12 & 0x07; }
        inline bool envelope_increases() const { return m_NR12 & 0x08; }
        inline u8 envelope_base_volume() const { return (m_NR12 & 0xf0) >> 4; }

    private:
        u8 m_NR10 { 0 };
        u8 m_NR11 { 0 };
        u8 m_NR12 { 0 };
        u8 m_NR13 { 0 };
        u8 m_NR14 { 0 };

        bool m_stopped { true };
        u16 m_duty_timer { 0 };
        usize m_frequency_timer { 0 };
        usize m_sweep_timer { 0 };
        usize m_sweep_counter { 0 };
        usize m_length_timer { 0 };
        u8 m_length_counter { 0 };
        usize m_envelope_timer { 0 };
        usize m_envelope_counter { 0 };
        u8 m_envelope_volume { 0 };

        void cycle_frequency();
        void cycle_sweep();
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

class Channel2 {
    public:
        void cycle();
        float sample();
        inline bool stopped() const { return m_stopped; }
        inline void stop() { m_stopped = true; }

        inline u8 NR20() const { return 0xff; }
        inline u8 NR21() const { return m_NR21 | 0x3f; }
        inline void set_NR21(u8 value)
        {
            m_NR21 = value;
            reset_length_counter();
        }
        inline u8 NR22() const { return m_NR22; }
        inline void set_NR22(u8 value) { m_NR22 = value; }
        inline u8 NR23() const { return 0xff; }
        inline void set_NR23(u8 value) { m_NR23 = value; }
        inline u8 NR24() const { return m_NR24 | 0xbf; }
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
        void cycle();
        float sample();
        inline void stop() { m_stopped = true; }
        inline bool stopped() const { return m_stopped; }

        inline u8 NR30() const { return m_NR30 | 0x7f; }
        inline void set_NR30(u8 value) { m_NR30 = value; }
        inline u8 NR31() const { return 0xff; }
        inline void set_NR31(u8 value) { m_NR31 = value; }
        inline u8 NR32() const { return m_NR32 | 0x9f; }
        inline void set_NR32(u8 value) { m_NR32 = value; }
        inline u8 NR33() const { return 0xff; }
        inline void set_NR33(u8 value) { m_NR33 = value; }
        inline u8 NR34() const { return m_NR34 | 0xbf; }
        inline void set_NR34(u8 value)
        {
            m_NR34 = value;
            if (m_NR34 & 0x80)
                restart();
        }
        inline u8 read_wave_pattern(u8 offset) const
        {
            return m_wave_pattern[offset];
        }
        inline void set_wave_pattern(u8 value, u8 offset)
        {
            if (playing())
                m_wave_pattern[m_wave_position] = value; // DMG quirk
            else
                m_wave_pattern[offset] = value;
        }

        inline bool playing() const { return m_NR30 & 0x80; }
        inline u8 sound_length() const { return 256 - m_NR31; }
        inline u8 output_level() const { return (m_NR32 & 0x60) >> 5; }
        inline u16 frequency() const
        {
            return ((u16)(m_NR34 & 0b111) << 8) | (u16)m_NR33;
        }
        inline u16 period() const { return (2048 - frequency()) * 2; }
        inline bool stop_after_length() const { return m_NR34 & 0x40; }
        inline void reset_length_counter() { m_length_counter = sound_length(); }

    private:
        u8 m_NR30 { 0 };
        u8 m_NR31 { 0 };
        u8 m_NR32 { 0 };
        u8 m_NR33 { 0 };
        u8 m_NR34 { 0 };
        u8 m_wave_pattern[0x10];

        bool m_stopped { true };
        usize m_frequency_timer { 0 };
        usize m_wave_position { 0 };
        usize m_length_timer { 0 };
        usize m_length_counter { 0 };

        void cycle_frequency();
        void cycle_length();
        void restart();
};

class Channel4 {
    public:
        void cycle();
        float sample();

        inline bool stopped() const { return m_stopped; }
        inline void stop() { m_stopped = true; }

        inline u8 NR40() const { return 0xff; }
        inline u8 NR41() const { return 0xff; }
        inline void set_NR41(u8 value) { m_NR41 = value; }
        inline u8 NR42() const { return m_NR42; }
        inline void set_NR42(u8 value) { m_NR42 = value; }
        inline u8 NR43() const { return m_NR43; }
        inline void set_NR43(u8 value) { m_NR43 = value; }
        inline u8 NR44() const { return m_NR44 | 0xbf; }
        inline void set_NR44(u8 value) {
            m_NR44 = value;
            if (m_NR44 & 0x80)
                restart();
        }
        inline u8 length_counter_base() const { return 64 - (m_NR41 & 0x1f); }
        inline bool stop_after_length() const { return m_NR44 & 0x40; }
        inline u8 envelope_period() const { return m_NR42 & 0x07; }
        inline bool envelope_increases() const { return m_NR42 & 0x08; }
        inline u8 envelope_base_volume() const { return (m_NR42 & 0xf0) >> 4; }
        inline u8 frequency_clock_shift() const { return (m_NR43 >> 4) + 1; }
        inline usize period() const
        {
            usize divisor = DIVISORS[m_NR43 & 0b111];
            return divisor << frequency_clock_shift();
        }
        inline bool soft_sound() const { return m_NR43 & 0x08; }

    private:
        static constexpr usize DIVISORS[8] = {
            8, 16, 32, 48, 64, 80, 96, 112,
        };

        u8 m_NR41 { 0 };
        u8 m_NR42 { 0 };
        u8 m_NR43 { 0 };
        u8 m_NR44 { 0 };

        bool m_stopped { true };
        usize m_frequency_timer { 0 };
        usize m_length_timer { 0 };
        usize m_length_counter { 0 };
        usize m_envelope_timer { 0 };
        usize m_envelope_counter { 0 };
        u8 m_envelope_volume { 0 };
        u16 m_shift_register { 0 };

        void cycle_frequency();
        void cycle_length();
        void cycle_envelope();
        void restart();
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

        void callback(i8*,int);

        inline u8 NR50() const { return m_NR50; }
        inline void set_NR50(u8 value) { m_NR50 = value; }
        inline u8 NR51() const { return m_NR51; }
        inline void set_NR51(u8 value) { m_NR51 = value; }
        inline u8 NR52() const
        {
            return (m_NR52 & 0x80)
                | (m_channel1.stopped() ? 0 : 0x01)
                | (m_channel2.stopped() ? 0 : 0x02)
                | (m_channel3.stopped() ? 0 : 0x04)
                | (m_channel4.stopped() ? 0 : 0x08)
                | 0x70;
        }
        inline void set_NR52(u8 value) {
            m_NR52 = value;
            if (!(m_NR52 & 0x80))
                stop();
        }

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

        inline u8 NR10() const { return m_channel1.NR10(); }
        inline void set_NR10(u8 value) { m_channel1.set_NR10(value); }
        inline u8 NR11() const { return m_channel1.NR11(); }
        inline void set_NR11(u8 value) { m_channel1.set_NR11(value); }
        inline u8 NR12() const { return m_channel1.NR12(); }
        inline void set_NR12(u8 value) { m_channel1.set_NR12(value); }
        inline u8 NR13() const { return m_channel1.NR13(); }
        inline void set_NR13(u8 value) { m_channel1.set_NR13(value); }
        inline u8 NR14() const { return m_channel1.NR14(); }
        inline void set_NR14(u8 value) { m_channel1.set_NR14(value); }

        inline u8 NR20() const { return m_channel2.NR20(); }
        inline u8 NR21() const { return m_channel2.NR21(); }
        inline void set_NR21(u8 value) { m_channel2.set_NR21(value); }
        inline u8 NR22() const { return m_channel2.NR22(); }
        inline void set_NR22(u8 value) { m_channel2.set_NR22(value); }
        inline u8 NR23() const { return m_channel2.NR23(); }
        inline void set_NR23(u8 value) { m_channel2.set_NR23(value); }
        inline u8 NR24() const { return m_channel2.NR24(); }
        inline void set_NR24(u8 value) { m_channel2.set_NR24(value); }

        inline u8 NR30() const { return m_channel3.NR30(); }
        inline void set_NR30(u8 value) { m_channel3.set_NR30(value); }
        inline u8 NR31() const { return m_channel3.NR31(); }
        inline void set_NR31(u8 value) { m_channel3.set_NR31(value); }
        inline u8 NR32() const { return m_channel3.NR32(); }
        inline void set_NR32(u8 value) { m_channel3.set_NR32(value); }
        inline u8 NR33() const { return m_channel3.NR33(); }
        inline void set_NR33(u8 value) { m_channel3.set_NR33(value); }
        inline u8 NR34() const { return m_channel3.NR34(); }
        inline void set_NR34(u8 value) { m_channel3.set_NR34(value); }
        inline u8 read_wave_pattern(u8 offset) const
        {
            return m_channel3.read_wave_pattern(offset);
        }
        inline void set_wave_pattern(u8 value, u8 offset)
        {
            m_channel3.set_wave_pattern(value, offset);
        }

        inline u8 NR40() const { return m_channel4.NR40(); }
        inline u8 NR41() const { return m_channel4.NR41(); }
        inline void set_NR41(u8 value) { m_channel4.set_NR41(value); }
        inline u8 NR42() const { return m_channel4.NR42(); }
        inline void set_NR42(u8 value) { m_channel4.set_NR42(value); }
        inline u8 NR43() const { return m_channel4.NR43(); }
        inline void set_NR43(u8 value) { m_channel4.set_NR43(value); }
        inline u8 NR44() const { return m_channel4.NR44(); }
        inline void set_NR44(u8 value) { m_channel4.set_NR44(value); }

    private:
        Emulator& m_emulator;

        SDL_AudioDeviceID m_device_id;
        SDL_AudioSpec m_audio_spec;

        SDL_sem* m_front_buffer_empty { nullptr };
        i8* m_front_buffer { nullptr };
        i8* m_back_buffer { nullptr };
        usize m_buffer_pos { 0 };
        usize m_cycle_counter { 0 };

        void sample_audio();
        void add_sample(i8 left, i8 right);
        void stop();

        u8 m_NR50 { 0 };
        u8 m_NR51 { 0 };
        u8 m_NR52 { 0 };
        Channel1 m_channel1;
        Channel2 m_channel2;
        Channel3 m_channel3;
        Channel4 m_channel4;
};

} // namespace GB
