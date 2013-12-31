#pragma once
#ifndef _DCPU_HPP_
#define _DCPU_HPP_ 1

#include <file.h>

#include <memory>
#include <cstdint>
#include <vector>
#include <deque>
#include <string>

namespace cpu {

const unsigned int DCPU_TOTAL_REGS  = 12;       /// DCPU registers
const unsigned int MAX_DEVICES      = 65535;    /// Max hardware devices
const unsigned int QUEUE_SIZE       = 256;      /// Int. queue size
const unsigned int RAM_SIZE         = 0x10000;  /// RAM size

class IHardware;

/**
 * @brief DCPU Virtual Machine
 * Based on Benedeck's DCPU VM (https://bitbucket.org/benedek/dcpu-16/overview)
 */
 
class DCPU : public std::enable_shared_from_this<DCPU> {
public:
    DCPU();
    virtual ~DCPU();
    
    /**
     * Return actual CPU model clock speed
     */
    virtual uint32_t getClock() const {return 100000; }
    
    /**
    * @brief Sets the CPU to the initial state
    * ADVICE: Also cleans the RAM
    */
    void reset();
    
    /**
     * @brief Executes a DCPU instrucction
     * @return Number of cycles that the DCPU need to do it
     */
    int step();
    
    /**
     * @brief Does a DCPU clock tick
     * @return True if executed an instrucction
     */
    bool tick(unsigned int n=1);
    
    /**
     * @brief Load a program to the DCPU RAM
     * @param prog Data program
     * @param size Data program size in 16 bit words
     * @param offset Offset to place the program in DCPU RAM
     */
    bool loadProgram (const uint16_t* prog,
                      unsigned int size,
                      unsigned int offset = 0);
                      
    /**
     * @brief Load a program to the DCPU RAM from a file
     * @param filename filename of the program
     * @param reverse_endian need to change the endianess
     * @param offset Offset to place the program in DCPU RAM
     */
    bool loadProgramFromFile (const std::string& filename,
                              bool reverse_endian=true,
                              unsigned int offset = 0);
    
    /**
     * @brief Attach a Hardware device to the DCPU
     * @param new_hw Ptr. to the device
     * @return Device Index if can be atached. If not, return -1
     */
    size_t attachHardware (std::shared_ptr<IHardware> new_hw);
    
    /**
     * @brief Detach a hardware device
     * @param index hardware to detach at desired index
     * @return Ptr to detached device. NULL if don't exists
     */
    std::shared_ptr<IHardware> detachHardware (size_t index);
    
    /**
     * @brief Return a pointer to the RAM
     * @return Ptr. to the RAM
     */
    inline uint16_t* getMem() {
        return ram;
    }
    
    /**
     * @brief Return total number of CPU clock cycles
     * @return Total number of cycles
     */
    inline uint64_t getTotCycles() const{
        return tot_cycles;
    };
    
    /**
     * @brief Return is in fire the CPU
     */
    inline bool getOnFire() const{
        return on_fire;
    }
    
    /**
     * @brief Return if is Queuing interrupts
     */
    inline bool isQueueing() const {
        return int_queueing;
    }

    // Get/Set for registers
    
    inline void setA (const uint16_t& ra) {
        this->ra = ra;
    }
    
    inline void setB (const uint16_t& rb) {
        this->rb = rb;
    }
    
    inline void setC (const uint16_t& rc) {
        this->rc = rc;
    }
    
    inline void setI (const uint16_t& ri) {
        this->ri = ri;
    }
    
    inline void setJ (const uint16_t& rj) {
        this->rj = rj;
    }
    
    inline void setX (const uint16_t& rx) {
        this->rx = rx;
    }
    
    inline void setY (const uint16_t& ry) {
        this->ry = ry;
    }
    
    inline void setZ (const uint16_t& rz) {
        this->rz = rz;
    }
    
    inline void setPC (const uint16_t& rpc) {
        this->rpc = rpc;
    }
    
    inline void setSP (const uint16_t& rsp) {
        this->rsp = rsp;
    }
    
