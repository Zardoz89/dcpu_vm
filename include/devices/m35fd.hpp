#ifndef __M35FD__HPP__
#define __M35FD__HPP__ 1

#include <dcpu.hpp>
#include <config.hpp>

#include <cstdint>
#include <fstream>

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
    GET_NUMBER_TRACKS
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
    EJECT,      /// The floppy was ejected while was reading/writing
    BAD_SECTOR, /// The requested sector is broken, the data on it is lost

    BROKEN = 0xFFFF /// There's been some major software/hardware problem. 
                    /// Try to do a hard reset the device.
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
    virtual unsigned handleInterrupt();
    virtual void tick();
   
    /**
     * @brief Inserts a floppy in the unit
     * If there is a floppy disk previusly inserte, this is ejected
     * @param floppy Floppy disk
     */
    void insertFloppy(std::shared_ptr<M35_Floppy> floppy);

    /**
     * @brief Ejects the floppy actually isnerted if is there one
     */
    void eject();

    /**
     * Return floppy frive actual state
     */
    STATE_CODES getState() const
    {
        return state;
    }

    /**
     * Return floppy drive actual error state
     */
    ERROR_CODES getError() const
    {
        return error;
    }

protected:
    std::shared_ptr<M35_Floppy> floppy;     /// Floppy inserted
    STATE_CODES state;      /// Floppy drive actual estatus
    ERROR_CODES error;      /// Floppy drive actual error state

    unsigned busy_cycles;   /// CPU Cycles that the device will be busy
    uint16_t msg;           /// Mesg to send if need to trigger a interrupt
};

/**
 * @brief M35FD floppy disk
 *
 * File structure must be like this :
 * 
 * Bytes      0          1          2          3           4
 *            ----------------------------------------------
 * Head:      #   Type   | Version  #  Unused  |   Tracks  |
 *            ----------------------------------------------
 *            |                   Data                     |
 *            |                  size =                    |
 *            |  Tracks * SECTORS_PER_TRACK * SECTOR_SIZE  |
 *            |                                            |
 *            ----------------------------------------------
 *            |             Bad Sectors BitMap             |
 *            |                   size =                   |
 *            | Tracks * SECTORS_PER_TRACK *SECTOR_SIZE / 8|
 *            |                                            |
 *            ----------------------------------------------
 * Header:
 * Type = 'F' (Floppy image)
 * Version = 1
 * This kind of header will allow future upgrades and if we need diferent
 * data files (cassetes, tapes, hard disk, etc...), we will share the same
 * basic header.
 *
 * Floppy data:
 * Tracks : Number of tracks, should be 40 or 80, I don't expect any other
 * track size.
 *
 * Data: RAW data. To access a particular sector, you only need to read at
 *     (4 + sector * SECTOR_SIZE)
 * 
 * BitMap:
 * The bitmap stores 8 sectors state in each byte. It uses the MSB bit for 
 * the lowest sector and LSB for the bigger sector.
 * To read is a particular sector is bad, you read the byte at
 *     ( ( (4 + Size of Data secction) + 
 *         (sector * SECTOR_SIZE)/8 ) & 128 >> (sector % 8) ) != 0
 *
 * The RAW data will be read/write directly to the file, but the bitmap will
 * be keep in RAM for quick read of it.
 */
class M35_Floppy {
public:

    /**
     * Creates a new floppy device
     * @param filename Filename were the floppy data is stored
     * @param tracks Number of tracks of the floppy medium
     * @param wp Write protected ?
     */
    M35_Floppy(const std::string filename, uint8_t tracks = 80, 
               bool wp = false);
    virtual ~M35_Floppy();

    /**
     * Total number of tracks of this floppy
     */
    uint8_t getTotalTracks() const
    {
        return tracks;
    }
    
    /**
     * Total number of sectors of this floppy
     */
    uint16_t getTotalSectors() const
    {
        return tracks * SECTORS_PER_TRACK;
    }

    /**
     * Gets the actual Track
     */
    uint16_t getTrack() const
    {
        return last_sector / SECTORS_PER_TRACK;
    }

    /**
     * Sets Write protecction flag
     */
    void setProtected(bool val)
    {
        wp_flag = val;
    }

    /**
     * Return if is write protected
     */
    bool isProtected() const
    {
        return wp_flag;
    }
   
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
    ERROR_CODES read (uint16_t sector, unsigned& cycles, uint16_t* data, 
                       size_t size);

protected:

    uint8_t tracks;         /// Total tracks of the floppy
    uint8_t* bad_sectors;   /// Bitmap of bad sectors

    std::fstream datafile;


    bool wp_flag;           /// Is write protected
    uint16_t last_sector;   /// Last sector write/read

    /**
     * Moves the head to the desired track
     * @retun Number of cycles that need to seek the desired track
     */
    unsigned setTrack(uint16_t track)
    {
        if (track > getTrack() ) {
            return (track - getTrack()) * SEEK_CYCLES_PER_TRACK;
        } else {
            return (getTrack() - track) * SEEK_CYCLES_PER_TRACK;
        }
    }

};

} // END OF NAMESPACE block_device

} // END OF NAMESPACE cpu

#endif // __M35FD__HPP__
