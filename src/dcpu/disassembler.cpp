#include <dcpu/disassembler.hpp>
#include <dcpu/dcpu_opcodes.hpp>

#include <cassert>
#include <sstream>
#include <iomanip>


namespace cpu {

std::string disassembly(const uint16_t* data, size_t size)
{
    using namespace std;

    
    assert (data != NULL);
    assert (size > 0);
    
    stringstream str;
    string a;
    string b;
    string op;

    const uint16_t* ptr = data;
    
    //for (unsigned int i=0; i <size;i++)
    //{
        uint16_t w = *data;//[i];
        switch (GET_A(w)) {
        case REG_A:
            a = "A";
            break;
            
        case REG_B:
            a = "B";
            break;
            
        case REG_C:
            a = "C";
            break;
            
        case REG_X:
            a = "X";
            break;
            
        case REG_Y:
            a = "Y";
            break;
            
        case REG_Z:
            a = "Z";
            break;
            
        case REG_I:
            a = "I";
            break;
            
        case REG_J:
            a = "J";
            break;
            
            // registers, indirect:
        case PTR_A:
            a = "[A]";
            break;
            
        case PTR_B:
            a = "[B]";
            break;
            
        case PTR_C:
            a = "[C]";
            break;
            
        case PTR_X:
            a = "[X]";
            break;
            
        case PTR_Y:
            a = "[Y]";
            break;
            
        case PTR_Z:
            a = "[Z]";
            break;
            
        case PTR_I:
            a = "[I]";
            break;
            
        case PTR_J:
            a = "[J]";
            break;
            
            // registers + next word, indirect:
        case PTR_NW_A:
            if (size + data > ++ptr) {
                stringstream s;
                s << "[A + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                a = s.str();
            }
            
            break;
            
        case PTR_NW_B:
            if (size + data > ++ptr) {
                stringstream s;
                s << "[B + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                a = s.str();
            }
            
            break;
            
        case PTR_NW_C:
            if (size + data > ++ptr) {
                stringstream s;
                s << "[C + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                a = s.str();
            }
            
            break;
            
        case PTR_NW_X:
            if (size + data > ++ptr) {
                stringstream s;
                s << "[X + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                a = s.str();
            }
            
            break;
            
        case PTR_NW_Y:
            if (size + data > ++ptr) {
                stringstream s;
                s << "[Y + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                a = s.str();
            }
            
            break;
            
        case PTR_NW_Z:
            if (size + data > ++ptr) {
                stringstream s;
                s << "[Z + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                a = s.str();
            }
            
            break;
            
        case PTR_NW_I:
            if (size + data > ++ptr) {
                stringstream s;
                s << "[I + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                a = s.str();
            }
            
            break;
            
        case PTR_NW_J:
            if (size + data > ++ptr) {
                stringstream s;
                s << "[J + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                a = s.str();
            }
            
            break;
            
            // special registers:
        case STACK:
            a = "POP";
            break;  // POP
            
        case PEEK:
            a = "PEEK";
            break;  // PEEK
            
        case PICK:
            if (size + data > ++ptr) {
                stringstream s;
                s << "PICK 0x" << hex << setfill ('0') << setw (4)  << *ptr;
                a = s.str();
            } else
                a = "PICK ?";
                
            break;
            
        case REG_SP:
            a = "SP";
            break;
            
        case REG_PC:
            a = "PC";
            break;
            
        case REG_EX:
            a = "EX";
            break;
            
            // next word, indirect:
        case PTR_NW:
            if (size + data > ++ptr) {
                stringstream s;
                s << "[0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                a = s.str();
            } else
                a = "[?]";
                
            break;
            
            // next word, direct (literal):
        case NEXT_WORD:
            if (size + data > ++ptr) {
                stringstream s;
                s << "0x" << hex << setfill ('0') << setw (4)  << *ptr;
                a = s.str();
            } else
                a = "?";
                
            break;
            
            // literal value:
        default:
            stringstream s;
            s << GET_A(w) - LIT_B - 1; // (-1 to 30)
            a = s.str();
            break;
        }
        
        if (GET_OPCODE(w) != SPECIAL ) {
            switch (GET_B(w)) {
            case REG_A:
                b = "A";
                break;
                
            case REG_B:
                b = "B";
                break;
                
            case REG_C:
                b = "C";
                break;
                
            case REG_X:
                b = "X";
                break;
                
            case REG_Y:
                b = "Y";
                break;
                
            case REG_Z:
                b = "Z";
                break;
                
            case REG_I:
                b = "I";
                break;
                
            case REG_J:
                b = "J";
                break;
                
                // registers, indirect:
            case PTR_A:
                b = "[A]";
                break;
                
            case PTR_B:
                b = "[B]";
                break;
                
            case PTR_C:
                b = "[C]";
                break;
                
            case PTR_X:
                b = "[X]";
                break;
                
            case PTR_Y:
                b = "[Y]";
                break;
                
            case PTR_Z:
                b = "[Z]";
                break;
                
            case PTR_I:
                b = "[I]";
                break;
                
            case PTR_J:
                b = "[J]";
                break;
                
                // registers + next word, indirect:
            case PTR_NW_A:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "[A + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                    b = s.str();
                }
                
                break;
                
            case PTR_NW_B:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "[B + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                    b = s.str();
                }
                
                break;
                
            case PTR_NW_C:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "[C + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                    b = s.str();
                }
                
