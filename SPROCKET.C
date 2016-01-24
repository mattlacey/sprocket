#include <stdio.h>

#include "sprocket.h"

#define DEBUG	0

int main(int argc, char ** argv)
{
	int i;
	char mode;
	char origin;
	char input[128];
	char outfile[128];
	char infile[128];
	FILE * source;
	FILE * dest;
	
	for(i = 0; i < 32; i++)
		printf("\n");
		
	printf("Welcome to Sprocket.\nWhat do you want to do?\n\n\t1. Reverse File\n\t2. Binary to ASM\n\t9. Quit\n\n");

	mode = getMode();

	if(mode == '9')
	{
		return 0;
	}

	printf("Enter the name of the file to process (.bin):\n");
	scanf("%s", input);
	sprintf(infile, "%s.bin", input);
	
	source = fopen(infile, "rb");
	
	if(!source)
	{
		printf("Could not open file: %s", input[1]);
		getchar();
		return 0;
	}

	if(mode == '1')
	{
		sprintf(outfile, "%s_r.bin", input);

		dest = fopen(outfile, "wb");

		if(!dest)
		{
			printf("Could not open file for output: %s", outfile);
			getchar();
			return 0;
		}
	}	
	else if(mode == '2')
	{
		printf("Enter the name of the file to create:\n");
		scanf("%s", outfile);
		
		dest = fopen(outfile, "wb");
	
		if(!dest)
		{
			printf("Could not open file for output: %s", outfile);
			getchar();
			return 0;
		}

		printf("Top left origin (1) or centered (2)?\n");
		origin = getMode();

	}
	
	if(mode == '1')
		reverseFile(source, dest);
	else if(mode == '2')
		bin2asm(input, source, dest, origin);
		
	fclose(source);
	fclose(dest);
	
	printf("\n\nDone. Hit enter to exit.\n");
	getchar();
	getchar();
	
	return 0;
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

void reverseFile(FILE * source, FILE * dest)
{
	long location;
	char byte;
	
	fseek(source, -1, SEEK_END);
	
	/* want to include the last byte! */
	location = 1 + ftell(source);
	
	while(location)
	{
		byte = fgetc(source);
		fputc(byte, dest);

		fseek(source, -2, SEEK_CUR);
		location--;
	}
}

void bin2asm(char * name, FILE * source, FILE * dest, char origin)
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
	int working = 1;
	int clearCount = 0;
	int count = 0;
	int totalCount = 0;
	int lineOffset = 0;
	int spriteCount = 0;
		
	/* skip the sprite mask 
	fseek(source, 16*16*2, SEEK_SET);*/
	
	if(clear)
	{
		fprintf(dest, "\t\tsection text\r\n%s%iclear:\r\n", name, spriteCount);
	}
	else
	{
		fprintf(dest, "\t\tsection text\r\n%s%i:\r\n", name, spriteCount);
	}
	 	
	while(working)
	{			
		working = fread(&pixel, 1, 2, source);
		count ++;
		totalCount++;
		
		if(!working)
		{
			break;
		}

		if(totalCount > 16*16)
		{
			fprintf(dest, "\t\trts\r\n\r\n");

			if(clear)
			{
				fprintf(dest, "\t\tsection text\r\n%s%iclear:\r\n", name, ++spriteCount);
			}
			else
			{
				fprintf(dest, "\t\tsection text\r\n%s%i:\r\n", name, ++spriteCount);
			}

			totalCount = 0;
			clearCount = 0;
			lineOffset = 0;
		}
		
		if(pixel)
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

			if(clear)
			{
				fprintf(dest, "\t\tmove.w\t(a1)+,(a0)+\r\n");			
			}
			else
			{
				fprintf(dest, "\t\tmove.w\t#$%04X,(a0)+\r\n", pixel);
			}
		}
		else
		{
			clearCount ++;
		}
			
		if(count == 16)
		{
			count = 0;
			lineOffset ++;
			
			if(DEBUG)
				fprintf(dest, "; set offset\r\n");
		}
	}
	
	fprintf(dest, "\t\trts\r\n\r\n");
}
