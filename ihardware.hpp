#ifndef IHARDWARE_HPP
#define IHARDWARE_HPP 1

#include <cstdint>

#include "dcpu.hpp"

namespace cpu {

class DCPU;
    
/**
 * Hardware device base class
 */
class IHardware {
public:

    virtual ~IHardware();    

    virtual uint32_t getId() = 0; 					/// Hardware ID
    virtual uint16_t getRevision() = 0;			/// Hardware Revision
    virtual uint32_t getManufacturer() = 0;	/// Hardware Manufacturer
    
    /**
     * @brief Attach the Hardware device to the CPU
     * @param cpu Ptr to the CPU
     */
    void attachTo (DCPU *cpu) {
        this->cpu = cpu;
    }
    
    /**
     * @brief Detachs from the CPU
     */
    void detach() {
        this->cpu = NULL;
    }
    
    /**
     * @brief Checks a interrupt from the CPU
     * @param msg Ref. to the message
     * @return
    */
    virtual bool checkInterrupt (uint16_t &msg) {
        return false;
    }
    
    /**
     * @brief Handle a interrupt
     */
    virtual void handleInterrupt();
    
    /**
     * @brief Does a Clock Tick
     */
    virtual void tick() = 0;
    
protected:
    DCPU *cpu;
    
};

}



#endif // IHARDWARE_HPP
