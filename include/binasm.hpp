#ifndef ____BIN__ASM
#define ____BIN__ASM

#include <dcpu/dcpu_opcodes.hpp>
#include <file.h>
#include <log.hpp>

#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cstdlib>
#include <cstring>

namespace cpu {

//Assembler
class BinAsm
{
	public:
		BinAsm();
		~BinAsm();
		
		
		//Load from filename;
		bool load(const std::string& filename);
		
		//Remove Comments from the text
		static void remove_comments(std::string& str);
		//Make lines from text
	    static std::vector<std::string> split_text(const std::string& text); 
		//Make word from lines (separator ' ' ',') "" conservate the integrity
		static std::vector<std::string> split_line(const std::string& line);
		
		//take a value from word
		bool get_value(const std::string& word, uint16_t& v, std::string& err, 
                                                            bool& unresolved);
        //Get strings datas or number
		bool get_data(const std::string& word,std::string& err
                                                            ,bool& unresolved);
/*
		//Get A operator from word return error
		bool resolve_a(const std::string& word,std::string& err);
        //Get B operator from word return error
        bool resolve_b(const std::string& word,std::string& err);
        //Add data return error
		bool resolve_data(const std::string& word,std::string& err);*/
        
        bool get_a(const std::string& word, uint8_t& a,
                                    uint16_t& data, std::string& err
                                                    ,bool& unresolved);
        bool get_b(const std::string& word, uint8_t& b,
                                    uint16_t& data, std::string& err
                                                    ,bool& unresolved,
                                                     bool is_conditionnal);
                            
               
                            
        //is the word an op
        static bool is_op(const std::string& word, uint8_t& op);
        //is the word an sop
        static bool is_sop(const std::string& word, uint8_t& op);
        //is the word a .dat or dat section
        static bool is_data_flag(const std::string& word);
        //is the word a .org or org section
        static bool is_offset_flag(const std::string& word);
        //is the word a .res or res or .resw or resw section
        static bool is_reserve_flag(const std::string& word);
        //is the word a register
        static char is_register(const std::string& word);
        //is the word a valid label
        static bool is_valid_label_name(const std::string& word);
        
        //is the current target/value requiert a data supplement
        static bool requiert_data(uint8_t a_or_b);
        //is the word a preprocessor directive
        static inline bool is_directive(const std::string& word)
        {
            return (word.size() && word[0]=='#');
        }
        
        bool assemble();

        //is the word a label definition
        static inline bool is_label_definition(const std::string& word)
        {
            if (word.size() && (word[0]==':' || word[word.size()-1]==':'))
                return true;
            return false;
        }
        
        static inline std::string remove_spaces(const std::string& word)
        {
            std::string u = word;
            size_t i = u.find(' ');
            while (i != std::string::npos)
            {
                u.erase(i,1);
                i = u.find(' ');
            }
            i = u.find('\t');
            while (i != std::string::npos)
            {
                u.erase(i,1);
                i = u.find('\t');
            }
            return u;
        }
        
        //resolves labels
        bool resolve_labels();
							
		
		bool save(const std::string& filename);
		
		bool print_error(unsigned int line, 
						 bool warning,
						 const std::string& err);
                         
        static inline bool is_conditionnal(uint8_t op)
        {
            return (op>=0x10 && op<=0x17);
        }
		
		
		
	protected:
		std::string _fullsrc;
		std::string _src;
        std::vector<std::string> _lines;
		std::string _filename;
        //Unresolved labels
        std::map<uint16_t,std::string> _unresolved;
        std::map<std::string,uint16_t> _labels;
        uint16_t _bin[0x10000];
        uint16_t _offset;
		

};
}

#endif
