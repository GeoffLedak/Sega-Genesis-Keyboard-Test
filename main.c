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
void PutHandshakeNibblePort2( short* hshkState, unsigned char byteToSend );

int FindESKeyboard( void );
void ReadESKeyboard ( void );
void WriteESKeyboard ( void );

void _vint_callback();

char xPosition = 4;
char yPosition = 0;

char aButtonPressed = 0;
char bButtonPressed = 0;
char cButtonPressed = 0;
char startButtonPressed = 0;

char capslockStatus = 0;
char capslockBytesSent = 1;

char capslockDataNotRegister = 0;




const unsigned int scancodeToAscii[] =
{
    0,     // 00, unused
    0,     // 01, F9
    0,     // 02, unused
    0,     // 03, F5
    0,     // 04, F3
    0,     // 05, F1
    0,     // 06, F2
    0,     // 07, F12
    0,     // 08, unused
    0,     // 09, F10
    0,     // 0A, F8
    0,     // 0B, F6
    0,     // 0C, F4
    '\t',  // 0D, TAB
    0x60,  // 0E, `
    0,     // 0F, unused
    0,     // 10, unused
    0,     // 11, L ALT
    0,     // 12, L SHFT
    0,     // 13, unused
    0,     // 14, L CTRL
    'q',   // 15, q
    '1',   // 16, 1
    0,     // 17, unused
    0,     // 18, unused
    0,     // 19, unused
    'z',   // 1A, z
    's',   // 1B, s
    'a',   // 1C, a
    'w',   // 1D, w
    '2',   // 1E, 2
    0,     // 1F, unused
    0,     // 20, unused
    'c',   // 21, c
    'x',   // 22, x
    'd',   // 23, d
    'e',   // 24, 
    '4',   // 25, 
    '3',   // 26, 
    0,     // 27, unused
    0,     // 28, unused
    ' ',   // 29, space
    'v',   // 2A, v
    'f',   // 2B, f
    't',   // 2C, t
    'r',   // 2D, r
    '5',   // 2E, 5
    0,     // 2F, unused
    0,     // 30, unused
    'n',   // 31, n
    'b',   // 32, b
    'h',   // 33, h
    'g',   // 34, g
    'y',   // 35, y
    '6',   // 36, 6
    0,     // 37, unused
    0,     // 38, unused
    0,     // 39, unused
    'm',   // 3A, m
    'j',   // 3B, j
    'u',   // 3C, u
    '7',   // 3D, 7
    '8',   // 3E, 8
    0,     // 3F, unused
    0,     // 40, unused
    ',',   // 41, ,
    'k',   // 42, k
    'i',   // 43, i
    'o',   // 44, o
    '0',   // 45, 0
    '9',   // 46, 9
    0,     // 47, unused
    0,     // 48, unused
    '.',   // 49, .
    '/',   // 4A, /
    'l',   // 4B, l
    ';',   // 4C, ;
    'p',   // 4D, p
    '-',   // 4E, -
    0,     // 4F, unused
    0,     // 50, unused
    0,     // 51, unused
    '\'',  // 52, '
    0,     // 53, unused
    '[',   // 54, [
    '=',   // 55, =
    0,     // 56, unused
    0,     // 57, unused
    0,     // 58, caps capslock
    0,     // 59, R SHFT
    0,     // 5A, ENTER
    ']',   // 5B, ]
    0,     // 5C, unused
    '\\',  // 5D, \ backslash
    0,     // 5E, unused
    0      // 5F, unused

    // keypad 1 starts at 69, goes up into 70?
    // esc is 76
    // numlock is 77
    // F11 is 78
    // scroll lock is 7E

    // 83 is F7

    // 83 is the largest value

};

unsigned int scancodeTableSize = sizeof(scancodeToAscii) / sizeof(unsigned int);





void flipCapslock() {

    if( !capslockStatus )
        capslockStatus = 1;
    else
        capslockStatus = 0;

    capslockDataNotRegister = 0;
    capslockBytesSent = 0;
}



int main(void)
{
    // 0x0000 = grey
    // 0x2000 = green
    // 0x4000 = red

	// put_str("A = A, B = B, C = C, Start = newline", 0x0000, 0, 0);
	put_str("C:\\>", 0x2000, 0, 0);

    while ( 1 ) // endless loop
    {


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
            flipCapslock();
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

        ReadESKeyboard();
        WriteESKeyboard();
    }
}



typedef unsigned char       UChar;
typedef unsigned long       ULong;

#define kData2      0xA10005
#define kCtl2       0xA1000B
#define kSerial2    0xA10019

#define kTH         0x40            // controller mode control line
#define kTR         0x20            // controller handshake request
#define kTL         0x10            // controller handshack acknowledge

#define kDataLines  0x0F            // data lines for 3-line hshk communication

#define nop          __asm__ __volatile__ ("nop\n\t");



