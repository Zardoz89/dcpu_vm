#include <sfml/square_gen.hpp>

#include <algorithm>
#include <cmath>
#include "config.hpp"

namespace audio {

SquareGenerator::~SquareGenerator()
{
    if (samples != NULL)
        delete[] samples;
}

void SquareGenerator::prepare (uint16_t samplerate)
{
    samplerate = samplerate < 11025 ? 11025: samplerate; // Min 11025

    // initialize the base class
    initialize(1, samplerate);
    dt = 1.0 / (double)(samplerate);

    if (samples != NULL)
        delete[] samples;
    buff_size = samplerate / INV_LATENCY; 

    // Enought big to store any signal from 1 Hz to st/2 Hz
    samples = new int16_t[samplerate];
    
    freq = 0; phase = 0;
    std::fill_n(samples, samplerate, 0); // Silence
}

void SquareGenerator::setFreq(uint16_t f)
{
    if (f == 0) { // Silence
        freq = f;
        std::fill_n(samples, getSampleRate(), 0);
        buff_size = getSampleRate() / INV_LATENCY;
        return;
    }
    
    // ejem... Nyquist theorem
    auto sr_2 = getSampleRate() /2;
    f = f > sr_2 ? sr_2 : f;

    if (f < INV_LATENCY) { // We not need to use all buffer
        buff_size = (size_t)( std::ceil(getSampleRate() / (double)(f)));
    } else {
        buff_size = (size_t)(std::ceil(getSampleRate() / (double)(INV_LATENCY)));
    }
   
    freq = f;
}


bool SquareGenerator::onGetData(Chunk& data)
{
    if (freq > 0) {
        for (size_t i=0; i < buff_size; i++) {
            samples[i] = generator();
        }
    }

    data.sampleCount = buff_size;
    data.samples = samples;

    return true;
}


int16_t SquareGenerator::generator()
{
    double tmp = 2*3.1416 /(double)(getSampleRate());
    double out = std::sin(tmp * phase * freq); // Base freq

    uint32_t sr_2 = getSampleRate() /2;

    // We must avoid add harmonics over Nyquist limit or will be alised and 
    // will sound like strange noise mixed with the signal
    if (freq*3 < sr_2)
        out += std::sin(tmp * phase * 3*freq) / 3.0;
    else if (freq*5 < sr_2)
        out += std::sin(tmp * phase * 5*freq) / 5.0;
    else if (freq*7 < sr_2)
        out += std::sin(tmp * phase * 7*freq) / 7.0;
    else if (freq*9 < sr_2)
        out += std::sin(tmp * phase * 9*freq) / 9.0;
    else if (freq*11 < sr_2)
        out += std::sin(tmp * phase * 11*freq) / 11.0;
    else if (freq*13 < sr_2)
        out += std::sin(tmp * phase * 13*freq) / 13.0;
    else if (freq*15 < sr_2)
        out += std::sin(tmp * phase * 15*freq) / 15.0;
 
    out = 2 * out / 3.1416;

    phase++;
    return (int16_t) (out * 0.9 * INT16_MAX);
}

} // END OF NAMESPACE audio
