#include "binasm.hpp"
#include <iostream>

namespace cpu {

BinAsm::BinAsm()
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
	
	
	FILE* f = fopen(filename.c_str(),"r");
	if (!f)
	{
		std::cerr << "error : " << filename << " not found !" << std::endl;
	    return false;
	}

	unsigned int size = fsize(f);
	char* buff = new char[size+1];
	fread(buff,1,size,f);
	buff[size] = '\0';
	fclose(f);
	
	_fullsrc = buff;
	_src=_fullsrc;
	remove_comments(_src);
	
	delete buff;
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
	lines.reserve(text.size()/8);
	unsigned int b=0;
	for (unsigned int i=0; i<text.size();i++)
	{
		if (text[i] == '\n' && i-b <= 1)
		{
			b = i+1;
		}
		else if (text[i] == '\n') 
		{
			lines.push_back(text.substr(b,i-b));
			b=i+1;
		}
	}
	if (text.size()-b > 1)
		lines.push_back(text.substr(b,lines.size()));
	return lines;
}

std::vector<std::string> BinAsm::split_line(const std::string& line)
{
	std::vector<std::string>  words;
	words.reserve(line.size()/3);
	unsigned int b=0;
	bool validate=false;
	char special_case=0;
	for (unsigned int i=0; i<line.size();i++)
	{
	    if (!special_case && !validate && line[i] != ' ' 
		  && line[i] != ',' && line[i] != '\t')
		{
			b=i;
			validate=true;
			if ((line[i] == '\'' || line[i] == '"') && 
			   (!i || line[i-1] != '\\'))
			  special_case = line[i];
			else if (line[i] == '[')
			  special_case = ']';
		}
		else if (special_case == line[i])
		{
			words.push_back(line.substr(b,i-b+1));
			validate=false;
			special_case=0;
		}
		else if (!special_case && validate &&
		        (line[i] == ' ' || line[i] == ',' || line[i] == '\t'))
		{
			words.push_back(line.substr(b,i-b));
			validate=false;
		}
	}
	if (validate) words.push_back(line.substr(b,line.size()));
	return words;
}

uint16_t BinAsm::get_value(const std::string& word, std::string& err)
{
	unsigned int s = word.size();
    if (!s) return 0;
	/*bool zero = true;
	for (int j=0; j < word.size();j++)
	{
		if (word[j] != '0')
		{
			zero=false;
			break;
		}
	}
	if (zero) return 0;*/
	if (word[0]== '\'' && s < 3)
	{
		err = "invalid symbol value " +  word;
		return 0;
	}
	if (word[0]== '\'' && s >= 3)
	{
		if (word[1] == '\\' && s >= 4)
		{
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
					return 0;
			}
		}
		else
			return word[1];
	}
	char* c = const_cast<char*>(word.c_str());
	int number=strtol(word.c_str(),&c,0);
	if (c==word.c_str())
	{
		if (_labels.find(word)!=_labels.end())
		{
			return _labels[word].offset;
		}
		err = "label " +  word + " not defined";
		return 0;
	}
	if (number > 0xFFFF || number < -(0xFFFF))
	{
		err = "value " +  word + " overflow (max 0xFFFF)";
		return 0;
	}
	uint16_t v=number;
	if (number < 0)
	{
	    int16_t n = number;
		v = *(uint16_t*)(&n); //convert signed value into unsigned
	}
	return v;
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

uint16_t BinAsm::get_data(const std::string& line, uint16_t* target, 
								std::string& err)
{
	std::vector<std::string> datas = split_line(line);
	
	for (std::vector<std::string>::iterator k=datas.begin();
				k!=datas.end(); k++)
	{
		std::string p = *k;
		std::transform(p.begin(), p.end(), p.begin(), ::tolower);
		if (p == ".dat" || p == "dat")
		{
			uint16_t size = 0;
			for (std::vector<std::string>::iterator i=k+1;
					i!=datas.end(); i++)
			{
				if (i->size() > 0 && (*i)[0] == '"')
				{
					for (unsigned int j=0; j < i->size()-1;j++)
					{
						if ((*i)[j-1] == '\\')
						{
							switch ((*i)[j+1])
							{
								case 'n':
									target[size] = '\n';
								case 't':
									target[size] = '\t';
								case '\\':
									target[size] = '\\';
								case 'r':
									target[size] = '\r';
								default:
									err = "invalid special symbol value "+(*i);
									return 0;
							}
							
						}
					    else if ((*i)[j] == '\\')
							continue;
						else	
							target[size] = (*i)[j];
							
						size++;
					}
				}
				else
				{
					target[size] = get_value(*i, err);
					size++;
				}
			}
			return size;
		}
	}
	return 0;
}