                break;
                
            case PTR_NW_X:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "[X + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                    b = s.str();
                }
                
                break;
                
            case PTR_NW_Y:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "[Y + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                    b = s.str();
                }
                
                break;
                
            case PTR_NW_Z:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "[Z + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                    b = s.str();
                }
                
                break;
                
            case PTR_NW_I:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "[I + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                    b = s.str();
                }
                
                break;
                
            case PTR_NW_J:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "[J + 0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                    b = s.str();
                }
                
                break;
                
                // special registers:
            case STACK:
                b = "PUSH";
                break;  // PUSH
                
            case PEEK:
                b = "PEEK";
                break;  // PEEK
                
            case PICK:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "PICK 0x" << hex << setfill ('0') << setw (4)  << *ptr;
                    b = s.str();
                } else
                    b = "PICK ?";
                    
                break;
                
            case REG_SP:
                b = "SP";
                break;
                
            case REG_PC:
                b = "PC";
                break;
                
            case REG_EX:
                b = "EX";
                break;
                
                // next word, indirect:
            case PTR_NW:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "[0x" << hex << setfill ('0') << setw (4)  << *ptr << "]";
                    b = s.str();
                } else
                    b = "[?]";
                    
                break;
                
                // next word, direct (literal):
            case NEXT_WORD:
                if (size + data > ++ptr) {
                    stringstream s;
                    s << "0x" << hex << setfill ('0') << setw (4)  << *ptr;
                    b = s.str();
                } else
                    b = "?";
                    
                break;
                
                // literal value:
            default:
                break;
                
            }
        }
        // Show Instrucction
        
        if (GET_OPCODE(w) != SPECIAL) {
            // Basic opcode
            
            switch (GET_OPCODE(w)) {
            case SET:
                op = "SET";
                break;
                
            case ADD:
                op = "ADD";
                break;
                
            case SUB:
                op = "SUB";
                break;
                
            case MUL:
                op = "MUL";
                break;
                
            case MLI:
                op = "MLI";
                break;
                
            case DIV:
                op = "DIV";
                break;
                
            case DVI:
                op = "DVI";
                break;
                
            case MOD:
                op = "MOD";
                break;
                
            case MDI:
                op = "MDI";
                break;
                
            case AND:
                op = "AND";
                break;
                
            case BOR:
                op = "BOR";
                break;
                
            case XOR:
                op = "XOR";
                break;
                
            case SHR:
                op = "SHR";
                break;
                
            case ASR:
                op = "ASR";
                break;
                
            case SHL:
                op = "SHL";
                break;
                
                // Branching
            case IFB:
                op = "IFB";
                break;
                
            case IFC:
                op = "IFC";
                break;
                
            case IFE:
                op = "IFE";
                break;
                
            case IFN:
                op = "IFN";
                break;
                
            case IFG:
                op = "IFG";
                break;
                
            case IFA:
                op = "IFA";
                break;
                
            case IFL:
                op = "IFL";
                break;
                
            case IFU:
                op = "IFU";
                break;
                
                
            case ADX:
                op = "ADX";
                break;
                
            case SBX:
                op = "SBX";
                break;
              
                
            case STI:
                op = "STI";
                break;
                
            case STD:
                op = "STD";
                break;
                
    /*        case NONB1:
            case NONB2:
            case NONB3:
            case NONB4:*/
            default:
                op = "???";
                break;
            }
            
            str << op << " " << b << ", " << a;
            
        } else {
            // Special opcode
            switch (GET_B(w)) {
            case JSR:
                op = "JSR";
                break;
                
            case HCF: // FIRE!
                op = "HCF";
                break;
                
            case INT:
                op = "INT";
                break;
                
            case IAG:
                op = "IAG";
                break;
                
            case IAS:
                op = "IAS";
                break;
                
            case RFI:
                op = "RFI";
                break;
                
            case IAQ:
                op = "IAQ";
                break;
                
            case HWN:
                op = "HWN";
                break;
                
            case HWQ:
                op = "HWQ";
                break;
                
            case HWI:
                op = "HWI";
                break;
                
            /*case NONS1:
            case NONS2:
            case NONS3:
            case NONS4:
            case NONS5:
            case NONS6:
            case NONS7:
            case NONS8:*/
            default:
                op = "???";
                break;
            }
            
            str << op << " " << a;
        }
        //str << std::endl;
    //}
    return str.str();
}

}
