#include <stdio.h>

#include "sprocket.h"

#define DEBUG	0

int main(int argc, char ** argv)
{
	int i;
	int lineLength;
	int width;
	char mode;
	char origin = 0;
	char input[128];
	char outfile[128];
	char infile[128];
	FILE * source;
	FILE * dest;
	
	for(i = 0; i < 25; i++)
		printf("\n");

	printf("Welcome to Sprocket\n-------------------\n\n");

	while(1)
	{		
		printf("What do you want to do?\n\n\t1. Bitmap to Binary\n\t2. Binary to ASM (Sprites)\n\t3. Binary to ASM (BG)\n\t9. Quit\n\n");

		mode = getMode();

		if(mode == '9')
		{
			break;
		}

		if(mode == '1')
		{
			printf("Enter the name of the file to convert (.bmp):\n");
			scanf("%s", input);
			sprintf(infile, "%s.bmp", input);
		}
		else
		{
			printf("Enter the name of the file to process (.bin):\n");
			scanf("%s", input);
			sprintf(infile, "%s.bin", input);
		}
		
		source = fopen(infile, "rb");
		
		if(!source)
		{
			printf("Could not open file: %s", input[1]);
			getchar();
			return 0;
		}

		if(mode == '1')
		{
			printf("Enter image width in bytes:\n");
			scanf("%i", &width);

			sprintf(outfile, "%s.bin", input);

			if(!openFile(&dest, outfile))
			{
				return 0;
			}
		}	
		else if(mode == '2' || mode == '3' || mode == '4')
		{
			printf("Enter the name of the file to create:\n");
			scanf("%s", outfile);
			
			if(!openFile(&dest, outfile))
			{
				return 0;
			}

			/*
			if(mode == '2')
			{
				printf("Top left origin (1) or centered (2)?\n");
				origin = getMode();
			*/
		}

		
		if(mode == '1')
			bmp2bin(source, dest, width);
		else if(mode == '2')
			bin2asmSprite(input, source, dest, origin);
		else if(mode == '3')
			bin2asmBG(input, source, dest);
			
		fclose(source);
		fclose(dest);
	}
	
	return 0;
}

int openFile(FILE ** ppFile, char * filename)
{
	*ppFile = fopen(filename, "wb");
		
	if(!*ppFile)
	{
		printf("Could not open file for output: %s", filename);
		getchar();
		return 0;
	}

	return 1;
}

char getMode(void)
{
	char mode = 0;
	
	while(mode < '1' || mode > '9')
	{
		mode = getchar();
	}

	return mode;
}

void bmp2bin(FILE * source, FILE * dest, int width)
{
	long lines;
	char bytes[2];
	
	fseek(source, 0, SEEK_END);
	
	/* want to include the last byte! */
	lines = (ftell(source) - 138) / width;


	/* Header is 138 bytes, want that to be cut off */
	while(lines)
	{
		fseek(source, - width, SEEK_CUR);

		int j = width;

		while(j)
		{
			bytes[0] = fgetc(source);
			bytes[1] = fgetc(source);
			fputc(bytes[1], dest);
			fputc(bytes[0], dest);
			j -= 2;
		}

		lines --;
		fseek(source, - width, SEEK_CUR);
	}
}

void bin2asmSprite(char * name, FILE * source, FILE * dest, char origin)
{
	printf("Writing sprite routine...\n");
 	writeSprite(name, source, dest, origin, 0);
 	
 	printf("Writing sprite clear routine...\n");
 	fseek(source, 0, SEEK_SET);
 	writeSprite(name, source, dest, origin, 1);
}

