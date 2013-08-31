#ifndef DCPU_HPP
#define DCPU_HPP 1

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
    
    uint16_t ra, rb, rc, rx, ry, rz , ri, rj, rex, rpc, ria, rsp; /// Registers
    
    const uint32_t cpu_clock = 100000;  /// CPU clock speed in Hz
    
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
    inline uint64_t getTotCycles() {
        return tot_cycles;
    };
    
    /**
     * @brief Return is in fire the CPU
     */
    inline bool getOnFire() {
        return on_fire;
    }
    
    // Get/Set for registers
    
    inline void SetA (const uint16_t& ra) {
        this->ra = ra;
    }
    
    inline void SetB (const uint16_t& rb) {
        this->rb = rb;
    }
    
    inline void SetC (const uint16_t& rc) {
        this->rc = rc;
    }
    
    inline void SetI (const uint16_t& ri) {
        this->ri = ri;
    }
    
    inline void SetJ (const uint16_t& rj) {
        this->rj = rj;
    }
    
    inline void SetX (const uint16_t& rx) {
        this->rx = rx;
    }
    
    inline void SetY (const uint16_t& ry) {
        this->ry = ry;
    }
    
    inline void SetZ (const uint16_t& rz) {
        this->rz = rz;
    }
    
    inline void SetPC (const uint16_t& rpc) {
        this->rpc = rpc;
    }
    
    inline void SetSP (const uint16_t& rsp) {
        this->rsp = rsp;
    }
    
    inline void SetEX (const uint16_t& rex) {
        this->rex = rex;
    }
    
    inline void SetIA (const uint16_t& ria) {
        this->ria = ria;
    }
    
    inline const uint16_t& GetA() const {
        return ra;
    }
    
    inline const uint16_t& GetB() const {
        return rb;
    }
    
    inline const uint16_t& GetC() const {
        return rc;
    }
    
    inline const uint16_t& GetI() const {
        return ri;
    }
    
    inline const uint16_t& GetJ() const {
        return rj;
    }
    
    inline const uint16_t& GetX() const {
        return rx;
    }
    
    inline const uint16_t& GetY() const {
        return ry;
    }
    
    inline const uint16_t& GetZ() const {
        return rz;
    }
    
    inline const uint16_t& GetPC() const {
        return rpc;
    }
    
    inline const uint16_t& GetSP() const {
        return rsp;
    }
    
    inline const uint16_t& GetEX() const {
        return rex;
    }
    
    inline const uint16_t& GetIA() const {
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
    
    
private:
    uint16_t* ram; /// RAM
	
	uint16_t* register_table[0x1E];
	
    
    inline int realStep();
    
    inline void triggerInterrupt (uint16_t msg);
    inline void handleInterrupt (uint16_t msg);
    inline void handleHWInterrupts();
    inline void tickHardware();
    
    bool int_queueing;  /// Queue interrupts
    bool skipping_flag; /// Skiping next instruccion ?
    bool on_fire; 		/// HCF
    
    /* hardware: */
    std::vector<std::shared_ptr<IHardware>> attached_hardware;
    
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

    IHardware() {
        index = -1;
        cpu = NULL;
    };
    virtual ~IHardware() {};
    
    virtual uint32_t getId() = 0;               /// Hardware ID
    virtual uint16_t getRevision() = 0;         /// Hardware Revision
    virtual uint32_t getManufacturer() = 0;     /// Hardware Manufacturer
    
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
    virtual void handleInterrupt() = 0;
    
    /**
     * @brief Does a Clock Tick
     */
    virtual void tick() = 0;
    
protected:
    DCPU* cpu;
    size_t index;   /// Hardware Device index
    
};


} // END OF NAMESPACE

#endif // DCPU_HPP
