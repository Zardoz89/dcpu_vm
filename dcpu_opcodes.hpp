#ifndef DCPU_OPCODES_HPP
#define DCPU_OPCODES_HPP 1

namespace cpu {

enum basic_opcodes {
    SPECIAL = 0X00,
    SET     = 0X01,
    ADD	    = 0X02,
    SUB	    = 0X03,
    MUL	    = 0X04,
    MLI	    = 0X05,
    DIV     = 0x06,
    DVI     = 0x07,
    MOD     = 0x08,
    MDI     = 0x09,
    AND     = 0x0A,
    BOR     = 0x0B,
    XOR     = 0x0C,
    SHR     = 0x0D,
    ASR     = 0x0E,   
    SHL     = 0x0F,
    
    IFB		= 0X10,
    IFC     = 0x11,
    IFE     = 0x12,
    IFN     = 0x13,
    IFG     = 0x14,
    IFA     = 0x15,
    IFL     = 0x16,
    IFU     = 0x17,
    
    NONB1   = 0x18,
    NONB2   = 0x19,
    
    ADX	    = 0X1A,
    SBX     = 0x1B,
    
    NONB3   = 0x1C,
    NONB4   = 0x1D,
    
    
    STI	    = 0X1E,
    STD     = 0x1F,
};

enum special_opcodes {
    N_A	    = 0x00,
    JSR     = 0x01,
    
    NONS1   = 0x02,
    NONS2   = 0x03,
    NONS3   = 0x04,
    MBG     = 0x05,
    MBS     = 0x06,
    
    HCF	    = 0x07,
    
    INT	    = 0X08,
    IAG     = 0x09,
    IAS     = 0x0A,
    RFI     = 0x0B,
    IAQ     = 0x0C,
    
    NONS4   = 0x0D,
    NONS5   = 0x0E,
    NONS6   = 0x0F,
    
    
    HWN	    = 0X10,
    HWQ     = 0x11,
    HWI     = 0x12,

    NONS7   = 0x13,
    NONS8   = 0x14,
    NONS9   = 0x15,

    GRM     = 0x16,
    DRM     = 0x17,
    SRT     = 0x18,
};

/**
 * Operator values
 */
enum operator_type {
    REG_A = 0x00,
    REG_B,
    REG_C,
    REG_X,
    REG_Y,
    REG_Z,
    REG_I,
    REG_J,
    
    PTR_A = 0x08,
    PTR_B,
    PTR_C,
    PTR_X,
    PTR_Y,
    PTR_Z,
    PTR_I,
    PTR_J,
    
    PTR_NW_A = 0x10,
    PTR_NW_B,
    PTR_NW_C,
    PTR_NW_X,
    PTR_NW_Y,
    PTR_NW_Z,
    PTR_NW_I,
    PTR_NW_J,
    
    STACK = 0x18,
    PEEK = 0x19,
    PICK = 0x1A,
    REG_SP = 0x1B,
    
    REG_PC = 0x1C,
    REG_EX = 0x1D,
    
    PTR_NW = 0x1E,
    NEXT_WORD = 0x1F,
    
    LIT_B = 0x20,
    LIT_E = 0x3F
};

/**
 * Instrucction extraction
 * Take directly from Benedeck code
 */
union opword {
    uint16_t raw;
    struct {
        uint8_t o : 5;
        uint8_t b : 5;
        uint8_t a : 6;
    } __attribute__ ( (__packed__) ) basic;
    struct {
        uint8_t zeros : 5;
        uint8_t o     : 5;
        uint8_t a     : 6;
    } __attribute__ ( (__packed__) ) nonbasic;
};

} // END OF namespace

#endif // DCPU_OPCODES_HPP