void writeSprite(char * name, FILE * source, FILE * dest, char origin, int clear)
{
	unsigned short int pixel = 0;
	unsigned short int pixel2 = 0;

	int working = 1;
	int clearCount = 0;
	int count = 0;
	int totalCount = 0;
	int lineOffset = 0;
	int spriteCount = 0;
		
	/* skip the sprite mask 
	fseek(source, 16*16*2, SEEK_SET);*/
	
	while(working)
	{
		working = fread(&pixel, 1, 2, source);
		working |= fread(&pixel2, 1, 2, source);

		if(!working)
		{
			break;
		}

		if(!totalCount)
		{	
			if(clear)
			{
				fprintf(dest, "\t\tsection text\r\n%s%iclear:\r\n", name, spriteCount++);
			}
			else
			{
				fprintf(dest, "\t\tsection text\r\n%s%i:\r\n", name, spriteCount++);
			}
		}

		count += 2;
		totalCount += 2;
		
		
		// if the first pixel is blank we need to allow for that
		if(!pixel && pixel2)
		{
			clearCount++;
		}

		// if there's a value in one and we were previously processing blank pixels then 
		// offset the memory address accordingly
		if(pixel || pixel2)
		{
			if(clearCount)
			{
				fprintf(dest, "\t\tadda.l\t#%i+(scr_w-16)*%i,a0\r\n", (clearCount * 2), (lineOffset * 2));
				
				if(clear)
				{
					fprintf(dest, "\t\tadda.l\t#%i+(scr_w-16)*%i,a1\r\n", (clearCount * 2), (lineOffset * 2));
				}
				
				clearCount = 0;
				lineOffset = 0;
				
				if(DEBUG)				
					fprintf(dest, "; reset clear and offset\r\n");
			}
		
			if(lineOffset)
			{
				fprintf(dest, "\t\tadda.l\t#(scr_w-16)*%i,a0\r\n", lineOffset * 2);
				
				if(clear)
				{
					fprintf(dest, "\t\tadda.l\t#(scr_w-16)*%i,a1\r\n", lineOffset * 2);
				}
				
				lineOffset = 0;
				
				if(DEBUG)
					fprintf(dest, "; reset offset\r\n");
			}
		}

		if(pixel && pixel2)
		{
			if(clear)
			{
				fprintf(dest, "\t\tmove.l\t(a1)+,(a0)+\r\n");			
			}
			else
			{
				fprintf(dest, "\t\tmove.l\t#$%04X%04X,(a0)+\r\n", pixel, pixel2);
			}
		}
		else if(pixel || pixel2)
		{
			if(clear)
			{
				fprintf(dest, "\t\tmove.w\t(a1)+,(a0)+\r\n");			
			}
			else
			{
				fprintf(dest, "\t\tmove.w\t#$%04X,(a0)+\r\n", pixel | pixel2);
			}

			if(pixel)
			{
				clearCount ++;
			}
		}
		else
		{
			clearCount += 2;
		}
			
		if(count == 16)
		{
			count = 0;
			lineOffset ++;
		
			if(DEBUG)
				fprintf(dest, "; set offset\r\n");
		}

		if(totalCount == 16*16)
		{
			fprintf(dest, "\t\trts\r\n\r\n");

			totalCount = 0;
			clearCount = 0;
			lineOffset = 0;
			count = 0;
		}
	}

	fprintf(dest, "%s%s_map:\tdc.l\t", name, clear ? "clear" : "");

	for(count = 0; count < spriteCount; count++)
	{
		fprintf(dest, "%s%i", name, count);
		fprintf(dest, count == spriteCount - 1 ? "\r\n\r\n" : ",");
	}
}

void bin2asmBG(char * name, FILE * source, FILE * dest)
{
	unsigned int pixels = 0;

	int working = 1;
	
	fprintf(dest, "\t\tsection text\r\nBG_%s:\r\n", name);
		 	
	while(working)
	{			
		working = fread(&pixels, 1, 4, source);

		if(!working)
		{
			break;
		}
		
		fprintf(dest, "\t\tmove.l\t#$%08X,(a0)+\r\n", pixels);
	}
	
	fprintf(dest, "\t\trts\r\n\r\n");
}
