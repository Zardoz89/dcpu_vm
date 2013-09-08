#include <binasm.hpp>

#include <iostream>

namespace cpu {

BinAsm::BinAsm() :
_offset(0)
{

}

BinAsm::~BinAsm()
{
	
}

bool BinAsm::load(const std::string& filename)
{
	_src = "";
	_fullsrc = "";
	_filename = filename;
	_offset=0;
    _unresolved.clear();
    _labels.clear();
	
	FILE* f = fopen(filename.c_str(),"r");
	if (!f)
	{
		std::cerr << "error : " << filename << " not found !" << std::endl;
	    return false;
	}

	unsigned int size = fsize(f);
    //using dawn good old memory allocation
	char* buff = (char*) malloc(size+1);
    memset((void*)buff,'\0',size+1);
	fread(buff,1,size,f);
	fclose(f);
    //strange bug windows add a t i don't know why...
    //if (size>2 && (buff[size-2]==EOF||buff[size-2]=='t')) buff[size-2] = '\0';
	buff[size] = '\0';
	_fullsrc = buff;
	_src=_fullsrc;
	remove_comments(_src);
    _lines = split_text(_src);
	free(buff);
	return true;
}

void BinAsm::remove_comments(std::string& str)
{
	std::string::iterator b = str.begin();
	bool in_comment = false;
	bool special_case = false;
	for (std::string::iterator i=b; i!=str.end();i++)
	{
		if (special_case && (*i == '\'' || *i == '"') && *(i-1) != '\\')
			special_case = false;
		else if (*i == '\'' || *i == '"')
			special_case = true;
		else if (*i == ';' && !special_case && !in_comment)
		{
			b = i;
			in_comment = true;
		}
		else if (*i == '\n' && in_comment) 
		{
			in_comment = false;
			str.erase(b,i);
			i=b;
		}
	}
	if (in_comment && !special_case) str.erase(b,str.end());
}

std::vector<std::string> BinAsm::split_text(const std::string& text)
{
	std::vector<std::string> lines;
    if (text.size()/8 > 0)
        lines.reserve(text.size()/8);
	unsigned int b=0;
	for (unsigned int i=0; i<text.size();i++)
	{
		if (text[i] == '\n' && i-b <= 1)
		{
            lines.push_back(std::string()); //empty line 
			b = i+1;
		}
		else if (text[i] == '\n') 
		{
			lines.push_back(text.substr(b,i-b));
			b=i+1;
		}
	}
    //last line
    if (b < text.size())
        lines.push_back(text.substr(b,text.size()-b));
	return lines;
}

std::vector<std::string> BinAsm::split_line(const std::string& line)
{
	std::vector<std::string>  words;
    if (line.size())
        words.reserve(line.size()/3);
    else 
        return words;
	unsigned int b=0;
	bool validate=false;
	char special_case=0;
    char separator=' ';
    char tab='\t';
	for (unsigned int i=0; i<line.size();i++)
	{
	    if (!special_case && !validate && line[i] != separator 
		  && line[i] != tab)
		{
			b=i;
			validate=true;
			if ((line[i] == '\'' || line[i] == '"') && 
			   (!i || line[i-1] != '\\'))
			  special_case = line[i];
			/*else if (line[i] == '[')
			  special_case = ']';*/
		}
		else if (special_case == line[i])
		{
			words.push_back(line.substr(b,i-b+1));
			validate=false;
			special_case=0;
		}
		else if (!special_case && validate &&
		        (line[i] == separator || line[i] == tab))
		{
            std::string word =line.substr(b,i-b);
            uint8_t u;
            if (is_op(word,u) || is_sop(word,u) || is_data_flag(word))
            {
                separator=',';
                tab='\0'; //tab
            }
			words.push_back(word);
			validate=false;
		}
	}
	if (validate) words.push_back(line.substr(b,line.size()-b));
	return words;
}

bool BinAsm::get_value(const std::string& word, uint16_t& v, std::string& err,
                                                            bool& unresolved)
{
	unsigned int s = word.size();
    unresolved=false;
    if (!s) return false;
	else if (word[0]== '\'' && s >= 3)
	{
        
		if (word[1] == '\\' && s == 4)
		{
            if (word[3] != '\'')
            {
                err = "excepted end ' with " + word;
                return false;
            }
			switch (word[2])
			{
				case 'n':
					return '\n';
				case 't':
					return '\t';
				case '\\':
					return '\\';
				case 'r':
					return '\r';
				default:
					err = "invalid special symbol value " +  word;
					return false;
			}
		}
        else if (s!=3)
        {
            err = "invalid char size for " + word;
            return false;
        }
        else if (word[2] != '\'')
        {
            err = "excepted end ' with " + word;
            return false;
        }
        v = word[1];
	    return true;
	}
    else if (word[0] >= '0' && word[0] <= '9')
    {
        char* c = const_cast<char*>(word.c_str());
        int number=strtol(word.c_str(),&c,0);
        if (c==word.c_str())
        {
            err = "invalid numericable value \"" + word + '"';
            return false; 
        }
        else if (number > 0xFFFF || number < -(0xFFFF))
        {
            err = "value " +  word + " overflow (max 0xFFFF)";
            return false;
        }
        v = static_cast<uint16_t>(number);
        return true;
    }
    else if (is_valid_label_name(word))
    {
        if (_labels.find(word)!=_labels.end())
        {
            v=_labels[word];
        }
        else
        {
            _unresolved[_offset]=word;
            err="unpreviously declared label ("+word;
            char buff[33];
            sprintf(buff,"0x%04X",_offset);
            err += std::string(" at offset :") + std::string(buff) + ')';
            unresolved=true;
            v=0x100; //big value cause won't create optimized instruction
        }
        return true;
    }
    return false;
}

bool BinAsm::print_error(unsigned int line, 
				bool warning,
				const std::string& err)
{
	if (!err.size()) return false;
	if (warning)
		std::cerr << "warning line " << line << ": " << err << std::endl;
	else
		std::cerr << "error line " << line << ": " << err << std::endl;
	return true;
}

bool BinAsm::get_data(const std::string& word,std::string& err,bool& unresolved)
{
    unresolved=false;
	if (word.size() && word[0] == '"')
    {
        if (word.size() < 2 || word[word.size()-1]!='"')
        {
            err="excepted \" at the end of input";
            return false;
        }
		for (unsigned int j=1; j < word.size()-1;j++)
		{
				if (word[j-1] == '\\')
				{
					switch (word[j+1])
					{
                        case 'n':
                            _bin[_offset] = '\n';
                        case 't':
                            _bin[_offset] = '\t';
                        case '\\':
                            _bin[_offset] = '\\';
                        case 'r':
                            _bin[_offset] = '\r';
                        default:
                            err = "invalid special symbol value " + word;
                            return false;
					}
							
				}
			    else if (word[j] == '\\')
                    continue;
                else
                    _bin[_offset] = word[j];
                    
                _offset++;
		}
    }
    else {
        uint16_t v;
        bool ret = get_value(remove_spaces(word), v, err,unresolved);
        _bin[_offset] = v;
        _offset++;
        return ret;
    }
    return true;
}


bool BinAsm::get_a(const std::string& word, uint8_t& a,
							uint16_t& data, std::string& err,bool& unresolved)
{
	std::string u = word;
	size_t i = u.find(' ');
    while (i != std::string::npos)
	{
		u.erase(i,1);
		i = u.find(' ');
	}
	std::string p = u;
	std::transform(p.begin(), p.end(), p.begin(), ::toupper);
	uint16_t code = 0;
	if (p=="A") {a=0x00; return true; }
    else if (p=="B") {a=0x01; return true; }
	else if (p=="C") {a=0x02; return true; }
	else if (p=="X") {a=0x03; return true; }
	else if (p=="Y") {a=0x04; return true; }
	else if (p=="Z") {a=0x05; return true; }
	else if (p=="I") {a=0x06; return true; }
	else if (p=="J") {a=0x07; return true; }
	
	else if (p=="[A]") {a=0x08; return true; }
	else if (p=="[B]") {a=0x09; return true; }
	else if (p=="[C]") {a=0x0A; return true; }
	else if (p=="[X]") {a=0x0B; return true; }
	else if (p=="[Y]") {a=0x0C; return true; }
	else if (p=="[Z]") {a=0x0D; return true; }
	else if (p=="[I]") {a=0x0E; return true; }
	else if (p=="[J]") {a=0x0F; return true; }
	
	else if (p.find("[A+")!=std::string::npos) {code=0x10; return true; }
	else if (p.find("[B+")!=std::string::npos) {code=0x11; return true; }
	else if (p.find("[C+")!=std::string::npos) {code=0x12; return true; }
	else if (p.find("[X+")!=std::string::npos) {code=0x13; return true; }
	else if (p.find("[Y+")!=std::string::npos) {code=0x14; return true; }
	else if (p.find("[Z+")!=std::string::npos) {code=0x15; return true; }
	else if (p.find("[I+")!=std::string::npos) {code=0x16; return true; }
	else if (p.find("[J+")!=std::string::npos) {code=0x17; return true; }
	
	
	else if (p=="POP"||p=="[SP++]") {a=0x18; return true; }
	else if (p=="PUSH"||p=="[--SP]") 
	{
		err="cannot push ([--SP]) on avalue";
        return false;
	}
	else if (p=="PEEK"||p=="[SP]") {a=0x19; return true; }
	else if (p=="PICK") {a=0x1A; return true; }
	
	
	else if (p=="SP") {a=0x1B; return true; }
	else if (p=="PC") {a=0x1C; return true; }
	else if (p=="EX") {a=0x1D; return true; }
	
	
	if (code && u.size() > 5)
	{
		if (get_value(u.substr(3,u.size()-4),data, err,unresolved))
        {
            a=code;
            return true;
        }
	}
	else if (u.size() > 2 && u[0] == '[')
	{
        a=0x1E; 
		if (u[u.size()-1] == ']') {
			return get_value(u.substr(1,u.size()-2), data, err,unresolved);
        }
		else {
			err="excepted ']' at the end of avalue pointer";
            return false;
        }
		
	}
	if (get_value(u,data, err, unresolved))
	{
		if (data == 0xFFFF) a=0x20;
		else if (data<=30)
		{
			a=data+0x21;
		}
		else
		{
			a=0x1F;
		}
        a = 0x1f;
        return true;
	}
	else
	{
        return false;
	}
}

bool BinAsm::get_b(const std::string& word, uint8_t& b,
							uint16_t& data, std::string& err,bool& unresolved)
{
	std::string u = word;
	size_t i = u.find(' ');
    while (i != std::string::npos)
	{
		u.erase(i,1);
		i = u.find(' ');
	}
	std::string p = u;
	std::transform(p.begin(), p.end(), p.begin(), ::toupper);
	uint16_t code = 0;
	if (p=="A") {b=0x00; return true; }
    else if (p=="B") {b=0x01; return true; }
	else if (p=="C") {b=0x02; return true; }
	else if (p=="X") {b=0x03; return true; }
	else if (p=="Y") {b=0x04; return true; }
	else if (p=="Z") {b=0x05; return true; }
	else if (p=="I") {b=0x06; return true; }
	else if (p=="J") {b=0x07; return true; }
	
	else if (p=="[A]") {b=0x08; return true; }
	else if (p=="[B]") {b=0x09; return true; }
	else if (p=="[C]") {b=0x0A; return true; }
	else if (p=="[X]") {b=0x0B; return true; }
	else if (p=="[Y]") {b=0x0C; return true; }
	else if (p=="[Z]") {b=0x0D; return true; }
	else if (p=="[I]") {b=0x0E; return true; }
	else if (p=="[J]") {b=0x0F; return true; }
	else if (p=="PUSH"||p=="[--SP]") {b=0x18; return true; }
	else if (p=="POP"||p=="[SP++]") 
	{
		err="cannot pop ([SP++]) on btarget";
        return false;
	}
	else if (p=="PEEK"||p=="[SP]") {b=0x19; return true; }
	else if (p=="PICK") {b=0x1A; return true; }
	
	
	else if (p=="SP") {b=0x1B; return true; }
	else if (p=="PC") {b=0x1C; return true; }
	else if (p=="EX") {b=0x1D; return true; }
    
    else if (p.find("[A+")!=std::string::npos) code=0x10;  
	else if (p.find("[B+")!=std::string::npos) code=0x11; 
	else if (p.find("[C+")!=std::string::npos) code=0x12;  
	else if (p.find("[X+")!=std::string::npos) code=0x13;  
	else if (p.find("[Y+")!=std::string::npos) code=0x14;  
	else if (p.find("[Z+")!=std::string::npos) code=0x15;  
	else if (p.find("[I+")!=std::string::npos) code=0x16; 
	else if (p.find("[J+")!=std::string::npos) code=0x17; 
	
	
	if (code && u.size() > 5)
	{
        b=code;
		return get_value(u.substr(3,u.size()-4),data, err,unresolved);
	}
	else if (u.size() > 2 && u[0] == '[')
	{
        b=0x1E;
		if (u[u.size()-1] == ']') {
			return get_value(u.substr(1,u.size()-2),data, err,unresolved);
            }
		else {
			err="excepted ']' at the end of btarget pointer";
            return false;
        }
		 
	}
	if (get_value(u, data,err,unresolved))
	{
		err="btarget must be a pointer or a register";
		/*if (data == 0xFFFF) b=0x20;
		else if (data<=30)
		{
			b=data+0x21;
		}
		else
		{
			b=0x1F;
		}
        //return true;*/
	}
	return false;
}

bool BinAsm::assemble()
{
	int error_count=0;
	unsigned int lc = 0;
    _offset=0;
	std::vector<std::string>::iterator lit = _lines.begin();
	for (;lit!=_lines.end();lit++)
	{
	    lc++;
		std::string err = std::string();
		std::vector<std::string> w=split_line(*lit);
		if (!w.size()) continue;
        
        //std::cout << "line " << lc << ":" << *lit << "\n";
        
        unsigned c = 0;
        w[c] = remove_spaces(w[c]);
        if (is_directive(w[c]))
        {
            print_error(lc,true,"preprocessor directive not supported yet (ignored)");
            continue;
        }
        if (is_label_definition(w[c]))
        {
            if (w[c][0]==':') w[c].erase(0,1);
            else w[c].erase(w[c].size()-1,1);
            
            if (is_valid_label_name(w[c]))
            {
                _labels[w[c]]=_offset;
            }
            else
            {
                print_error(lc,false,"invalid label name");
                error_count++;
            }
            c++; //lol
        }
        if (w.size() <= c) continue;
        w[c] = remove_spaces(w[c]);
        uint8_t op;
        if (is_op(w[c], op))
        {
            if (w.size() != c+3)
            {
                err ="instruction " + w[c];
				err += " need 2 arguments";
                print_error(lc,false,err);
                error_count++;
            }
            else 
            {
                uint16_t opcode = op & 0x1F;
                uint16_t a_word=0, b_word=0;
                uint8_t a=0, b=0;
                bool unresolved=false;
                //A Block
                _offset++; //for get_a bug
                if (get_a(w[c+2],a,a_word,err,unresolved))
                {
                    opcode |= (a << 10);
                    if (unresolved)
                    {
                        unresolved=false;
                        print_error(lc,true,err);
                    }
                    if (requiert_data(a))
                    {
                        _bin[_offset]=a_word;
                    }
                }
                else
                {
                    if (err.size())
                    {
                        print_error(lc,false,err);
                    }
                    else {
                        print_error(lc,false,"cannot get avalue properly");
                    }
					error_count++;
                }
                _offset--;
                 
                 
                //B Block
                _offset++; //for get_b bug
                if (requiert_data(a))  _offset++;
                if (get_b(w[c+1],b,b_word,err,unresolved))
                {
                    opcode |= (b & 0x1F) << 5;
                    if (unresolved)
                    {
                        unresolved=false;
                        print_error(lc,true,err);
                    }
                    if (requiert_data(b))
                    {
                        _bin[_offset]=b_word;
                    }
                    _offset--;
                    if (requiert_data(a))
                    {
                        _offset--;
                    }
                    _bin[_offset]=opcode;
                }
                else 
                {
                    if (err.size())
                    {
                        print_error(lc,false,err);
                    }
                    else {
                        print_error(lc,false,"cannot get btarget properly");
                    }
					error_count++;
                }
                _offset++;
                if (requiert_data(b))
                {
                    _offset++;
                }
                if (requiert_data(a))
                {
                    _offset++;
                }
            }
        }
        else if (is_sop(w[c], op))
        {
            if (w.size() != c+2)
            {
                err ="instruction " + w[c];
				err += " need 2 arguments";
                print_error(lc,false,err);
                error_count++;
            }
            else 
            {
                uint16_t opcode = (op & 0x1F) << 5;
                uint16_t a_word=0;
                uint8_t a=0;
                bool unresolved=false;
                //A Block
                _offset++;//for get_b bug
                if (get_a(w[c+1],a,a_word,err,unresolved))
                {
                    opcode |= a << 10;
                    if (unresolved)
                    {
                        unresolved=false;
                        print_error(lc,true,err);
                    }
                    if (requiert_data(a))
                    {
                        _bin[_offset]=a_word;
                    }
                    _offset--;
                    _bin[_offset]=opcode;
                }
                else 
                {
                    if (err.size())
                    {
                        print_error(lc,false,err);
                    }
                    else {
                        print_error(lc,false,"cannot get avalue properly");
                    }
					error_count++;
                }
                _offset++;
                if (requiert_data(a))
                {
                    _offset++;
                }
            }
        }
        else if (is_data_flag(w[c]))
        {
            err=std::string();
            for (c++;c<w.size();c++)
            {
                bool unresolved = false;
                if (get_data(w[c],err,unresolved))
                {
                    if (unresolved)
                    {
                      print_error(lc,true,err);
                    }
                }
                else if (err.size())
                {
                    print_error(lc,false,err);
                    error_count++;
                }
                if (err.size()) //warnings
                {
                    print_error(lc,true,err);
                }
            }
        }
        else 
        {
            err ="unexcepted expression " + w[c];
            print_error(lc,false,err);
            error_count++;
        }
	}
	
	
	
	if (error_count)
	{
		std::cerr << "assembling terminated with " << error_count; 
		std::cerr << " error(s)" << std::endl;
        return false;
	}
    return true;
}
    
bool BinAsm::save(const std::string& filename)
{
    FILE* f = fopen(filename.c_str(), "wb");
    if (!f) {
        std::cerr << "error: cannot open output file " << filename << '\n';
        return false;
    }
        
    fswitchendian(_bin, _offset);
    fwrite(_bin,2,_offset,f);
    fclose(f);
    std::cout << "writing " << filename;
    std::cout << " terminated final size " << _offset*2;
    std::cout << " bytes" << std::endl;
        
    return true;
}

bool BinAsm::is_op(const std::string& word, uint8_t& op)
{
    std::string p = word;
	std::transform(p.begin(), p.end(), p.begin(), ::toupper);
    if (p=="SET") op=0x01;
	else if (p=="ADD") op=0x02;
	else if (p=="SUB") op=0x03;
	else if (p=="MUL") op=0x04;
	else if (p=="MLI") op=0x05;
	else if (p=="DIV") op=0x06;
	else if (p=="DVI") op=0x07;
	else if (p=="MOD") op=0x08;
	else if (p=="MDI") op=0x09;
	else if (p=="AND") op=0x0A;
	else if (p=="BOR") op=0x0B;
	else if (p=="XOR") op=0x0C;
	else if (p=="SHR") op=0x0D;
	else if (p=="ASR") op=0x0E;   
	else if (p=="SHL") op=0x0F;
	else if (p=="IFB") op=0x10;
	else if (p=="IFC") op=0x11;
	else if (p=="IFE") op=0x12;
	else if (p=="IFN") op=0x13;
	else if (p=="IFG") op=0x14;
	else if (p=="IFA") op=0x15;
	else if (p=="IFL") op=0x16;
	else if (p=="IFU") op=0x17;
	else if (p=="ADX") op=0x1A;
	else if (p=="SBX") op=0x1B;
	else if (p=="STI") op=0x1E;
	else if (p=="STD") op=0x1F;
	else return false;
    return true;
}

bool BinAsm::is_sop(const std::string& word, uint8_t& op)
{
    std::string p = word;
	std::transform(p.begin(), p.end(), p.begin(), ::toupper);
	if (p=="JSR") op=0x01;
	else if (p=="HCF") op=0x07;
	else if (p=="INT") op=0X08;
	else if (p=="IAG") op=0x09;
	else if (p=="IAS") op=0x0A;
	else if (p=="RFI") op=0x0B;
	else if (p=="IAQ") op=0x0C;
	else if (p=="HWN") op=0X10;
	else if (p=="HWQ") op=0x11;
	else if (p=="HWI") op=0x12;
	else return false;
    return true;
}

bool BinAsm::is_data_flag(const std::string& word)
{
    std::string p = word;
	std::transform(p.begin(), p.end(), p.begin(), ::toupper);
    if (p=="DAT"|| p==".DAT")
        return true;
    return false;
}

bool BinAsm::is_valid_label_name(const std::string& word)
{
    uint8_t useless;
    if (is_sop(word, useless) || is_op(word, useless)  || 
        is_data_flag(word) || is_register(word))
        return false;
    for (unsigned i=0;i<word.size();i++)
    {
        if (!((word[i] >= 'a' && word[i] <= 'z') || 
              (word[i] >= 'A' && word[i] <= 'Z') ||
              (word[i] >= '0' && word[i] <= '9' && i>0) ||
              (word[i] == '_')))
        {
              //std::cout << "cause " << word[i] << std::endl;
              return false;
        }
    }
    return true;
}


char BinAsm::is_register(const std::string& word)
{
    if (!word.size()) return 0;
    std::string p = word;
	std::transform(p.begin(), p.end(), p.begin(), ::toupper);
    if (p=="A"||p=="B"||p=="C"||p=="X"||p=="Y"||p=="Z"
        ||p=="I"||p=="J")
        return p[0];
    return 0;
}

bool BinAsm::resolve_labels()
{
    std::string err;
    std::map<uint16_t,std::string>::iterator it;
    
    for (it=_unresolved.begin();it!=_unresolved.end();)
    {
        std::map<std::string,uint16_t>::iterator jt;
        bool ok=false;
        if (_unresolved.size()==0)
            break;
        for (jt=_labels.begin();jt!=_labels.end();jt++)
        {
            if (it->second == jt->first)
            {
                ok = true;
                printf("linker: resolve 0x%04X (%s)\n",it->first,it->second.c_str());
                _bin[it->first]-=0x100;
                _bin[it->first]+=jt->second;
                _unresolved.erase(it);
                it=_unresolved.begin();
                break;
            }
        }      
        if (!ok)
        {
            std::cerr <<"linker error: unresolved symbol \""<< it->second << "\"\n";
            it++;
        }
    }
    
    return _unresolved.size()==0;
}

bool BinAsm::requiert_data(uint8_t a_or_b)
{
    switch (a_or_b)
    {
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x1E:
        case 0x1F:
            return true;
        default:
            break;
    }
    return false;
}

}
