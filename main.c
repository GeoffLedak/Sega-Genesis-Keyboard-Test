#include <stdio.h>
#include <stdlib.h>

#include "hw_md.h"
#include "scancodeTable.h"

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
void EmulateJoypadWithKeyboard( void );
unsigned char GetNextESKeyboardChar( void );
unsigned char GetNextESKeyboardRawcode( void );
void BackUpKeycodeTail();

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


unsigned char scancodeTableSize = sizeof(scancodeToAscii) / sizeof(unsigned char);



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
        EmulateJoypadWithKeyboard();
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
    register            ULong       kbID = 0xC030609;

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
                readScan = ControlGlobals.keycodeBuf;
                while ( len )
                {
                    // REFGLOBAL( controls, keycodeHead )++;                        // bump head
                    ControlGlobals.keycodeHead ++;
                    // REFGLOBAL( controls, keycodeHead ) &= kKeybdDataFifoMask;    // circular buf
                    ControlGlobals.keycodeHead &= kKeybdDataFifoMask;
                    temp = GetHandshakeNibblePort2(&hshkState);
                    temp <<= 4;
                    temp |= GetHandshakeNibblePort2(&hshkState);
                    // readScan[REFGLOBAL( controls, keycodeHead )] = temp;
                    readScan[ControlGlobals.keycodeHead] = temp;
                    len--;
                }
            }
            else
            {                                                       // Status Bytes follow
                // readScan = REFGLOBAL( controls, statusBuf );
                readScan = ControlGlobals.statusBuf;
                while ( len )
                {
                    // REFGLOBAL( controls, statusHead )++;                            // bump head
                    ControlGlobals.statusHead ++;
                    // REFGLOBAL( controls, statusHead ) &= kKeybdCmdStatusFifoMask;   // circular buf
                    ControlGlobals.statusHead &= kKeybdCmdStatusFifoMask;
                    temp = GetHandshakeNibblePort2(&hshkState);
                    temp <<= 4;
                    temp |= GetHandshakeNibblePort2(&hshkState);
                    // readScan[REFGLOBAL( controls, statusHead )] = temp;
                    readScan[ControlGlobals.statusHead] = temp;
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



void EmulateJoypadWithKeyboard( void )
{
    key = GetNextESKeyboardChar();

    // REFGLOBAL( controls, sysKeysHead )++;       /* not pad-like, stick in key buf */
    ControlGlobals.sysKeysHead ++;
    // REFGLOBAL( controls, sysKeysHead ) &= kSysKeysFifoMask;
    ControlGlobals.sysKeysHead &= kSysKeysFifoMask;
    // REFGLOBAL( controls, sysKeysBuf )[REFGLOBAL( controls, sysKeysHead )] = key;
    ControlGlobals.sysKeysBuf[ControlGlobals.sysKeysHead] = key;

}




void BackUpKeycodeTail( void )
{
    /*
    if ( REFGLOBAL( controls, keycodeTail ) == 0 )              // back up; we'll try again later
        REFGLOBAL( controls, keycodeTail ) = kKeybdDataFifoMask;
    else
        REFGLOBAL( controls, keycodeTail )--;
    */

    if( ControlGlobals.keycodeTail == 0 )
        ControlGlobals.keycodeTail = kKeybdDataFifoMask;
    else
        ControlGlobals.keycodeTail --;
}




unsigned char GetNextESKeyboardChar( void )
{
unsigned char raw;
unsigned char map;
unsigned long offset;
unsigned char fuck[2];
Boolean       specialPending;
    
    specialPending = false;
    map = kNoKey;
    
    // if ( REFGLOBAL( controls, keyMapTable ) )
    if ( scancodeToAscii )
    {
        raw = GetNextESKeyboardRawcode();           // get next code out of buffer
        
        //
        // BREAK (KEYUP) CODES
        //
        
        if ( raw == 0xF0 )                              
        {
            raw = GetNextESKeyboardRawcode();                               // try to get the one that's breaking
            
            if ( raw == 0xFF )                                              // not in buf yet?
            {
                BackUpKeycodeTail();
                return( kNoKey );
            }
                
            if ( raw < scancodeTableSize )       // valid code?
            {
                // map = (REFGLOBAL( controls, keyMapTable )->mapTbl)[raw];
                map = scancodeToAscii[raw];
                
                if ( map == kShiftKey )             
                    // REFGLOBAL( controls, keyboardFlags ) &= ~kShiftDown;
                    ControlGlobals.keyboardFlags &= ~kShiftDown;

                if ( map == kAltKey )               
                    // REFGLOBAL( controls, keyboardFlags ) &= ~kAltDown;
                    ControlGlobals.keyboardFlags &= ~kAltDown;

                if ( map == kControlKey )               
                    // REFGLOBAL( controls, keyboardFlags ) &= ~kControlDown;
                    ControlGlobals.keyboardFlags &= ~kControlDown;

                if ( map == kCapsLockKey )          
                    // REFGLOBAL( controls, keyboardFlags ) &= ~kCapsLockDown;     // caps lock up
                    ControlGlobals.keyboardFlags &= ~kCapsLockDown;
            }
            
            return( kNoKey );
        }
        
        //
        // 101-STYLE CODES
        //

        if ( raw == 0xE0 )                                                  
        {
            raw = GetNextESKeyboardRawcode();                       // get key code
            
            if ( raw == 0xFF )                                      // special key there yet?
            {
                BackUpKeycodeTail();
                return( kNoKey );
            }
            else                                                    // special key *is* there
            {
                if ( raw == 0xF0 )                                  // just trash breaks
                {
                    raw = GetNextESKeyboardRawcode();               // try to get one to break
                    if ( raw == 0xFF )                              // key to break here?
                    {
                        BackUpKeycodeTail();
                        BackUpKeycodeTail();
                    }
                    return( kNoKey );
                }
                specialPending = true;              // flag it special
            }
        }
        
        //
        // NORMAL CODES
        //
        
        // if ( raw < (REFGLOBAL( controls, keyMapTable )->tblSectionSize) )       // valid code?
        if( raw < scancodeToAscii )
        {
            // map = (REFGLOBAL( controls, keyMapTable )->mapTbl)[raw];            // map to lower case
            map = scancodeToAscii[raw];

            // if ( REFGLOBAL( controls, keyboardFlags ) & kShiftDown )            // shift key -> ALL shifted
            if( ControlGlobals.keyboardFlags & kShiftDown )
            {
                if ( (map >= 0x20) && (map <= 0x7E) )
                {
                    // offset = raw + REFGLOBAL( controls, keyMapTable )->tblSectionSize;  // use upper tbl
                    // map = (REFGLOBAL( controls, keyMapTable )->mapTbl)[offset];         // map to shifted
                    map = scancodeToAsciiShifted[raw];
                }   
            }
            else
                // if ( REFGLOBAL( controls, keyboardFlags ) & kCapsLocked )           // capslock key -> CHARS shifted
                if( ControlGlobals.keyboardFlags & kCapsLocked )
                {
                    if ( (map >= 'a') && (map <= 'z') )
                    {
                        // offset = raw + REFGLOBAL( controls, keyMapTable )->tblSectionSize;  // use upper tbl
                        // map = (REFGLOBAL( controls, keyMapTable )->mapTbl)[offset];         // map to shifted
                        map = scancodeToAsciiShifted[raw];
                    }
                }
        }
        
        //
        // META KEYS
        //
            
        switch(map)
        {   
            case kShiftKey:
                    // REFGLOBAL( controls, keyboardFlags ) |= kShiftDown;
                    ControlGlobals.keyboardFlags |= kShiftDown;
                    return( kNoKey );
                    break;
        
            case kAltKey:
                    REFGLOBAL( controls, keyboardFlags ) |= kAltDown;
                    return( kNoKey );
                    break;
        
            case kControlKey:
                    REFGLOBAL( controls, keyboardFlags ) |= kControlDown;
                    return( kNoKey );
                    break;
        }
        
        if ( (map == kCapsLockKey) && !(REFGLOBAL( controls, keyboardFlags ) & kCapsLockDown) )
        {
            REFGLOBAL( controls, keyboardFlags ) |= kCapsLockDown;
            REFGLOBAL( controls, keyboardFlags ) ^= kCapsLocked;            // flip caps lock state
            fuck[0] = 0xED;                                                 // hit the LED reg
            fuck[1] = REFGLOBAL( controls, keyboardFlags ) & kCapsLocked;   // bits [2:0] are caps/num/scroll lock
            SendCmdToESKeyboard( fuck, 2 );
            return( kNoKey );
        }

        //
        // Special Keys
        //

        if ( !(REFGLOBAL( controls, keyboardFlags ) & kReturnIsLF) && (map == 0x0A) )
            map = kEnterKey;

        if ( specialPending )
        {
            switch( raw )
            {
                case 0x75:                          // up arrow
                            map = kUpArrowKey;
                            break;
                case 0x72:                          // down arrow
                            map = kDownArrowKey;
                            break;
                case 0x6B:                          // left arrow
                            map = kLeftArrowKey;
                            break;
                case 0x74:                          // right arrow
                            map = kRightArrowKey;
                            break;
                case 0x5A:
                            map = kEnterKey;
                            break;
                default:
                            map = kNoKey;
                            break;
            }
        }   

    }
        
    return( map );
}





unsigned char GetNextESKeyboardRawcode( void )
{
    // if ( REFGLOBAL( controls, keyboardPresent ) && (REFGLOBAL( controls, keycodeTail ) != REFGLOBAL( controls, keycodeHead )) )
    if( ControlGlobals.keycodeTail != ControlGlobals.keycodeHead )
    {
        // REFGLOBAL( controls, keycodeTail )++;
        ControlGlobals.keycodeTail ++;
        // REFGLOBAL( controls, keycodeTail ) &= kKeybdDataFifoMask;
        ControlGlobals.keycodeTail &= kKeybdDataFifoMask;
        // return( REFGLOBAL( controls, keycodeBuf )[REFGLOBAL( controls, keycodeTail )] );
        return ControlGlobals.keycodeBuf[ControlGlobals.keycodeTail];
    }
    else
        return( 0xFF );
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


