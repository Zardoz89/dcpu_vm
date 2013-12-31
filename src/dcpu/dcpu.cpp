#include <dcpu/dcpu.hpp>
#include <dcpu/dcpu_opcodes.hpp>

#include <iostream>
#include <algorithm>
#include <sstream>
#include <cstdio>
#include <iomanip>

#include <cassert>

namespace cpu {


/// Checks if a OpCode is a Branch instrucction
#define IS_CONDITIONAL(x) ((x) >= IFB && (x) <= IFU)

#define PUSH ((uint16_t)(--rsp))    /// Push the stack
#define POP ((uint16_t)(rsp++))     /// Pop the stack


DCPU::DCPU()
{
    ram = new uint16_t[RAM_SIZE];
    attached_hardware.reserve (100); // Reserve space for some logical small qty
    register_table[REG_A] = &ra;
    register_table[REG_B] = &rb; 
    register_table[REG_C] = &rc; 
    register_table[REG_X] = &rx; 
    register_table[REG_Y] = &ry; 
    register_table[REG_Z] = &rz; 
    register_table[REG_I] = &ri; 
    register_table[REG_J] = &rj; 
    
    register_table[REG_SP] = &rsp; 
    register_table[REG_PC] = &rpc; 
    register_table[REG_EX] = &rex; 
    
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
    wait_cycles =  tot_cycles = 0;
    
}

bool DCPU::loadProgram (const uint16_t* prog,unsigned int size,unsigned int offset)
{
    assert (prog != NULL);
    assert (size > 0);    
    if (RAM_SIZE < offset + size)
    {
        LOG_ERROR << "Cannot load the program not enough ram !";
        return false;
    }
    
    std::copy_n (prog, size, ram + offset);
    return true;
}

bool DCPU::loadProgramFromFile(const std::string& filename,
                              bool reverse_endian,
                              unsigned int offset)
{

    FILE* f = fopen(filename.c_str(),"rb");
    if (!f)
    {
        LOG_ERROR << "File \"" << filename << "\" not found";
        return false;
    }
    int size = fsize(f);
    if (RAM_SIZE < offset + size/2 + 1)
    {
        char buff[33];
        sprintf(buff,"%d",size);
        LOG_ERROR << "Cannot load the program not enough ram ! (Program"
                  + std::string(" size: ") + buff;
        fclose(f);
        return false;
    }
    else if (size%2)
    {
        LOG_WARN <<  "File cannot be splitted into WORD properly";
    }
    
    
    fread(ram + offset, 2, size/2, f);
    fclose(f);
    
    if (reverse_endian)
        fswitchendian(ram + offset,size/2);
    return true;
}

bool DCPU::tick(unsigned int n)
{
    bool yes = false;
    for (unsigned int i = 0; i < n;i++)
    {
        tot_cycles++;
        if (wait_cycles <= 0) {
            wait_cycles = realStep();
            tickHardware();
            yes = true;
        }
        
        wait_cycles--;
        
        tickHardware();
    }
    return yes;
}

int DCPU::step() {
    int i, cycles = realStep();
    i = cycles;
    while (i-- > 0)
        tickHardware();

    tot_cycles += cycles;
    return cycles;
}

/**
 * @brief Executes a DCPU instrucction (real function)
 * @return Number of cycles that the DCPU need to do it
 */
int DCPU::realStep()
{
    int cycles = 0;
    
    uint16_t pos = rpc++;
    register uint16_t *a = NULL, *b = NULL;
    register int16_t sa, sb;            // Signed versions
    register uint_fast32_t tmp_result;
    register uint16_t tmp_a;            // Used by Literal values
    uint_fast16_t old_sp = rsp;
    
    const uint16_t op_word = GET_OPCODE(ram[pos]);
    const uint16_t op_a = GET_A(ram[pos]);
    const uint16_t op_b = GET_B(ram[pos]);
    const uint16_t special_op = op_b;

    const bool f_special = (op_word == SPECIAL);
    
    if (op_a <= REG_J || (op_a >= REG_SP && op_a <= REG_EX)) {
      a = register_table[op_a];
    
    } else if (op_a >=PTR_A && op_a <= PTR_J) {
      a = ram + *(register_table[op_a-PTR_A]);

    } else if (op_a >=PTR_NW_A && op_a <=PTR_NW_J) {
      a = ram + (uint16_t)(*(register_table[op_a-PTR_NW_A]) 
              + ram[(uint16_t)(rpc++)]);
    } else {
        switch (op_a) {

            // special registers:
        case STACK:
            a = ram + (uint16_t)(POP);
            break;  // POP
            
        case PEEK:
            a = ram + rsp;
            break;  // PEEK
            
        case PICK:
            a = ram + (uint16_t)(rsp + rpc++); // PICK n
            cycles++;
            break;
            
            // next word, indirect:
        case PTR_NW:
            a = ram + (uint16_t)(ram[(uint16_t)(rpc++)]);
            cycles++;
            break;
            
            // next word, direct (literal):
        case NEXT_WORD:
            a = ram + (uint16_t)(rpc++);
            cycles++;
            break;
            
            // literal value:
        default:
            tmp_a = op_a - LIT_B - 1; // (-1 to 30)
            a = &tmp_a;
            break;
        }
    }
    
    if (!f_special ) {
        if (op_b <= REG_J || (op_b >= REG_SP && op_b <= REG_EX)) {
          b=register_table[op_b];
        
        } else if (op_b >=PTR_A && op_b <=PTR_J) {
          b=ram + *(register_table[op_b-PTR_A]);

        } else if (op_b >=PTR_NW_A && op_b <=PTR_NW_J) {
          b=ram + *(register_table[op_b-PTR_NW_A]) + ram[ (rpc++)];
        } else {
            switch (op_b) {
                
                // special registers:
            case STACK:
                b = ram + (uint16_t)(PUSH);
                break;  // PUSH
                
            case PEEK:
                b = ram + rsp;
                break;    // PEEK
                
            case PICK:
                b = ram + (uint16_t)(rsp + rpc++); // PICK n
                cycles++;
                break;
                
                // next word, indirect:
            case PTR_NW:
                b = ram + (uint16_t)(ram[(uint16_t)(rpc++)]);
                cycles++;
                break;
                
                // next word, direct (literal):
            case NEXT_WORD:
                b = ram + (uint16_t)(rpc++);
                cycles++;
                break;
                
            }
        }
    }
    
    assert (a != NULL); // a and b must point to something at this point
    // assert( (op->basic.o != SPECIAL && b != NULL) || op->basic.o == SPECIAL);
    
    if (skipping_flag) { // We can't skip before, because we need to calculate cycles
        cycles = 0;
        // do not execute instruction when skipping
        // stop skipping when non-branch instruction is encountered
        if (!IS_CONDITIONAL (op_word) ) {
            skipping_flag = false;
        } else {
            cycles = 1;
        }
        
        // reading the values (above) can modify the SP; reset that here:
        rsp = old_sp;
        
        // Decode OpCodes and execute
    } else if (!f_special) {
        // Basic opcode
        
        switch (op_word) {
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
            /*printf("Error op Ox%x a 0x%x b 0x%x\n",WOPGET_OP(op),
                                                 WOPGET_A(op),WOPGET_B(op));*/
            cycles++;
            break;
        }
        
    } else {
        // Special opcode
        switch (special_op) {
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
            int_queueing = (*a > 0); // interrupts will be added to queue if a != 0
            cycles += 2;
            break;
            
        case HWN:
            *a = (uint16_t) attached_hardware.size();
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
                cycles += attached_hardware[*a]->handleInterrupt();
            }
            
            cycles += 4;
            break;
            
            
        default:
            // reserved; Does a NOP
            /*printf("Error spec zeros Ox%x a 0x%x op 0x%x\n",WOPGET_OP(op),
                                                 WOPGET_SPECIAL_A(op),WOPGET_SPECIAL_OP(op));*/
            cycles++;
            break;
        }
    }
    
