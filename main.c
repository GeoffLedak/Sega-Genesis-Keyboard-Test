#include <stdio.h>
#include <stdlib.h>

#include "hw_md.h"

void putChar(char character);
void advanceCursor();
void putNewline();
void WaitForVBlank();
void readControllers();
void readKeyboard();

short GetHandshakeNibblePort2( short* hshkState );

int FindESKeyboard(void);

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


    if ( FindESKeyboard() ) {

        put_str("Found ES Keyboard!", 0x0000, 2, 2);
    }



    // ReadESKeyboard();
    // WriteESKeyboard();
}



typedef unsigned char		UChar;
typedef unsigned long		ULong;

#define	kData2		0xA10005
#define kCtl2		0xA1000B
#define	kSerial2	0xA10019

#define kTH			0x40			// controller mode control line
#define kTR			0x20			// controller handshake request
#define kTL			0x10			// controller handshack acknowledge

#define nop			 __asm__ __volatile__ ("nop\n\t");



int
FindESKeyboard(void) {
    UChar readBuf[4];
    register UChar*		readScan = readBuf;
    volatile register UChar* reg = (UChar*) kData2;	// only support keyboard on PORT 2!
    short hshkState;
    register long timeout = 100;
    // register ULong kbID = REFGLOBAL( controls, keyboardID );
    register ULong kbID = 0xC030609;

    *(reg) = kTH + kTR;					// both flags hi
    *(char *)kSerial2 = 0;							// clear serial modes
    *(char *)kCtl2 = kTH + kTR;					// both flags are outputs now

    nop; nop;
    *readScan++ = *reg & 0x0F;							// 1st nybble = identifying ID

    *reg = kTR;											// turn off TH to start hshk seq

    do {
        *readScan = *reg & 0x0F;
    }
    while ( (*readScan != ((kbID >> 16) & 0xF)) && --timeout );		// 2nd nybble has no handshake

    if ( !timeout )
    {
        *reg = kTH + kTR;								// make sure we leave with TH & TR hi
        return ( 0 );
    }

    readScan++;

    hshkState = 0;										// start flipping TR
    *readScan++ = GetHandshakeNibblePort2(&hshkState);	// 3rd nybble = local ID
    *readScan = GetHandshakeNibblePort2(&hshkState);	// 4th nybble = local ID

    *reg |= kTH;										// abort the transaction

    for ( timeout = 0; timeout != 50; timeout++ );		// spin a bit

    *reg = kTH + kTR;									// make sure we leave with TH & TR hi

    if ( *(ULong *) readBuf == kbID )					// found a good Eric Smith Keyboard
        return ( 1 );
    else
        return ( 0 );
}




short
GetHandshakeNibblePort2( short* hshkState )
{
    register 			long		timeout = 100;
    volatile register	UChar*		reg = (UChar*) kData2;

    if (*hshkState == -1)	// timed out, abort (see below)
        return 0x0F;

    if ((*hshkState ^= 1) == 0)
    {
        *reg |= kTR;			// raise TR, wait for TL high
        nop; nop;
        do {}
        while (!(*reg & kTL) && --timeout);
        if (timeout)
        {
            nop; nop;
            return *reg & 0x0F;
        }
    }
    else
    {
        *reg &= ~kTR;			// lower TR, wait for TL low
        nop; nop;
        do {}
        while ((*reg & kTL) && --timeout);
        if (timeout)
        {
            nop; nop;
            return *reg & 0x0F;
        }
    }

    // if we got this far, we've timed out. return 0xFFs to abort.

    *hshkState = -1;
    return 0xFF;
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


