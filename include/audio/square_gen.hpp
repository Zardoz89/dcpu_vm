#ifndef _SQUARE_GENERATOR_HPP_
#define _SQUARE_GENERATOR_HPP_ 1

#include <cstdint>
#include <cassert>

#include <SFML/Audio.hpp>

namespace audio {

/**
 * Generates a variable freq Square wave by Fourire synthesis
 * TODO: This a placeholder for a future more eficcient and elaborated generator
 * using BLIP_BUFF
 */
class SquareGenerator : public sf::SoundStream {
public:
	SquareGenerator() : samples(NULL) {}
    virtual ~SquareGenerator();

    void prepare (uint16_t samplerate = 44100);

    /**
     * @brief Sets generated frecuency
     * Set to 0 to generated a silence
     */
    void setFreq(uint16_t f);

    /**
     * @brief Returns actual frecuency
     */
    const uint16_t& getFreq() const { return freq;}

    void play()
    {
        phase = 0;
        this->sf::SoundStream::play();
    }

    /**
     * Wrapper static method to allow from a callback, change the freq
     */
    static void WrappeCallback(uint16_t f, void* obj)
    {
        assert(obj != NULL);
        SquareGenerator* mySelf = (SquareGenerator*) obj;
        mySelf->setFreq(f);
    }

protected:
    // Latency = 1/ Inv_Latency
    static const uint16_t INV_LATENCY = 10;

    double dt;                  /// Delta Time
    uint16_t freq;              /// Freq being generated
    size_t  phase;              /// Phase displazament in seconds
    
    int16_t* samples;           /// Samples buffer
    size_t buff_size;           /// Used buffer size (sr >= buff_size)

    /**
     * Returns a chunk of audio data to SFML playing thread
     */
    virtual bool onGetData(Chunk& data);
    
    /**
     * Not implemented
     */
    virtual void onSeek(sf::Time timeOffset)
    { }

    /**
     * Fourier synthesis generator
     */
    int16_t generator();
};


} // END OF NAMESPACE audio

#endif // _SQUARE_GENERATOR_HPP_