    // only handle interrupts while not skipping (Notch said so)
    if (!skipping_flag)
        handleHWInterrupts();
         
    return cycles;
}


size_t DCPU::attachHardware (std::shared_ptr<IHardware> new_hw)
{
    if (attached_hardware.size() < MAX_DEVICES && new_hw) {
        attached_hardware.push_back (new_hw);
        new_hw->attachTo (this, attached_hardware.size() - 1);

		if (new_hw->needTick())
			needtick_hardware.push_back(new_hw);

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
		needtick_hardware.clear();

		for (auto hw = attached_hardware.begin(); hw != attached_hardware.end();
            hw ++) {
			if ((*hw)->needTick())
				needtick_hardware.push_back(*hw);
		}
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
 * @brief Handle an interrupt. Enqueue it if is necesary
 */
void DCPU::handleInterrupt (uint16_t msg)
{

    if (int_queueing) { // Queue interrupts if  It can
        if (int_queue.size() >= QUEUE_SIZE) {
            on_fire = true; // Catch fire and dont enqueue a aditional interrupt
            LOG_DEBUG << "[DCPU] Gets fire!!! by interrupt overlflow"; 
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

    for (auto hw = needtick_hardware.begin(); hw != needtick_hardware.end();
            hw ++) {
        (*hw)->tick();
    }
}


} // END NAMESPACE
