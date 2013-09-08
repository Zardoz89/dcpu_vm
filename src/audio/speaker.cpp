#include "speaker.hpp"


namespace cpu {

namespace speaker {

    Speaker::Speaker() : freq(0), cb_function(NULL), c_obj(NULL)
    { }

    void Speaker::attachTo (DCPU* cpu, size_t index)
    {
        this->IHardware::attachTo(cpu, index);
        freq = 0;
    }

    void Speaker::handleInterrupt()
    {
        if (this->cpu == NULL)
            return;

        // We ignore A register, to allow a fake compatibility with benedeck's
        // dual channel speaker specs
        if (cpu->GetB() != freq) {
            freq = cpu->GetB();
            if (cb_function != NULL)
                cb_function(freq, c_obj);
        }
    }


} // END OF NAMESPACE speaker

} // END OF NAMESPACE cpu
