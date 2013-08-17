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
    DIV,
    DVI,
    MOD,
    MDI,
    AND,
    BOR,
    XOR,
    SHR,
    ASR,
    SHL,
    IFB		= 0X10,
    IFC,
    IFE,
    IFN,
    IFG,
    IFA,
    IFL,
    IFU,
    
    ADX	    = 0X1A,
    SBX,
    STI	    = 0X1E,
    STD,
};

enum special_opcodes {
    N_A	    = 0x00,
    JSR,
    
    HCF	    = 0x07,
    
    INT	    = 0X08,
    IAG,
    IAS,
    RFI,
    IAQ,
    
    HWN	    = 0X10,
    HWQ,
    HWI,
};

} // END OF namespace

#endif // DCPU_OPCODES_HPP
