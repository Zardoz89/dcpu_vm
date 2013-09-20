#ifndef DISASSEMBLER_HPP
#define DISASSEMBLER_HPP 1

#include <string>
#include <cstdint>

namespace cpu
{

/**
 * Disassembly DCPU-16 code to a humman redable text
 * @param data Ptr. to dcpu-16 machine code
 * @param size Size of data
 */
std::string disassembly(const uint16_t* data, size_t size);




}

#endif // DISASSEMBLER_HPP