int FindESKeyboard(void) {
    UChar readBuf[4];
    register UChar*     readScan = readBuf;
    volatile register UChar* reg = (UChar*) kData2;     // only support keyboard on PORT 2!
    short hshkState;
    register long timeout = 100;
    register ULong kbID = 0xC030609;

    *(reg) = kTH + kTR;                                 // both flags hi
    *(char *)kSerial2 = 0;                              // clear serial modes
    *(char *)kCtl2 = kTH + kTR;                         // both flags are outputs now

    nop; nop;
    *readScan++ = *reg & 0x0F;                          // 1st nybble = identifying ID

    *reg = kTR;                                         // turn off TH to start hshk seq

    do {
        *readScan = *reg & 0x0F;
    }
    while ( (*readScan != ((kbID >> 16) & 0xF)) && --timeout );     // 2nd nybble has no handshake

    if ( !timeout )
    {
        *reg = kTH + kTR;                               // make sure we leave with TH & TR hi
        return ( 0 );
    }

    readScan++;

    hshkState = 0;                                      // start flipping TR
    *readScan++ = GetHandshakeNibblePort2(&hshkState);  // 3rd nybble = local ID
    *readScan = GetHandshakeNibblePort2(&hshkState);    // 4th nybble = local ID

    *reg |= kTH;                                        // abort the transaction

    for ( timeout = 0; timeout != 50; timeout++ );      // spin a bit

    *reg = kTH + kTR;                                   // make sure we leave with TH & TR hi

    if ( *(ULong *) readBuf == kbID )                   // found a good Eric Smith Keyboard
        return ( 1 );
    else
        return ( 0 );
}





#define kESKeycodeData          0
#define kESControllerEcho       1
#define kESControllerVersion    2

void ReadESKeyboard ( void )
{
    UChar       readBuf[4];
    register            UChar*      readScan = readBuf;
    volatile register   UChar*      reg = (UChar*) kData2;      // only support keyboard on PORT 2!
    short       hshkState;
    UChar       len;
    UChar       temp;
    register            long        timeout = 100;
    register ULong kbID = 0xC030609;

    *(reg)              = kTH + kTR;                        // both flags hi
    *(char *)kSerial2   = 0;                                // clear serial modes
    *(char *)kCtl2      = kTH + kTR;                        // both flags are outputs now

    nop; nop;
    *readScan++ = *reg & 0x0F;                              // 1st nybble = identifying ID

    *reg = kTR;                                             // turn off TH to start hshk seq

    do {
        *readScan = *reg & 0x0F;
    }
    while ( (*readScan != ((kbID >> 16) & 0xF)) && --timeout );     // 2nd nybble has no handshake

    if ( !timeout )
    {
        *reg = kTH + kTR;                               // make sure we leave with TH & TR hi
        return;
    }

    readScan++;

    hshkState = 0;                                          // start flipping TR
    *readScan++ = GetHandshakeNibblePort2(&hshkState);      // 3rd nybble = local ID
    *readScan++ = GetHandshakeNibblePort2(&hshkState);      // 4th nybble = local ID

    if ( *(ULong *) readBuf == kbID )                       // found a good Eric Smith Keyboard
    {
        len = GetHandshakeNibblePort2(&hshkState);          // 5th nybble = BYTE count, 0-15

        if (len)
        {
            temp = GetHandshakeNibblePort2(&hshkState);             // get data type
            temp <<= 4;
            temp |= GetHandshakeNibblePort2(&hshkState);

            len--;                                                  // len includes data type byte

            if ( temp == kESKeycodeData )
            {                                                       // Key Codes follow
                // readScan = REFGLOBAL( controls, keycodeBuf );
                while ( len )
                {
                    // putChar('K');

                    // REFGLOBAL( controls, keycodeHead )++;                        // bump head
                    // REFGLOBAL( controls, keycodeHead ) &= kKeybdDataFifoMask;    // circular buf
                    temp = GetHandshakeNibblePort2(&hshkState);
                    temp <<= 4;
                    temp |= GetHandshakeNibblePort2(&hshkState);
                    // readScan[REFGLOBAL( controls, keycodeHead )] = temp;

                    if(temp < scancodeTableSize)
                    {
                        putChar(scancodeToAscii[temp]);
                    }
                    else
                    {
                        putChar('X');
                    }

                    len--;
                }
            }
            else
            {                                                       // Status Bytes follow
                // readScan = REFGLOBAL( controls, statusBuf );
                while ( len )
                {
                    putChar('S');
                    // REFGLOBAL( controls, statusHead )++;                         // bump head
                    // REFGLOBAL( controls, statusHead ) &= kKeybdCmdStatusFifoMask;    // circular buf
                    temp = GetHandshakeNibblePort2(&hshkState);
                    temp <<= 4;
                    temp |= GetHandshakeNibblePort2(&hshkState);
                    // readScan[REFGLOBAL( controls, statusHead )] = temp;
                    len--;
                }
            }
        }
        *reg = kTH + kTR;                               // make sure we leave with TH & TR hi
    }

}





