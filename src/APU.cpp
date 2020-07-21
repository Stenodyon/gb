#include "APU.hpp"
#include "Emulator.hpp"
#include <cassert>
#include <utility>
#include <cstring>

namespace GB {

void audio_callback(void*, u8*, int);

APU::APU(Emulator& emulator)
    : m_emulator(emulator)
    , m_channel2(*this)
{
    SDL_AudioSpec desired_spec;
    desired_spec.freq = 44100;
    desired_spec.format = AUDIO_U8;
    desired_spec.channels = 2;
    desired_spec.samples = AUDIO_SAMPLES_COUNT;
    desired_spec.callback = audio_callback;
    desired_spec.userdata = (void*)this;

    m_device_id = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &m_audio_spec, 0);
    if (m_device_id == 0) {
        fprintf(stderr, "Could not open audio device: %s\n", SDL_GetError());
        exit(-1);
    }

    m_front_buffer_empty = SDL_CreateSemaphore(1);
    m_front_buffer = (u8*)malloc(m_audio_spec.size * 2);
    m_back_buffer = m_front_buffer + m_audio_spec.size;
}

APU::~APU()
{
    SDL_CloseAudioDevice(m_device_id);

    if (m_front_buffer > m_back_buffer)
        std::swap(m_front_buffer, m_back_buffer);
    free(m_front_buffer);
}

void APU::cycle()
{
    m_channel1.cycle();
    m_channel2.cycle();

    m_cycle_counter += 1000;

    if (m_cycle_counter >= CYCLES_PER_SAMPLE) {
        sample_audio();
        m_cycle_counter %= CYCLES_PER_SAMPLE;
    }
}

void APU::pause()
{
    SDL_PauseAudioDevice(m_device_id, true);
}

void APU::unpause()
{
    SDL_PauseAudioDevice(m_device_id, false);
}

void APU::callback(u8* audio_stream, int length)
{
    if (SDL_SemValue(m_front_buffer_empty)) {
        std::memset(audio_stream, 0, length);
        fprintf(stderr, "WARNING: audio can't keep up!\n");
        return;
    }

    std::memcpy(audio_stream, m_front_buffer, length);
    SDL_SemPost(m_front_buffer_empty);
}

void audio_callback(void* apu, u8* audio_stream, int stream_length)
{
    reinterpret_cast<APU*>(apu)->callback(audio_stream, stream_length);
}

void APU::sample_audio()
{
    if (!sound_enabled()) {
        add_sample(silence(), silence());
        return;
    }

    float left_output = 0.0;
    float right_output = 0.0;

    float channel1_sample = m_channel1.sample();
    float channel2_sample = m_channel2.sample();

    if (channel1_to_left())
        left_output += channel1_sample;
    if (channel1_to_right())
        right_output += channel1_sample;
    if (channel2_to_left())
        left_output += channel2_sample;
    if (channel2_to_right())
        right_output += channel2_sample;

    left_output *= left_volume() * BASE_VOLUME;
    right_output *= right_volume() * BASE_VOLUME;

    u8 left_sample = (u8)(left_output * 255);
    u8 right_sample = (u8)(right_output * 255);

    add_sample(left_sample, right_sample);
}

void APU::add_sample(u8 left_sample, u8 right_sample)
{
    m_back_buffer[m_buffer_pos] = left_sample;
    ++m_buffer_pos;
    m_back_buffer[m_buffer_pos] = right_sample;
    ++m_buffer_pos;

    if (m_buffer_pos >= m_audio_spec.size) {
        SDL_SemWait(m_front_buffer_empty);
        std::swap(m_front_buffer, m_back_buffer);
        m_buffer_pos = 0;
    }
}

static const usize CYCLES_PER_LENGTH_TICK = 16384;
static const usize CYCLES_PER_ENVELOPE_TICK = 65536;

void Channel1::cycle()
{
}

float Channel1::sample()
{
    return 0.0;
}

Channel2::Channel2(APU& apu)
    : m_apu(apu)
{
}

void Channel2::cycle()
{
    ++m_duty_timer;

    if (m_duty_timer >= period()) {
        m_duty_timer = 0;
        cycle_frequency();
    }

    ++m_length_timer;
    if (m_length_timer >= CYCLES_PER_LENGTH_TICK) {
        m_length_timer = 0;
        if (m_length_counter != 0)
            cycle_length();
    }

    ++m_envelope_timer;
    if (m_envelope_timer >= CYCLES_PER_ENVELOPE_TICK) {
        m_envelope_timer = 0;
        if (envelope_period() != 0)
            cycle_envelope();
    }
}

void Channel2::cycle_frequency()
{
    ++m_frequency_timer;
    if (m_frequency_timer == 8)
        m_frequency_timer = 0;
}

void Channel2::cycle_length()
{
    --m_length_counter;

    if (m_length_counter == 0) {
        if (!stop_after_length())
            reset_length_counter();
        else
            stop();
    }
}

void Channel2::cycle_envelope()
{
    ++m_envelope_counter;
    if (m_envelope_counter >= envelope_period()) {
        m_envelope_counter = 0;

        if (m_envelope_volume == 0 && !envelope_increases())
            return;
        if (m_envelope_volume >= 0x0f && envelope_increases())
            return;
        m_envelope_volume += envelope_increases() ? 1 : -1;
    }
}

void Channel2::restart()
{
    reset_length_counter();
    reset_envelope();
    m_stopped = false;
}

float Channel2::sample()
{
    if (stopped())
        return 0;

    auto volume = (float)(m_envelope_volume & 0xf) / (float)0xf;
    return volume * (float)DUTIES[duty()][m_frequency_timer];
}


} // namespace GB
