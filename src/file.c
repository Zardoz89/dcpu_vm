#include <file.h>


int fsize(FILE *fp){
    int prev=ftell(fp);
	int sz;
    fseek(fp, 0L, SEEK_END);
    sz=ftell(fp);
    fseek(fp,prev,SEEK_SET); //go back to where we were
    return sz;
}

void fwritetext(const char* filename, const char* text)
{
   FILE* f = fopen(filename,"w");
   if (!f) return;
   fprintf(f,"%s",text);
   fclose(f);
}

void fswitchendian(uint16_t* f, unsigned int size)
{
	unsigned int i;
	for (i = 0; i < size; i++)
	{
		const uint8_t up =  f[i] & 0xFF;
		const uint8_t down =  f[i] >> 8;
		f[i] = (up << 8) | down;
	}
}