uint8_t BinAsm::get_op(const std::string& word)
{
	std::string p = word;
	std::transform(p.begin(), p.end(), p.begin(), ::toupper);
	size_t i = p.find(' ');
	while (i != std::string::npos)
	{
		p.erase(i,1);
		i = p.find(' ');
	}
	 if (p=="SET") return 0x01;
	 else if (p=="ADD") return 0x02;
	 else if (p=="SUB") return 0x03;
	 else if (p=="MUL") return 0x04;
	 else if (p=="MLI") return 0x05;
	 else if (p=="DIV") return 0x06;
	 else if (p=="DVI") return 0x07;
	 else if (p=="MOD") return 0x08;
	 else if (p=="MDI") return 0x09;
	 else if (p=="AND") return 0x0A;
	 else if (p=="BOR") return 0x0B;
	 else if (p=="XOR") return 0x0C;
	 else if (p=="SHR") return 0x0D;
	 else if (p=="ASR") return 0x0E;   
	 else if (p=="SHL") return 0x0F;
	 else if (p=="IFB") return 0x10;
	 else if (p=="IFC") return 0x11;
	 else if (p=="IFE") return 0x12;
	 else if (p=="IFN") return 0x13;
	 else if (p=="IFG") return 0x14;
	 else if (p=="IFA") return 0x15;
	 else if (p=="IFL") return 0x16;
	 else if (p=="IFU") return 0x17;
	 else if (p=="ADX") return 0x1A;
	 else if (p=="SBX") return 0x1B;
	 else if (p=="STI") return 0x1E;
	 else if (p=="STD") return 0x1F;
	 else
	 {
		return 0;
	 }
}

uint8_t BinAsm::get_sop(const std::string& word, std::string& err)
{
	std::string p = word;
	std::transform(p.begin(), p.end(), p.begin(), ::toupper);
	size_t i = p.find(' ');
	while (i != std::string::npos)
	{
		p.erase(i,1);
		i = p.find(' ');
	}
	if (p=="JSR") return 0x01;
	else if (p=="HCF") return 0x07;
	else if (p=="INT") return 0X08;
	else if (p=="IAG") return 0x09;
	else if (p=="IAS") return 0x0A;
	else if (p=="RFI") return 0x0B;
	else if (p=="IAQ") return 0x0C;
	else if (p=="HWN") return 0X10;
	else if (p=="HWQ") return 0x11;
	else if (p=="HWI") return 0x12;
	else
	{
		err = "unknow instruction " + word; 
		return 0;
	}
}

uint8_t BinAsm::get_a(const std::string& word,
							uint16_t& data, std::string& err)
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
	if (p=="A") return 0x00;
    else if (p=="B") return 0x01;
	else if (p=="C") return 0x02;
	else if (p=="X") return 0x03;
	else if (p=="Y") return 0x04;
	else if (p=="Z") return 0x05;
	else if (p=="I") return 0x06;
	else if (p=="J") return 0x07;
	
	else if (p=="[A]") return 0x08;
	else if (p=="[B]") return 0x09;
	else if (p=="[C]") return 0x0A;
	else if (p=="[X]") return 0x0B;
	else if (p=="[Y]") return 0x0C;
	else if (p=="[Z]") return 0x0D;
	else if (p=="[I]") return 0x0E;
	else if (p=="[J]") return 0x0F;
	
	else if (p.find("[A+")!=std::string::npos) code=0x10;
	else if (p.find("[B+")!=std::string::npos) code=0x11;
	else if (p.find("[C+")!=std::string::npos) code=0x12;
	else if (p.find("[X+")!=std::string::npos) code=0x13;
	else if (p.find("[Y+")!=std::string::npos) code=0x14;
	else if (p.find("[Z+")!=std::string::npos) code=0x15;
	else if (p.find("[I+")!=std::string::npos) code=0x16;
	else if (p.find("[J+")!=std::string::npos) code=0x17;
	
	
	else if (p=="POP"||p=="[SP++]") return 0x18;
	else if (p=="PUSH"||p=="[--SP]") 
	{
		err="cannot push ([--SP]) on avalue";
		return 0x18;
	}
	else if (p=="PEEK"||p=="[SP]") return 0x19;
	else if (p=="PICK") return 0x1A;
	
	
	else if (p=="SP") return 0x1B;
	else if (p=="PC") return 0x1C;
	else if (p=="EX") return 0x1D;
	
	
	if (code && u.size() > 5)
	{
		data=get_value(u.substr(3,u.size()-4), err);
		return code;
	}
	else if (u.size() > 2 && u[0] == '[')
	{
		if (u[u.size()-1] == ']') 
			data=get_value(u.substr(1,u.size()-2), err);
		else
			err="excepted ']' at the end of avalue pointer";
		return 0x1E; 
	}
	uint16_t value=get_value(u, err);
	if (!err.size())
	{
		if (value == 0xFFFF) return 0x20;
		else if (value<=30)
		{
			return value+0x21;
		}
		else
		{
			data=value;
			return 0x1F;
		}
	}
	else
	{
		return 0xFF;
	}
}

