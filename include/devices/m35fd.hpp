#ifndef __M35FD__HPP__
#define __M35FD__HPP__ 1

#include <dcpu.hpp>
#include <config.hpp>

#include <cstdint>
#include <iostream>

namespace cpu {

namespace m35fd {

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
 * How many cycles need to move to a neraby track
 */
static const unsigned SEEK_CYCLES_PER_TRACK     = 240;

/**
 * How many cycles need to read/write a whole sector
 */
static const unsigned READ_CYCLES_PER_SECTOR    = 1668;
static const unsigned WRITE_CYCLES_PER_SECTOR   = 1668;

/**
 * How many sectors are in a track
 */
static const unsigned SECTORS_PER_TRACK         = 18;

/**
 * How many words are stored in a sector
 */
static const unsigned SECTOR_SIZE               = 512;

/**
 * Mackapar 3,5" floppy drive
 */
class M35FD : public IHardware {
public:
    M35FD();
    virtual ~M35FD();

    uint32_t getId() const               
    { 
        return 0x4FD524C5; 
    }

    virtual uint16_t getRevision() const 
    { 
        return 0x000B; 
    }

    uint32_t getManufacturer() const     
    { 
        return 0x1EB37E91;  // MACKAPAR

    }

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

    /**
     * Creates a new floppy device
     */
    M35_Floppy(const std::string filename , uint8_t tracks = 80);
    virtual ~M35_Floppy();

    /**
     * Total number of tracks of this floppy
     */
    uint8_t getTotalTracks() const;
    
    /**
     * Total number of sectors of this floppy
     */
    uint16_t getTotalSectors() const;

    /**
     * Gets the actual Track
     */
    uint16_t getTrack() const;
   
    /**
     * See if that sector is bad
     * Return True if is a bad sector
     */
    bool isSectorBad (uint16_t sector) const;

    /**
     * Change the bad sector flag of a particular sector
     * @param sector Desired sector
     * @param state True to damage these particular sector
     */
    void setSectorBad (uint16_t sector, bool state);

    /**
     * Try to write data at the desired sector
     * @param sector Desired sector to be writed
     * @param cycles Number of cycles that takes to finish the operation
     * @param data Data to be written in the sector
     * @return NONE, PROTECTED or BAD_SECTOR
     */
    ERROR_CODES write (uint16_t sector, unsigned& cycles, const uint16_t* data,
                       size_t size);
    
    /**
     * Try to read data at the desired sector
     * @param sector Desired sector to be writed
     * @param cycles Number of cycles that takes to finish the operation
     * @param data Buffer were to write the data
     * @return NONE or BAD_SECTOR
     */
    ERROR_CODES readS (uint16_t sector, unsigned& cycles, uint16_t* data, 
                       size_t size);

protected:
    // Note, this vars, are what will be write to the disk file in real world
    // To find a particular sector we use this :
    // ptr_word = sector * SECTORS_SIZE

    // File structure should be like this:
    // Byte : Number of tracks
    // RAW Data chunk of Tracks * SECTORS_PER_TRACK size
    // Bitmap of bad sectors of (Tracks * SECTORS_PER_TRACK) / 8
    //
    // The bitmap stores 8 sectors state in a byte. It uses the MSB bit for the
    // lowest sector and LSB for the bigger sector.
    //
    // The RAW data will be read/write directly to the file, butthe bitmap will
    // be keep in RAM for quick read of it.

    uint8_t tracks; /// Total tracks of the floppy
    uint8_t* bad_sectors; /// Bitmap of bad sectors

    std::iostream& datafile;

    /**
     * Moves the head to the desired track
     * @retun Number of cycles that need to seek the desired track
     */
    unsigned setTrack(uint16_t track);
};

} // END OF NAMESPACE block_device

} // END OF NAMESPACE cpu

#endif // __M35FD__HPP__
