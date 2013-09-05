#ifndef ____BIN__ASM
#define ____BIN__ASM

#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include "dcpu_opcodes.hpp"
#include "file.h"
#include <cstdlib>

namespace cpu {

//Assembler
class BinAsm
{
	public:
		BinAsm();
		~BinAsm();
		
		typedef struct
		{
			uint16_t offset;
			unsigned int line;
			std::string name;
		} Label;
		
		//Load from filename;
		bool load(const std::string& filename);
		
		//Remove Comments from the text
		void remove_comments(std::string& str);
		//Make lines from text
	    static std::vector<std::string> split_text(const std::string& text); 
		//Make word from lines (separator ' ' ',') "" conservate the integrity
		static std::vector<std::string> split_line(const std::string& line);
		
		//take a value from word
		uint16_t get_value(const std::string& word, std::string& err);
		
		//get_data from line return size target must have line.size()*2 
		//to be safe
		uint16_t get_data(const std::string& line, 
						  uint16_t* target, std::string& err);
		
		//Get op from word
		uint8_t get_op(const std::string& word);
		//Get sop from word
		uint8_t get_sop(const std::string& word, std::string& err);
		//Get A operator from word
		uint8_t get_a(const std::string& word, 
							uint16_t& data, std::string& err);
		uint8_t get_b(const std::string& word, 
							uint16_t& data, std::string& err);
							
		//search labels on the asm code
		bool finds_labels();

		
		bool save(const std::string& filename);
		
		bool print_error(unsigned int line, 
						 bool warning,
						 const std::string& err);
		
		void print_labels();
		
		
		
	protected:
		std::map<std::string,Label> _labels;
		std::string _fullsrc;
		std::string _src;
		std::string _filename;
		

};
}

#endif