uint8_t BinAsm::get_b(const std::string& word,
							uint16_t& data, std::string& err)
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
	if (p=="A") return 0x00;
    else if (p=="B") return 0x01;
	else if (p=="C") return 0x02;
	else if (p=="X") return 0x03;
	else if (p=="Y") return 0x04;
	else if (p=="Z") return 0x05;
	else if (p=="I") return 0x06;
	else if (p=="J") return 0x07;
	
	else if (p=="[A]") return 0x08;
	else if (p=="[B]") return 0x09;
	else if (p=="[C]") return 0x0A;
	else if (p=="[X]") return 0x0B;
	else if (p=="[Y]") return 0x0C;
	else if (p=="[Z]") return 0x0D;
	else if (p=="[I]") return 0x0E;
	else if (p=="[J]") return 0x0F;
	
	else if (p.find("[A+")!=std::string::npos) code=0x10;
	else if (p.find("[B+")!=std::string::npos) code=0x11;
	else if (p.find("[C+")!=std::string::npos) code=0x12;
	else if (p.find("[X+")!=std::string::npos) code=0x13;
	else if (p.find("[Y+")!=std::string::npos) code=0x14;
	else if (p.find("[Z+")!=std::string::npos) code=0x15;
	else if (p.find("[I+")!=std::string::npos) code=0x16;
	else if (p.find("[J+")!=std::string::npos) code=0x17;
	
	
	else if (p=="PUSH"||p=="[--SP]") return 0x18;
	else if (p=="POP"||p=="[SP++]") 
	{
		err="cannot pop ([SP++]) on btarget";
		return 0x18;
	}
	else if (p=="PEEK"||p=="[SP]") return 0x19;
	else if (p=="PICK") return 0x1A;
	
	
	else if (p=="SP") return 0x1B;
	else if (p=="PC") return 0x1C;
	else if (p=="EX") return 0x1D;
	
	
	if (code && u.size() > 5)
	{
		data=get_value(u.substr(3,u.size()-4), err);
		return code;
	}
	else if (u.size() > 2 && u[0] == '[')
	{
		if (u[u.size()-1] == ']') 
			data=get_value(u.substr(1,u.size()-2), err);
		else
			err="excepted ']' at the end of btarget pointer";
		return 0x1E; 
	}
	uint16_t value=get_value(u, err);
	if (!err.size())
	{
		err="btarget must be a pointer or a register";
		if (value == 0xFFFF) return 0x20;
		else if (value<=30)
		{
			return value+0x21;
		}
		else
		{
			data=value;
			return 0x1F;
		}
	}
	else
	{
		return 0xFF;
	}
}

