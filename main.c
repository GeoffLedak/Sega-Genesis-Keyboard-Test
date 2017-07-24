#include <stdio.h>
#include <stdlib.h>

#include "hw_md.h"

void putChar(char character);
void advanceCursor();
void putNewline();
void WaitForVBlank();
void readControllers();
void readKeyboard();

char xPosition = 4;
char yPosition = 0;

char aButtonPressed = 0;
char bButtonPressed = 0;
char cButtonPressed = 0;
char startButtonPressed = 0;


int main(void)
{

	
	// put_str("A = A, B = B, C = C, Start = newline", 0x0000, 0, 0);
	put_str("C:\\>", 0x0000, 0, 0);

    while ( 1 ) // endless loop
    {
        readControllers();

        readKeyboard();
		

		

		
		WaitForVBlank();
    }
	
	
	
    return 0;
}



void readControllers() {

    short buttons = 0;
    buttons = get_pad(0);

    // check A button
    if( buttons & SEGA_CTRL_A )
    {
        if( !aButtonPressed ) {
            putChar('A');
            aButtonPressed = 1;
        }
    }
    else
    {
        aButtonPressed = 0;
    }

    // check B button
    if ( buttons & SEGA_CTRL_B )
    {
        if( !bButtonPressed ) {
            putChar('B');
            bButtonPressed = 1;
        }
    }
    else
    {
        bButtonPressed = 0;
    }

    // check C button
    if ( buttons & SEGA_CTRL_C )
    {
        if( !cButtonPressed ) {
            putChar('C');
            cButtonPressed = 1;
        }
    }
    else
    {
        cButtonPressed = 0;
    }

    // check Start button
    if ( buttons & SEGA_CTRL_START )
    {
        if( !startButtonPressed ) {
            putNewline();
            startButtonPressed = 1;
        }
    }
    else
    {
        startButtonPressed = 0;
    }
}



void readKeyboard() {



}




void WaitForVBlank() {
	
	
	volatile unsigned short *pw;

    pw = (volatile unsigned short *) 0xC00004;

    while (*pw & (1 << 3));
	while (!(*pw & (1 << 3)));
	
}


void advanceCursor()
{
	xPosition++;

	if( xPosition > 39 )
	{
		xPosition = 0;
		yPosition++;
		
		if( yPosition > 27 )
		{
			yPosition = 0;
		}
	}
}

void putChar(char character)
{
	put_chr(character, 0x0000, xPosition, yPosition);
	advanceCursor();
}

void putNewline()
{
	xPosition = 0;
	yPosition++;
	
	if( yPosition > 27 )
	{
		yPosition = 0;
	}
}


