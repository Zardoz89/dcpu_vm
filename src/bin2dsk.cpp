#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "file.h"
#include <dcpu/devices/m35fd.hpp>
#include <log.hpp>

#define PARTITION_HEADER_SIZE 4

int main(int argc, char** argv)
{
  if (argc < 3)
  {
    LOG_ERROR << "no input arguments";
    LOG << std::string("Usage: ") + std::string(argv[0]) + " input output";
    return 0xdead;
  }
  FILE* in = fopen(argv[1],"rb");
  if (!in)
  {
    LOG_ERROR << std::string(argv[1]) + " : cannot open the file !";
  }
  int in_size = fsize(in);
  char* buffer_in = (char*) malloc(in_size);
  fread(buffer_in,1,in_size,in);
  fclose(in);
  cpu::m35fd::M35_Floppy floppy(argv[2]);
  
  
  /*int rest = ;
  if (rest <= 0)
  {
    std::cerr << argv[1] << " : file is too big !" << std::endl;
  }
  
  FILE* out = fopen(argv[2],"wb");
  if (!out)
  {
    std::cerr << argv[2] << " : cannot open the file !" << std::endl;
    return 0xdead;
  }
  else
  {
    /// File Header 
    const char magic1 = 'F', magic2 = 1;
    const unsigned char bootable_sign = 0xFE; //FloppyExecutable
    const char nb_tracks = 80;
    fwrite(&magic1,1,1,out);
    fwrite(&magic2,1,1,out);
    fwrite(&bootable_sign,1,1,out);
    fwrite(&nb_tracks,1,1,out);
    
    /// write pure datas 
    fwrite(buffer_in,1,in_size,out);
    if (rest > 0) //Fill the rest with 0
    { 
      char* zeros = (char*) malloc(rest);
      memset(zeros,0,rest);
      fwrite(zeros,1,rest,out);
      free(zeros);
    }
    
    /// write bad sectors 
    unsigned char* bad_sectors = (unsigned char*) malloc(80*512*18/8);
    int end_good = rest/8;
    memset(bad_sectors,0x00,end_good);
    if (rest%8)
    {
      bad_sectors[end_good] = 0xFF << (8 - rest%8);
      end_good++;
    }
    memset(&(bad_sectors[end_good]),0xFF,80*512*18/8 - end_good);
    fwrite(bad_sectors,1,80*512*18/8,out);
    free(bad_sectors);
    
    /// finnish 
    fclose(out);
  }
  free(buffer_in);*/
  return 0;
}