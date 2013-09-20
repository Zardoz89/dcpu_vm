#include <dcpu/devices/speaker.hpp>
#include <config.hpp>

namespace cpu {

namespace speaker {

Speaker::Speaker() : freq(0), cb_function(NULL), c_obj(NULL)
{ }

void Speaker::attachTo (DCPU* cpu, size_t index)
{
    this->IHardware::attachTo(cpu, index);
    freq = 0;
}

unsigned Speaker::handleInterrupt()
{
    if (this->cpu == NULL)
        return 0;

    // We ignore A register, to allow a fake compatibility with benedeck's
    // dual channel speaker specs
    if (cpu->getB() != freq) {
        freq = cpu->getB();
        if (freq > 10000)
            freq = 10000;

        LOG_DEBUG << "[speaker] Freq set to " << freq;
        if (cb_function != NULL)
            cb_function(freq, c_obj);
    }

    return 0;
}


} // END OF NAMESPACE speaker

} // END OF NAMESPACE cpu
