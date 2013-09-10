#ifndef __M35FD__HPP__
#define __M35FD__HPP__ 1

#include <dcpu.hpp>

#include <cstdint>

namespace cpu {

namespace m35fd {

// TODO Find information about the real write/read speed

/**
 * M35 Floppy Drive commands
 */
enum class COMMANDS : uint16_t {
    POLL,
    SET_INTERRUPT,
    READ_SECTOR,
    WRITE_SECTOR,
};

/**
 * M35 Floppy Drive status codes
 */
enum class STATE_CODES : uint16_t {
    NO_MEDIA,   /// There's no floppy in the drive
    READY,      /// The drive is ready to accept commands
    READY_WP,   /// Same as ready, but the floppy is Write Protected
    BUSY,       /// The drive is busy either reading or writing a sector
};

/**
 * M35 Floppy Device error codes
 */
enum class ERROR_CODES : uint16_t {
    NONE,       /// No error since the last poll
    BUSY,       /// Drive is busy performning a action
    NO_MEDIA,   /// Attempted to read or write without a floppy
    PROTECTED,  /// Attempted to write to a protected floppy
    EJECT,      /// The floppt was ejected while was reading/writing
    BAD_SECTOR, /// The requested sector is broken, the data on it is lost
    BROKEN,     /// There's been some major software/hardware problem. Try to 
                /// a hard reset the debice.
};

class M35_Floppy;

/**
 * Mackapar 3,5" floppy drive
 */
class M35FD : public IHardware {
public:
    M35FD();
    virtual ~M35FD();

    uint32_t getId()                { return 0x4FD524C5; }
    virtual uint16_t getRevision()  { return 0x000B; }
    uint32_t getManufacturer()      { return 0x1EB37E91; } // MACKAPAR

    virtual void attachTo (DCPU* cpu, size_t index);

    virtual bool checkInterrupt (uint16_t& msg);
    virtual void handleInterrupt();
    virtual void tick();
   
    /**
     * @brief Inserts a floppy in the unit
     * If there is a floppy disk previusly inserte, this is ejected
     * @param floppy Floppy disk
     */
    void insertFloppy(M35_Floppy& floppy);

    /**
     * @brief Ejects the floppy actually isnerted if is there one
     */
    void eject(); 

protected:
    M35_Floppy* floppy; 

};

/**
 * M35FD floppy disk
 */
class M35_Floppy {
public:
    M35_Floppy();
    virtual ~M35_Floppy();


protected:
};

} // END OF NAMESPACE block_device

} // END OF NAMESPACE cpu

#endif // __M35FD__HPP__