    inline void setEX (const uint16_t& rex) {
        this->rex = rex;
    }
    
    inline void setIA (const uint16_t& ria) {
        this->ria = ria;
    }
    
    inline const uint16_t& getA() const {
        return ra;
    }
    
    inline const uint16_t& getB() const {
        return rb;
    }
    
    inline const uint16_t& getC() const {
        return rc;
    }
    
    inline const uint16_t& getI() const {
        return ri;
    }
    
    inline const uint16_t& getJ() const {
        return rj;
    }
    
    inline const uint16_t& getX() const {
        return rx;
    }
    
    inline const uint16_t& getY() const {
        return ry;
    }
    
    inline const uint16_t& getZ() const {
        return rz;
    }
    
    inline const uint16_t& getPC() const {
        return rpc;
    }
    
    inline const uint16_t& getSP() const {
        return rsp;
    }
    
    inline const uint16_t& getEX() const {
        return rex;
    }
    
    inline const uint16_t& getIA() const {
        return ria;
    }
    
    // pretty dump
    
    /**
     * @brief Generates a pretty print dump of registers values
     */
    std::string dumpRegisters();
    
    /**
     * @brief Generates a pretty HEX print dump of a RAM region
     */
    std::string dumpRam (uint16_t init = 0, uint16_t end = 0);
   
protected:

private:
    uint16_t ra, rb, rc, rx, ry, rz , ri, rj, rex, rpc, ria, rsp; /// Registers
    uint16_t* ram; /// RAM
    
    uint16_t* register_table[0x1E];
    
    
    inline int realStep();
    
    inline void triggerInterrupt (uint16_t msg);
    inline void handleInterrupt (uint16_t msg);
    inline void handleHWInterrupts();
    inline void tickHardware();
    
    bool int_queueing;  /// Queue interrupts
    bool skipping_flag; /// Skiping next instruccion ?
    bool on_fire;         /// HCF
    
    /* hardware: */
    std::vector<std::shared_ptr<IHardware>> attached_hardware;
    std::vector<std::shared_ptr<IHardware>> needtick_hardware; //reduce useless calls

    /* interrupt queue: */
    std::deque<uint16_t> int_queue;
    
    int wait_cycles;    // How many cycles need to wait to finish a instrucction
    uint64_t tot_cycles;
    
};


/**
 * Hardware device base class
 */
class IHardware {
public:

    IHardware() : cpu(NULL), index(-1)  { };
    virtual ~IHardware() {};
    
    virtual uint32_t getId() const = 0;             /// Hardware ID
    virtual uint16_t getRevision() const = 0;       /// Hardware Revision
    virtual uint32_t getManufacturer() const = 0;   /// Hardware Manufacturer
    
    /**
     * @brief Attach the Hardware device to the CPU
     * @param cpu Ptr to the CPU
     */
    virtual void attachTo (DCPU* cpu, size_t index) {
        this->cpu = cpu;
        this->index = index;
    }
    
    /**
     * @brief Detachs from the CPU
     */
    virtual void detach() {
        this->cpu = NULL;
    }
    
    /**
     * @brief Checks if need to trigger a interupt to the CPU
     * @param msg Ref. to the message
     * @return True if trigger a interrupt
    */
    virtual bool checkInterrupt (uint16_t &msg) {
        return false;
    }
    
    /**
     * @brief Handle a interrupt from DCP to Hardware
     * @return Number of cycles that DCPU waits
     */
    virtual unsigned handleInterrupt() = 0;
    
    /**
     * @brief Does a CPU Clock Tick
     */
	virtual void tick() = 0;

	/**
	 * @brief does the hardware need to be ticked ?!
	 */
	virtual bool needTick() {return true;} //leave true for maximum compatibility

    /**
     * @brief Return attached Hardware Dev. Index
     */
    const size_t& getDevIndex() const { return index; }
    
protected:
    DCPU* cpu;
    size_t index;   /// Hardware Device index
    
};


} // END OF NAMESPACE

#endif // _DCPU_HPP_
