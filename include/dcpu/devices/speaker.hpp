#ifndef _SPEAKER_HPP_
#define _SPEAKER_HPP_ 1

#include <dcpu/dcpu.hpp>

namespace cpu {

namespace speaker {

/**
 * CallBack function of what to do when generated frecuency has changed
 * @param f New frecuency
 * @param obj Pointer to a object or anything that could need
 */
typedef void (*FreqChangeCallback)(uint16_t, void*);

/**
 * Hardware device that generates a square wave sound at desired frecueny
 * Emualtes 1 channel IBM/PC like speaker
 */
class Speaker : public IHardware {
public:

    Speaker();
    virtual ~Speaker() 
    { }

    uint32_t getId() const               
    {
        return 0x02060001;
    }

    virtual uint16_t getRevision() const 
    {
        return 0x0001;
    }
    uint32_t getManufacturer() const
    {
        return 0x5672746B; // VARTOK_HW
    } 
    virtual void attachTo (DCPU* cpu, size_t index);

    virtual unsigned handleInterrupt();
    virtual void tick() 
    { }

	virtual bool needTick() {return false;}

    /**
     * @brief Returns actual frencuency of square wave sound generated
     */
    const uint16_t& getFrequency() const { return freq; }

    /**
     * @brief Sets a callback to be called when the frecuency has been changed
     * @param cb Callback function. Set to NULL to disable callback
     * @param obj Object or other data needed by callback function
     */
    void setFreqCallback(FreqChangeCallback cb, void* obj)
    {
        cb_function = cb;
        c_obj = obj;
    }


protected:
    uint16_t freq;                      /// Square wave Frequency

    FreqChangeCallback cb_function;     /// Callback function
    void* c_obj;                        /// Callback object

};

} // END OF NAMESPACE speaker

} // END OF NAMESPACE cpu


#endif // _SPEAKER_HPP_
