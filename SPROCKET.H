#ifndef _SPROCKET_H
#define _SPROCKET_H

char getMode(void);
void bmp2bin(FILE * source, FILE * dest, int width);
void bin2asmSprite(char * input, FILE * source, FILE * dest, char origin);
void writeSprite(char * input, FILE * source, FILE * dest, char origin, int clear);
void bin2asmBG(char * input, FILE * source, FILE * dest);
// void log(char * text);

#endif