bool BinAsm::finds_labels()
{
	unsigned int lc = 0;
	unsigned int errc = 0;
	unsigned int offset = 0;
	uint16_t opd = 0;
	std::string err = "";
	std::vector<std::string> lines=split_text(_src);
	std::vector<std::string>::iterator lit = lines.begin();
	for (;lit!=lines.end();lit++)
	{
	    lc++;
		std::vector<std::string> w=split_line(*lit);
		if (!w.size()) continue;
		if (get_op(w[0]))
		{
			offset++;
			if (w.size() < 3) continue;
			opd=0;
			get_b(w[1],opd,err);
			if (opd) offset++;
			opd=0;
			get_a(w[2],opd,err);
			if (opd) offset++;
			
		}
		else if (get_sop(w[0],err) && !err.size())
		{
			offset++;
			if (w.size() < 2) continue;
			opd=0;
			get_a(w[1],opd,err);
			if (opd) offset++;
		}
		else
		{
			std::vector<std::string>::iterator wit = w.begin();
			Label l;
			l.line = lc;
			l.offset = offset;
			l.name = std::string();
			for (;wit!=w.end();wit++)
			{
				if (wit->size() < 2) continue;
				if ((*wit)[wit->size()-1] == ':')
				{
					l.name = wit->substr(0,wit->size()-1);
					break;
				}
				else if ((*wit)[0] == ':')
				{
					l.name = wit->substr(1,wit->size());
					break;
				}
			}
			if (_labels.find(l.name)!=_labels.end())
			{
				err="label " + l.name + " redefined";
				print_error(lc, false,err);
				errc++;
			}
			else if (l.name.size())
			{
				_labels[l.name]=l;
			}
		}
		uint16_t* data = new uint16_t[lit->size()]; 
		offset += get_data(*lit, data,err);
		delete data;
	}
	
	return !errc;
}

void BinAsm::print_labels()
{
	for (std::map<std::string,Label>::iterator i = _labels.begin();
		i != _labels.end(); i++)
	{
		std::cout << i->first<< " at offset "<< i->second.offset<<'\n';
	}
}

bool BinAsm::save(const std::string& filename)
{
	uint32_t allocsize=0x10000;
	uint32_t filesize=0;
	int error_count=0;
	uint16_t* buff = new uint16_t[allocsize];
	unsigned int lc = 0;
	uint16_t opcode = 0;
	std::vector<std::string> lines=split_text(_src);
	std::vector<std::string>::iterator lit = lines.begin();
	for (;lit!=lines.end();lit++)
	{
	    lc++;
		std::string err = std::string();
		std::vector<std::string> w=split_line(*lit);
		if (!w.size()) continue;
		opcode = 0;
		
		if (filesize  > allocsize - lit->size() - 3)
		{
			std::cerr << "fatal error: assembling file is too big";
			std::cerr << "cannot be used on dcpu-16" << std::endl;
			break;
		}
		
		if (w[0].size() && (w[0][w[0].size()-1] == ':' || w[0][0] == ':'))
		{
			continue;
		}
		
		if ((opcode = get_op(w[0])))
		{
			if (w.size() != 3)
			{
				err = "instruction " + w[0];
				err += " need 2 arguments";
			}
			else
			{
				uint16_t a_word=0, b_word=0;
				opcode |= ((get_b(w[1],b_word,err) & 0x1F) << 5);
				if (err.size())
				{
					print_error(lc,false,err);
					error_count++;
				}
				opcode |= (get_a(w[2],a_word,err) << 10);
				buff[filesize]=opcode;
				filesize++;
				if (a_word)
				{
					buff[filesize]=a_word;
					filesize++;
				}
				if (b_word)
				{
					buff[filesize]=b_word;
					filesize++;
			    }
			}
		}
		else if ((opcode=get_sop(w[0],err)) && !err.size())
		{
			
			if (w.size() != 2)
			{
				err = "special instruction " + w[0];
				err += " need 1 argument";
			}
			else
			{
				uint16_t a_word=0;
				opcode = ((opcode & 0x1F) << 5);
				opcode |= (get_a(w[1],a_word,err) << 10);
				buff[filesize]=opcode;
				filesize++;
				if (a_word)
				{
					buff[filesize]=a_word;
					filesize++;
				}
			}
		} 
		else {
			if (w[0]=="dat"||w[0]==".dat"||w[0]=="DAT"||w[0]==".DAT")
				err="";
			filesize += get_data(*lit, &(buff[filesize]),err);
		}
		if (err.size())
		{
			print_error(lc,false,err);
			error_count++;
		}
		
	}
	
	
	
	if (error_count)
	{
		std::cerr << "assembling terminated with " << error_count; 
		std::cerr << " error(s)" << std::endl;
        
	}
	else
	{
		FILE* f = fopen(filename.c_str(), "wb");
		if (!f) {
            std::cerr << "error: cannot open output file " << filename << '\n';
        }
        else
        {
            fswitchendian(buff, filesize);
            fwrite(buff,2,filesize,f);
            fclose(f);
            std::cout << "assembling " << filename;
            std::cout << " terminated final size " << filesize*2;
            std::cout << " bytes" << std::endl;
        }
	}
	delete buff;
    return !error_count;
}

}