#include "dcpu.hpp"
#include "dcpu_opcodes.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>

#include <assert.h>

namespace cpu {


/// Checks if a OpCode is a Branch instrucction
#define IS_CONDITIONAL(x) ((x) >= IFB && (x) <= IFU)

#define PUSH (--rsp)    /// Push the stack
#define POP (rsp++)     /// Pop the stack


DCPU::DCPU()
{
    ram = new uint16_t[RAM_SIZE];
    attached_hardware.reserve (100); // Reserve space for some logical small qty
    reset();
}

DCPU::~DCPU()
{
    delete[] ram;
}

void DCPU::reset()
{
    std::fill_n (ram, RAM_SIZE, 0);
    ra = rb = rc = rx = ry = rz = ri = rj = rex = rsp = ria = rpc = 0;
    
    int_queueing = skipping_flag = on_fire = false;
    wait_cycles = tot_cycles = 0;
    
}

void DCPU::loadProgram (const uint16_t* prog, int size, int offset)
{
    assert (prog != NULL);
    assert (size > 0);
    assert (offset >= 0);
    assert (offset + size < 	UINT16_MAX);
    
    std::copy_n (prog, size, ram + offset);
    
}

bool DCPU::tick()
{
    if (wait_cycles <= 0) {
        wait_cycles = realStep();
        tickHardware();
        return true;
    }
    
    wait_cycles--;
    
    tickHardware();
    return false;
}

int DCPU::step() {
    int i, cycles = realStep();
    i = cycles;
    while (i-- > 0)
        tickHardware();
    
    return cycles;
}

/**
 * @brief Executes a DCPU instrucction (real function)
 * @return Number of cycles that the DCPU need to do it
 */
int DCPU::realStep()
{
    int cycles = 0;
    
    uint16_t *a = NULL, *b = NULL;
    int16_t sa, sb;   	// Signed versions
    uint_fast32_t tmp_result;
    uint16_t tmp_a;			// Used by Literal values
    uint_fast16_t old_sp = rsp;
    
    opword* op = (opword*) (ram + (rpc++) );
    
    switch (op->basic.a) {
        // registers, direct:
    case REG_A:
        a = &ra;
        break;
        
    case REG_B:
        a = &rb;
        break;
        
    case REG_C:
        a = &rc;
        break;
        
    case REG_X:
        a = &rx;
        break;
        
    case REG_Y:
        a = &ry;
        break;
        
    case REG_Z:
        a = &rz;
        break;
        
    case REG_I:
        a = &ri;
        break;
        
    case REG_J:
        a = &rj;
        break;
        
        // registers, indirect:
    case PTR_A:
        a = ram + ra;
        break;
        
    case PTR_B:
        a = ram + rb;
        break;
        
    case PTR_C:
        a = ram + rc;
        break;
        
    case PTR_X:
        a = ram + rx;
        break;
        
    case PTR_Y:
        a = ram + ry;
        break;
        
    case PTR_Z:
        a = ram + rz;
        break;
        
    case PTR_I:
        a = ram + ri;
        break;
        
    case PTR_J:
        a = ram + rj;
        break;
        
        // registers + next word, indirect:
    case PTR_NW_A:
        a = ram + ra + ram[ (rpc++)];
        cycles++;
        break;
        
    case PTR_NW_B:
        a = ram + rb + ram[ (rpc++)];
        cycles++;
        break;
        
    case PTR_NW_C:
        a = ram + rc + ram[ (rpc++)];
        cycles++;
        break;
        
    case PTR_NW_X:
        a = ram + rx + ram[ (rpc++)];
        cycles++;
        break;
        
    case PTR_NW_Y:
        a = ram + ry + ram[ (rpc++)];
        cycles++;
        break;
        
    case PTR_NW_Z:
        a = ram + rz + ram[ (rpc++)];
        cycles++;
        break;
        
    case PTR_NW_I: // TODO Mirar que pasa aqui !
        a = ram + ri + ram[ (rpc++)];
        cycles++;
        break;
        
    case PTR_NW_J:
        a = ram + rj + ram[ (rpc++)];
        cycles++;
        break;
        
        // special registers:
    case STACK:
        a = ram + (POP);
        break;  // POP
        
    case PEEK:
        a = ram + rsp;
        break;  // PEEK
        
    case PICK:
        a = ram + ( (uint16_t) (rsp + rpc++) ); // PICK n
        cycles++;
        break;
        
    case REG_SP:
        a = &rsp;
        break;
        
    case REG_PC:
        a = &rpc;
        break;
        
    case REG_EX:
        a = &rex;
        break;
        
        // next word, indirect:
    case PTR_NW:
        a = ram + ram[ (rpc++)];
        cycles++;
        break;
        
        // next word, direct (literal):
    case NEXT_WORD:
        a = ram + (rpc++);
        cycles++;
        break;
        
        // literal value:
    default:
        tmp_a = op->basic.a - LIT_B - 1; // (-1 to 30)
        a = &tmp_a;
        break;
    }
    
    if (op->basic.o != SPECIAL ) {
        switch (op->basic.b) {
            // registers, direct:
        case REG_A:
            b = &ra;
            break;
            
        case REG_B:
            b = &rb;
            break;
            
        case REG_C:
            b = &rc;
            break;
            
        case REG_X:
            b = &rx;
            break;
            
        case REG_Y:
            b = &ry;
            break;
            
        case REG_Z:
            b = &rz;
            break;
            
        case REG_I:
            b = &ri;
            break;
            
        case REG_J:
            b = &rj;
            break;
            
            // registers, indirect:
        case PTR_A:
            b = ram + ra;
            break;
            
        case PTR_B:
            b = ram + rb;
            break;
            
        case PTR_C:
            b = ram + rc;
            break;
            
        case PTR_X:
            b = ram + rx;
            break;
            
        case PTR_Y:
            b = ram + ry;
            break;
            
        case PTR_Z:
            b = ram + rz;
            break;
            
        case PTR_I:
            b = ram + ri;
            break;
            
        case PTR_J:
            b = ram + rj;
            break;
            
            // registers + next word, indirect:
        case PTR_NW_A:
            b = ram + ra + ram[ (rpc++)];
            cycles++;
            break;
            
        case PTR_NW_B:
            b = ram + rb + ram[ (rpc++)];
            cycles++;
            break;
            
        case PTR_NW_C:
            b = ram + rc + ram[ (rpc++)];
            cycles++;
            break;
            
        case PTR_NW_X:
            b = ram + rx + ram[ (rpc++)];
            cycles++;
            break;
            
        case PTR_NW_Y:
            b = ram + ry + ram[ (rpc++)];
            cycles++;
            break;
            
        case PTR_NW_Z:
            b = ram + rz + ram[ (rpc++)];
            cycles++;
            break;
            
        case PTR_NW_I:
            b = ram + ri + ram[ (rpc++)];
            cycles++;
            break;
            
        case PTR_NW_J:
            b = ram + rj + ram[ (rpc++)];
            cycles++;
            break;
            
            // special registers:
        case STACK:
            b = ram + (PUSH);
            break;  // PUSH
            
        case PEEK:
            b = ram + rsp;
            break;    // PEEK
            
        case PICK:
            b = ram + ( (uint16_t) (rsp + rpc++) ); // PICK n
            cycles++;
            break;
            
        case REG_SP:
            b = &rsp;
            break;
            
        case REG_PC:
            b = &rpc;
            break;
            
        case REG_EX:
            b = &rex;
            break;
            
            // next word, indirect:
        case PTR_NW:
            b = ram + ram[ (rpc++)];
            cycles++;
            break;
            
            // next word, direct (literal):
        case NEXT_WORD:
            b = ram + (rpc++);
            cycles++;
            break;
            
        }
    }
    
    assert (a != NULL); // a and b must point to something at this point
    // assert( (op->basic.o != SPECIAL && b != NULL) || op->basic.o == SPECIAL);
    
    if (skipping_flag) { // We can't skip before, because we need to calculate cycles
        // do not execute instruction when skipping
        // stop skipping when non-branch instruction is encountered
        if (!IS_CONDITIONAL (op->basic.o) ) {
            skipping_flag = false;
        } else {
            cycles += 2;
        }
        
        cycles ++;
        // reading the values (above) can modify the SP; reset that here:
        rsp = old_sp;
        
        // Decode OpCodes and execute
    } else if (op->basic.o != SPECIAL) {
        // Basic opcode
        
        switch (op->basic.o) {
        case SET:
            *b = *a;
            cycles++;
            break;
            
        case ADD:
            tmp_result = *b + *a;
            
            if ( (tmp_result & 0xFFFF0000) > 0) // Overflow
                rex = 0x0001;
            else
                rex = 0x0000;
                
            *b = (uint16_t) tmp_result;
            cycles += 2;
            break;
            
        case SUB:
            tmp_result = *b - *a;
            
            if ( (tmp_result & 0xFFFF0000) > 0) // Underflow
                rex = 0xFFFF;
            else
                rex = 0x0000;
                
            *b = (uint16_t) tmp_result;
            cycles += 2;
            break;
            
        case MUL:
            tmp_result = *b * *a;
            rex = (uint16_t) tmp_result >> 16;
            *b = (uint16_t) tmp_result;
            cycles += 2;
            break;
            
        case MLI:
            sa = *a;
            sb = *b;
            tmp_result = (uint32_t) (sa * sb);
            rex = (uint16_t) tmp_result >> 16;
            *b = (uint16_t) tmp_result;
            cycles += 2;
            break;
            
        case DIV:
            if (*a == 0) {
                rex = 0;
                *a = 0;
            } else {
                rex = ( (*b) << 16) / (*a);
                *b /= *a;
            }
            
            break;
            
        case DVI:
            sa = *a;
            sb = *b;
            
            if (sa == 0) {
                rex = 0;
                *a = 0;
            } else {
                rex = (sb << 16) / sa;
                *b = sb / sa;
            }
            
            cycles += 3;
            break;
            
        case MOD:
            if (*a == 0) *b = 0;
            else        *b = *b % *a;
            
            cycles += 3;
            break;
            
        case MDI:
            sa = *a;
            sb = *b;
            
            if (sa == 0) *b = 0;
            else        *b = sb % sa;
            
            cycles += 3;
            break;
            
        case AND:
            *b = *b & *a;
            cycles++;
            break;
            
        case BOR:
            *b = *b | *a;
            cycles++;
            break;
            
        case XOR:
            *b = *b ^ *a;
            cycles++;
            break;
            
        case SHR:
            rex = ( ( *b  << 16) >>  *a) & 0xffff;
            *b >>= *a;
            cycles++;
            break;
            
        case ASR:
            // C and C++ does arithmetic shift with signed types
            sa = *a;
            sb = *b;
            rex = ( (sb << 16) >> sa) & 0xffff;
            *b = sb >> sa;
            cycles ++;
            break;
            
        case SHL:
            rex = ( (*b << *a) >> 16) & 0xffff;
            *b <<= *a;
            cycles++;
            break;
            
            // Branching
        case IFB:
            if ( ( *b & *a) == 0) {
                skipping_flag = true;
                cycles ++;
            }
            
            cycles += 2;
            break;
            
        case IFC:
            if ( (*b & *a) != 0) {
                skipping_flag = true;
                cycles ++;
            }
            
            cycles += 2;
            break;
            
        case IFE:
            if (*b != *a ) {
                skipping_flag = true;
                cycles ++;
            }
            
            cycles += 2;
            break;
            
        case IFN:
            if (*b == *a ) {
                skipping_flag = true;
                cycles ++;
            }
            
            cycles += 2;
            break;
            
        case IFG:
            if (*b <= *a ) {
                skipping_flag = true;
                cycles ++;
            }
            
            cycles += 2;
            break;
            
        case IFA:
            sa = *a;
            sb = *b;
            
            if (sb <= sa ) {
                skipping_flag = true;
                cycles ++;
            }
            
            cycles += 2;
            break;
            
        case IFL:
            if (*b >= *a ) {
                skipping_flag = true;
                cycles ++;
            }
            
            cycles += 2;
            break;
            
        case IFU:
            sa = *a;
            sb = *b;
            
            if (sb >= sa ) {
                skipping_flag = true;
                cycles ++;
            }
            
            cycles += 2;
            break;
            
        case ADX:
            tmp_result = *b + *a + rex;
            
            if ( (tmp_result & 0xFFFF0000) > 0) // Overflow
                rex = 0x0001;
            else
                rex = 0x0000;
                
            *b = (uint16_t) tmp_result;
            cycles += 3;
            break;
            
        case SBX:
            tmp_result = *b - *a + rex;
            
            if ( (tmp_result & 0xFFFF0000) > 0) // Underflow
                rex = 0xFFFF;
            else
                rex = 0x0000;
                
            *b = (uint16_t) tmp_result;
            cycles += 3;
            break;
            
        case STI:
            *b = *a;
            ri++;
            rj++;
            cycles += 2;
            break;
            
        case STD:
            *b = *a;
            ri--;
            rj--;
            cycles += 2;
            break;
            
        default:
            // reserved; Act like a NOP
            cycles++;
            break;
        }
        
    } else {
        // Special opcode
        switch (op->nonbasic.o) {
        case JSR:
            ram[PUSH] = rpc;
            rpc = *a;
            cycles += 3;
            break;
            
        case HCF: // FIRE!
            on_fire = true;
            cycles += 9;
            break;
            
        case INT:
            handleInterrupt (*a);
            cycles += 4;
            break;
            
        case IAG:
            *a = ria;
            cycles++;
            break;
            
        case IAS:
            ria = *a;
            cycles++;
            break;
            
        case RFI:
            int_queueing = false;
            ra = ram[POP];
            rpc = ram[POP];
            cycles += 3;
            break;
            
        case IAQ:
            int_queueing = a != 0; // interrupts will be added to queue if a != 0
            cycles += 2;
            break;
            
        case HWN:
            *a = attached_hardware.size();
            cycles += 2;
            break;
            
        case HWQ:
        
            // sets A, B, C, X, Y registers to infoabout hardware a:
            //  A+(B<<16)  is a 32-bit word identifying the hardware id
            //  C          is the hardware version
            //  X+(Y<<16)  is a 32-bit word identifying the manufacturer
            if (*a < attached_hardware.size() ) {
                std::shared_ptr<IHardware> hw = attached_hardware[*a];
                ra = hw->getId();
                rb = hw->getId() >> 16;
                rc = hw->getRevision();
                rx = hw->getManufacturer();
                ry = hw->getManufacturer() >> 16;
            }
            
            cycles += 4;
            break;
            
        case HWI:
            if (*a < attached_hardware.size() ) {
                attached_hardware[*a]->handleInterrupt();
            }
            
            cycles += 4;
            break;
            
            
        default:
            // reserved; Does a NOP
            cycles++;
            break;
        }
    }
    
    // only handle interrupts while not skipping (Notch said so)
    if (!skipping_flag)
        handleHWInterrupts();
        
    tot_cycles += cycles;
    
    return cycles;
}


size_t DCPU::attachHardware (std::shared_ptr<IHardware> new_hw)
{
    if (attached_hardware.size() < MAX_DEVICES && new_hw) {
        attached_hardware.push_back (new_hw);
        new_hw->attachTo (this, attached_hardware.size() - 1);
        return attached_hardware.size() - 1;
    }
    
    return -1;
}

std::shared_ptr<IHardware> DCPU::detachHardware (size_t index)
{
    if ( index < attached_hardware.size() ) {
        auto dev = attached_hardware[index];
        attached_hardware.erase (index + attached_hardware.begin() );
        dev->detach();
        return dev;
    }
    
    return NULL;
}

std::string DCPU::dumpRegisters()
{
    using namespace std;
    
    stringstream str;
    str << "PC = 0x";
    str << hex << setfill ('0') << setw (4) << rpc << "\t";
    str << " A = 0x";
    str << hex << setfill ('0') << setw (4) << ra;
    str << " B = 0x";
    str << hex << setfill ('0') << setw (4) << rb;
    str << " C = 0x";
    str << hex << setfill ('0') << setw (4) << rc;
    str << " X = 0x";
    str << hex << setfill ('0') << setw (4) << rx;
    str << " Y = 0x";
    str << hex << setfill ('0') << setw (4) << ry;
    str << " Z = 0x";
    str << hex << setfill ('0') << setw (4) << rz << endl;
    str << "\t\t I = 0x";
    str << hex << setfill ('0') << setw (4) << ri;
    str << " J = 0x";
    str << hex << setfill ('0') << setw (4) << rj;
    str << "\t\tSP = 0x";
    str << hex << setfill ('0') << setw (4) << rsp;
    str << " IA = 0x";
    str << hex << setfill ('0') << setw (4) << ria;
    str << " EX = 0x";
    str << hex << setfill ('0') << setw (4) << rex;
    
    return str.str();
}

std::string DCPU::dumpRam (uint16_t init, uint16_t end)
{
    using namespace std;
    
    uint32_t bigend = end;
    uint32_t i = init;
    
    assert (i >= 0);
    assert (bigend >= i);
    assert (bigend < RAM_SIZE);
    
    if (bigend == 0 && i == 0) {
        i = bigend = rpc;
    }
    
    stringstream str;
    
    int wide = 0;
    
    for (; i <= bigend ; i++) {
    
        str << "0x";
        str << hex << setfill ('0') << setw (4) << ram[i] << " ";
        
        if (wide++ >= 7) {
            wide = 0;
            str << endl;
        }
        
    }
    
    return str.str();
}

// Private stuff

/**
 * @brief Trigger a Interrupt with a message if IA != 0
 */
void DCPU::triggerInterrupt (uint16_t msg)
{
    if (ria != 0) {
        int_queueing = true;
        ram[PUSH] = rpc;
        ram[PUSH] = ra;
        rpc = ria;
        ra = msg;
    }
}

/**
 * @brief Handle a interrupt. Enqueue it if is necesary
 */
void DCPU::handleInterrupt (uint16_t msg)
{
    if (int_queueing) { // Queue interrupts if  It can
        if (int_queue.size() >= QUEUE_SIZE) {
            on_fire = true; // Catch fire and dont enqueue a aditional interrupt
        } else {
            int_queue.push_back (msg);
        }
    } else {
        triggerInterrupt (msg);
    }
    
}

/**
 * @brief Handles interrupts FROM hardware
 */
void DCPU::handleHWInterrupts()
{
    if (on_fire) return;
    
    // check for new hardware interrupts
    for (auto hw = attached_hardware.begin(); hw != attached_hardware.end();
            hw++) {
        uint16_t msg;
        
        if ( (*hw)->checkInterrupt (msg) ) {
            handleInterrupt (msg);
        }
    }
    
    // trigger an interrupt from the queue:
    if (!int_queueing && int_queue.size() > 0) {
        triggerInterrupt (int_queue.front() );
        int_queue.pop_front();
    }
}

/**
 * @brief Execute a Hardware clock tic
 */
void DCPU::tickHardware()
{

    for (auto hw = attached_hardware.begin(); hw != attached_hardware.end();
            hw ++) {
        (*hw)->tick();
    }
}


} // END NAMESPACE
