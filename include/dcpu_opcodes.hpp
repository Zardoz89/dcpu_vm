#ifndef DCPU_OPCODES_HPP
#define DCPU_OPCODES_HPP 1

namespace cpu {

enum basic_opcodes {
    SPECIAL = 0X00,
    SET     = 0X01,
    ADD        = 0X02,
    SUB        = 0X03,
    MUL        = 0X04,
    MLI        = 0X05,
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
    
    IFB        = 0X10,
    IFC     = 0x11,
    IFE     = 0x12,
    IFN     = 0x13,
    IFG     = 0x14,
    IFA     = 0x15,
    IFL     = 0x16,
    IFU     = 0x17,
    
    NONB1   = 0x18,
    NONB2   = 0x19,
    
    ADX        = 0X1A,
    SBX     = 0x1B,
    
    NONB3   = 0x1C,
    NONB4   = 0x1D,
    
    
    STI        = 0X1E,
    STD     = 0x1F,
};

enum special_opcodes {
    N_A        = 0x00,
    JSR     = 0x01,
    
    NONS1   = 0x02,
    NONS2   = 0x03,
    NONS3   = 0x04,
    NONS4   = 0x05,
    NONS5   = 0x06,
    
    HCF        = 0x07,
    
    INT        = 0X08,
    IAG     = 0x09,
    IAS     = 0x0A,
    RFI     = 0x0B,
    IAQ     = 0x0C,
    
    NONS6   = 0x0D,
    NONS7   = 0x0E,
    NONS8   = 0x0F,
    
    
    HWN        = 0X10,
    HWQ     = 0x11,
    HWI     = 0x12,
};

/**
 * Operator values
 */
enum operator_type {
    REG_A = 0x00,
    REG_B = 0x01,
    REG_C = 0x02,
    REG_X = 0x03,
    REG_Y = 0x04,
    REG_Z = 0x05,
    REG_I = 0x06,
    REG_J = 0x07,
    
    PTR_A = 0x08,
    PTR_B = 0x09,
    PTR_C = 0x0A,
    PTR_X = 0x0B,
    PTR_Y = 0x0C,
    PTR_Z = 0x0D,
    PTR_I = 0x0E,
    PTR_J = 0x0F,
    
    PTR_NW_A = 0x10,
    PTR_NW_B = 0x11,
    PTR_NW_C = 0x12,
    PTR_NW_X = 0x13,
    PTR_NW_Y = 0x14,
    PTR_NW_Z = 0x15,
    PTR_NW_I = 0x16,
    PTR_NW_J = 0x17,
    
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


// OpCode and parameters extracction
#define GET_OPCODE(x)       ((x)&0x1F)
#define GET_A(x)            ((x) >> 10)
#define GET_B(x)            (((x)&0x03E0)>> 5)

} // END OF namespace

#endif // DCPU_OPCODES_HPP