void WriteESKeyboard ( void )
{
    UChar       readBuf[4];
    register            UChar*      readScan = readBuf;
    volatile register   UChar*      reg = (UChar*) kData2;          // only support keyboard on PORT 2!
    short       hshkState;
    register            long        timeout = 100;
    register ULong kbID = 0xC030609;
    UChar       byteToSend;



//    if ( REFGLOBAL( controls, keyboardPresent ) &&
//         (REFGLOBAL( controls, cmdTail ) != REFGLOBAL( controls, cmdHead )) )
    
    if( !capslockBytesSent )
    {
        *(reg)              = kTH + kTR;                        // both flags hi
        *(char *)kSerial2   = 0;                                // clear serial modes
        *(char *)kCtl2      = kTH + kTR;                        // both flags are outputs now

        nop; nop;
        *readScan++ = *reg & 0x0F;                              // 1st nybble = identifying ID

        *reg = kTR;                                             // turn off TH to start hshk seq

        do {
            *readScan = *reg & 0x0F;
        }
        while ( (*readScan != ((kbID >> 16) & 0xF)) && --timeout );     // 2nd nybble has no handshake

        if ( !timeout )
        {
            *reg = kTH + kTR;                                   // make sure we leave with TH & TR hi
            return;
        }

        readScan++;

        hshkState = 0;                                          // start flipping TR
        *readScan++ = GetHandshakeNibblePort2(&hshkState);      // 3rd nybble = local ID


        if ( (*(ULong *) readBuf & 0xFFFFFF00) == (kbID & 0xFFFFFF00) )     // found a good Eric Smith Keyboard?
        {
            *reg &= 0xF0;                                       // ensure data lines are 0
            *(char *)kCtl2 |= kDataLines;                       // 4 data lines are outputs now


//            REFGLOBAL( controls, cmdTail )++;
//            REFGLOBAL( controls, cmdTail ) &= kKeybdCmdStatusFifoMask;
//            byteToSend = REFGLOBAL( controls, cmdBuf )[REFGLOBAL( controls, cmdTail )];


            if( !capslockDataNotRegister )
                byteToSend = 0xED;
            else {
                if( capslockStatus )
                    byteToSend = 0x00000004;
                else
                    byteToSend = 0x00000000;
            }


            PutHandshakeNibblePort2(&hshkState, 0);             // 4th nybble = 0 ==> I'm talking to him
            PutHandshakeNibblePort2(&hshkState, 2);             // 2 bytes follow; type & data

            PutHandshakeNibblePort2(&hshkState, ((kESKeycodeData & 0xF0)>>4));
            PutHandshakeNibblePort2(&hshkState, (kESKeycodeData & 0x0F));           // 1st byte = type

            PutHandshakeNibblePort2(&hshkState, ((byteToSend & 0xF0)>>4));
            PutHandshakeNibblePort2(&hshkState, (byteToSend & 0x0F));               // 2nd byte = data

            *(char *)kCtl2 &= ~kDataLines;                      // 4 data lines are back to being inputs
            *reg = kTH + kTR;                                   // make sure we leave with TH & TR hi


            if( capslockDataNotRegister == 1 )
                capslockBytesSent = 1;

            capslockDataNotRegister = 1;
        }
    }

}





short GetHandshakeNibblePort2( short* hshkState )
{
    register            long        timeout = 100;
    volatile register   UChar*      reg = (UChar*) kData2;

    if (*hshkState == -1)   // timed out, abort (see below)
        return 0x0F;

    if ((*hshkState ^= 1) == 0)
    {
        *reg |= kTR;            // raise TR, wait for TL high
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
        *reg &= ~kTR;           // lower TR, wait for TL low
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





/* By the time we get here, the 4 data lines have been set up as outputs.
   The caller must ensure that there is nothing in the upper nybble of byteToSend.
 */

void PutHandshakeNibblePort2( short* hshkState, unsigned char byteToSend )
{
    register            long        timeout = 100;
    volatile register   UChar*      reg = (UChar*) kData2;

    if (*hshkState == -1)   // timed out, abort (see below)
        return;

    *reg = (*reg & 0xF0) | byteToSend;  // up to caller to be sure nothing is in hi nybble

    if ((*hshkState ^= 1) == 0)
    {
        *reg |= kTR;                    // raise TR, wait for TL high
        nop; nop;
        do {}
        while (!(*reg & kTL) && --timeout);
        if ( timeout )
            return;
    }
    else
    {
        *reg &= ~kTR;                   // lower TR, wait for TL low
        nop; nop;
        do {}
        while ((*reg & kTL) && --timeout);
        if ( timeout )
            return;
    }

    // if we got this far, we've timed out. return 0xFFs to abort.
    *hshkState = -1;
}



void _vint_callback()
{
    readControllers();
    readKeyboard();
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


