#ifndef _SPROCKET_H
#define _SPROCKET_H

char getMode(void);
void reverseFile(FILE * source, FILE * dest);
void bin2asm(char * input, FILE * source, FILE * dest);
void writeSprite(char * input, FILE * source, FILE * dest, int clear);
void log(char * text);		

#endif

