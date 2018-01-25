#include <stdio.h>
#include <stdlib.h>

#include "hw_md.h"
#include "structures.h"
#include "scancodeTable.h"

unsigned char globalCounter = 0;

void ReadCharacters();

void putChar(char character);
void advanceCursor();
void putNewline();
void WaitForVBlank();
void readControllers();
void readKeyboard();

short GetHandshakeNibblePort2( short* hshkState, char *debugFlag );
void PutHandshakeNibblePort2( short* hshkState, unsigned char byteToSend, char *debugFlag );

int FindESKeyboard( void );
void ReadESKeyboard ( void );
void WriteESKeyboard ( void );
void SendCmdToESKeyboard( unsigned char *cmdBuf, unsigned char cmdLen );
unsigned char GetNextHardwareKeyboardChar( void );
void EmulateJoypadWithKeyboard( void );
long GetHardwareKeyboardFlags( void );
void SetHardwareKeyboardFlags( long flags );
unsigned char GetNextESKeyboardChar( void );
unsigned char GetNextESKeyboardRawcode( void );
void BackUpKeycodeTail();

void _vint_callback();

void setDebugFlag(char *debugFlag, char pass);

char xPosition = 3;
char yPosition = 19;

char aButtonPressed = 0;
char bButtonPressed = 0;
char cButtonPressed = 0;
char startButtonPressed = 0;

unsigned char scancodeTableSize = sizeof(scancodeToAscii) / sizeof(unsigned char);

ControlGlobals ControlGlobalz;

short packetDumpArray[40];

textbox_t console;
textbox_t packetDump;



// Debug Flags

char keyboardFoundDebug = 0;
char keyboardFoundDebugPrev = 0;

char keyboardIdString[5] = {' ', ' ', ' ', ' ', '\0'};
char keyboardIdStringPrev[5] = {' ', ' ', ' ', ' ', '\0'};

char f2Debug = 0;
char f2DebugPrev = 0;
char f3Debug = 0;
char f3DebugPrev = 0;
char f4Debug = 0;
char f4DebugPrev = 0;

char r2Debug = 0;
char r2DebugPrev = 0;
char r3Debug = 0;
char r3DebugPrev = 0;
char r4Debug = 0;
char r4DebugPrev = 0;
char rCDebug = 0;
char rCDebugPrev = 0;
char rT1Debug = 0;
char rT1DebugPrev = 0;
char rT2Debug = 0;
char rT2DebugPrev = 0;
char rKC1Debug = 0;
char rKC1DebugPrev = 0;
char rKC2Debug = 0;
char rKC2DebugPrev = 0;
char rSB1Debug = 0;
char rSB1DebugPrev = 0;
char rSB2Debug = 0;
char rSB2DebugPrev = 0;

char w2Debug = 0;
char w2DebugPrev = 0;
char w3Debug = 0;
char w3DebugPrev = 0;

char eraseThis = 0;




void scrollUp(textbox_t* self)
{
    int i, j;

        for( i = 0; i < self->width; i++ )
        {
            for( j = 0; j < self->height; j++ )
            {
                *(self->charBuffer + self->height * i + j) = *(self->charBuffer + self->height * i + (j + 1) );
            }
        }
}


void advanceWindowCursor(textbox_t* self)
{
	self->cursorX++;

	if( self->cursorX >= (self->x + self->width) )
	{
		self->cursorX = self->x;
		self->cursorY++;
		self->newlineFlag = 1;

		if( self->cursorY >= (self->y + self->height) )
		{
            self->cursorY--;
            scrollUp(self);
            self->scrollFlag = 1;
            self->newlineFlag = 0;
		}
	}

	drawCursor(self);
}


void drawCharToWindow(textbox_t* self, char theChar)
{
	self->drawFlag = 1;
	*(self->charBuffer + self->height * (self->cursorX - self->x) + (self->cursorY - self->y)) = theChar;
	advanceWindowCursor(self);
}


void drawHexStringToWindow(textbox_t* self, short *theString)
{
    int i = 0;
    int j = 0;

    char tempArray[40];

    while( theString[i] != 0xFF )
    {

        if( (theString[i] >> 4) >= 0x00 && (theString[i] >> 4) <= 0x09 )
            tempArray[j] = (theString[i] >> 4) + 0x30;
        else if( (theString[i] >> 4) >= 0x0A && (theString[i] >> 4) <= 0x0F )
        {
            tempArray[j] = (theString[i] >> 4) + 0x37;
        }
        else
            tempArray[j] = '-';


        j++;


        if( (theString[i] & 0xF) >= 0x00 && (theString[i] & 0xF) <= 0x09 )
            tempArray[j] = (theString[i] & 0xF) + 0x30;
        else if( (theString[i] & 0xF) >= 0x0A && (theString[i] & 0xF) <= 0x0F )
        {
            tempArray[j] = (theString[i] & 0xF) + 0x37;
        }
        else
            tempArray[j] = '-';


        j++;
        tempArray[j] = ' ';


        i++;
        j++;
    }

    tempArray[j] = '\0';



    self->drawFlag = 1;


    char *tempArrayIndex = tempArray;


    if( self->cursorY >= (self->y + self->height - 1) )
    {
        scrollUp(self);
    }



    while( *tempArrayIndex != '\0' )
    {   
        *(self->charBuffer + self->height * (self->cursorX - self->x) + (self->cursorY - self->y)) = *tempArrayIndex;

        self->cursorX++;

        if( self->cursorX >= (self->x + self->width) )
        {
            self->cursorX = self->x;
            self->cursorY++;

            if( self->cursorY >= (self->y + self->height) )
            {
                self->cursorY--;
                scrollUp(self);
            }
        }


        tempArrayIndex++;
    }


    while( self->cursorX < (self->x + self->width) )
    {
        *(self->charBuffer + self->height * (self->cursorX - self->x) + (self->cursorY - self->y)) = ' ';
        self->cursorX++;
    }



    // scrollUp(self);
    self->cursorX = self->x;
    self->cursorY++;

    if( self->cursorY >= (self->y + self->height) )
    {
        self->cursorY--;
    }


}


void drawCursor(textbox_t* self)
{
	put_chr(219, 0x0000, self->cursorX, self->cursorY);
}


void drawWindow(textbox_t* self)
{
	if(self->drawFlag)
	{
        if( self->scrollFlag )
        {

        int i, j;
        unsigned char *point;

        for( j = 0; j < self->height; j++ )
        {
            point = (self->scrollBuffer);

            if( j != self->height - 1 )
            {
                for( i = 0; i < self->width; i++ )
                {
                    *point = *(self->charBuffer + self->height * i + j);
                    point++;
                }

                *point = '\0';
                put_str( self->scrollBuffer, 0x0000, self->x, self->y + j );
            }
            else
            {
                put_str( "                                 ", 0x0000, self->x + 1, self->y + j );
            }

        }

        self->scrollFlag = 0;

        }
		else if(!self->newlineFlag)
			put_chr(*(self->charBuffer + self->height * (self->cursorX - self->x - 1) + (self->cursorY - self->y) ), 0x0000, self->cursorX - 1, self->cursorY);
		else
		{
			put_chr(*(self->charBuffer + self->height * (self->width - 1) + (self->cursorY - self->y - 1) ), 0x0000, self->x + self->width - 1, self->cursorY - 1);
			self->newlineFlag = 0;
		}

		self->drawFlag = 0;
	}
}


void drawPacketDumpWindow(textbox_t* self)
{
    if(self->drawFlag)
    {
        int i, j;
        unsigned char *point;

        for( j = 0; j < self->height; j++ )
        {
            point = (self->scrollBuffer);

            for( i = 0; i < self->width; i++ )
            {
                *point = *(self->charBuffer + self->height * i + j);
                point++;
            }

            *point = '\0';
            put_str( self->scrollBuffer, 0x0000, self->x, self->y + j );
        }

        self->drawFlag = 0;
    }
}


void drawDebugFlags()
{
	if( keyboardFoundDebug != keyboardFoundDebugPrev )
	{
		if( keyboardFoundDebug )
			put_str("Found ES Keyboard!", 0x2000, 19, 3);
		else
			put_str("Keyboard not found", 0x4000, 19, 3);

		keyboardFoundDebugPrev = keyboardFoundDebug;
	}
	
	int i;
	int stringsEqual = 1;
	
	for( i = 0; i < 5; i++ )
	{
		if( keyboardIdString[i] != keyboardIdStringPrev[i] )
		{
			stringsEqual = 0;
			break;
		}
	}
	
	if( !stringsEqual )
	{
		put_str(keyboardIdString, 0x0000, 32, 4);
		
		for( i = 0; i < 5; i++ )
		{
			keyboardIdStringPrev[i] = keyboardIdString[i];
		}
	}
	
	
	
	if( f2Debug != f2DebugPrev )
	{
		if( f2Debug == 1 )
			put_str( "f2", 0x2000, 0, 0);
		else
			put_str( "f2", 0x4000, 0, 0);
		
		f2DebugPrev = f2Debug;
	}
	
	if( f3Debug != f3DebugPrev )
	{
		if( f3Debug == 1 )
			put_str( "f3", 0x2000, 1, 0);
		else
			put_str( "f3", 0x4000, 1, 0);
		
		f3DebugPrev = f3Debug;
	}
}


int main(void)
{
	console.newlineFlag = 0;
	console.drawFlag = 0;
    console.scrollFlag = 0;
	console.x = 3;
	console.y = 19;
	console.width = 34;
	console.height = 6;
	console.cursorX = console.x;
	console.cursorY = console.y;
	console.self = &console;
	console.charBuffer = malloc( sizeof(unsigned char) * console.width * console.height );
    console.scrollBuffer = malloc( sizeof(unsigned char) * console.width + 1 );

    packetDump.newlineFlag = 0;
    packetDump.drawFlag = 0;
    packetDump.scrollFlag = 0;
    packetDump.x = 3;
    packetDump.y = 11;
    packetDump.width = 34;
    packetDump.height = 6;
    packetDump.cursorX = packetDump.x;
    packetDump.cursorY = packetDump.y;
    packetDump.self = &packetDump;
    packetDump.charBuffer = malloc( sizeof(unsigned char) * packetDump.width * packetDump.height );
    packetDump.scrollBuffer = malloc( sizeof(unsigned char) * packetDump.width + 1 );


    // 0x0000 = grey
    // 0x2000 = green
    // 0x4000 = red

	WaitForVBlank();
    drawBoxes();
	drawCursor(console.self);

    while ( 1 ) // endless loop
    {	
		ReadCharacters();
		drawWindow(console.self);
        drawPacketDumpWindow(packetDump.self);
		drawDebugFlags();
		


		// WaitForVBlank();
    }
	
	
    return 0;
}


void drawBoxes()
{
    put_str("\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xD1\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB", 0x0000, 2, 2);
	put_str("\xBA               \xB3                  \xBA", 0x0000, 2, 3);
	put_str("\xBA               \xB3                  \xBA", 0x0000, 2, 4);
	put_str("\xBA               \xB3                  \xBA", 0x0000, 2, 5);
	put_str("\xBA               \xB3                  \xBA", 0x0000, 2, 6);
	put_str("\xBA               \xB3                  \xBA", 0x0000, 2, 7);
	put_str("\xBA               \xB3                  \xBA", 0x0000, 2, 8);
	put_str("\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCF\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC", 0x0000, 2, 9);
	put_str("\xDA\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xBF", 0x0000, 2, 10);
	put_str("\xB3                                  \xB3", 0x0000, 2, 11);
	put_str("\xB3                                  \xB3", 0x0000, 2, 12);
	put_str("\xB3                                  \xB3", 0x0000, 2, 13);
	put_str("\xB3                                  \xB3", 0x0000, 2, 14);
	put_str("\xB3                                  \xB3", 0x0000, 2, 15);
	put_str("\xB3                                  \xB3", 0x0000, 2, 16);
	put_str("\xC0\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xD9", 0x0000, 2, 17);
	put_str("\xDA\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xBF", 0x0000, 2, 18);
	put_str("\xB3                                  \xB3", 0x0000, 2, 19);
	put_str("\xB3                                  \xB3", 0x0000, 2, 20);
	put_str("\xB3                                  \xB3", 0x0000, 2, 21);
	put_str("\xB3                                  \xB3", 0x0000, 2, 22);
	put_str("\xB3                                  \xB3", 0x0000, 2, 23);
	put_str("\xB3                                  \xB3", 0x0000, 2, 24);
	put_str("\xC0\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xD9", 0x0000, 2, 25);
}


void ReadCharacters()
{
    short command = 0;

    char keyPress = 0;
    short result = 0;
    unsigned char HWKeypress;
    long hwflags;
    

    HWKeypress = GetNextHardwareKeyboardChar();

    switch( HWKeypress )
    {
        case 0x08:                              // BS?
                command = kButtonC;
                break;
                
        case 0x09:                              // Tab?
                command = kButtonA;
                HWKeypress = keySwitch;
                break;
                
        case 0x0A:  
                command = kButtonA;
                HWKeypress = '\n';
                break;
                
        case 0x1B:                              // ESC?
                command = kButtonA;
                HWKeypress = keyCancel;
                break;
                                
        default:
                if ( HWKeypress != kNoKey )
                    command = kButtonA;         // fake a button down
                else
                {
                    hwflags = GetHardwareKeyboardFlags();
                    if ( hwflags & ( kShiftDown | kCapsLocked ) )
                    {
                        HWKeypress = keyShift;     
                    }
                }
                break;
    }

    /*
    if (command & kLEFT && state->x > kKeyboardLeft)
    {
        --state->x;
        if (state->x == (kKeyboardRight-1)) 
            SetSegaCursorImage(state->cursor, kSmallKeyboardCursor);
    }
    if (command & kRIGHT && state->x < kKeyboardRight)
    {
        ++state->x;
        if (state->x == kKeyboardRight) 
            SetSegaCursorImage(state->cursor, kLargeKeyboardCursor);
    }
    if (command & kUP && state->y > kKeyboardTop)
        --state->y;
    if (command & kDOWN && state->y < kKeyboardBottom)
        ++state->y;
    
    if (command & (kLEFT | kRIGHT | kUP | kDOWN))
        PlayDBFX( kKeyboardMoveSnd, 0, 0 );
    
    MoveSegaCursor(state->cursor,
                state->keyLayout->keyboardX + state->x*kKeyXSize + kKeyboardXOffset,
                state->keyLayout->keyboardY + state->y*kKeyYSize + kKeyboardYOffset);
    */
    
    // Did the user click the button?
    if (command & (kButtonA | kButtonB | kButtonC | kStart))
    {


        
        if (command & kButtonA)
        {
            // User typed a character. Find out which key was hit
            if ( HWKeypress != kNoKey )     // did she type it on the hardware keybd?
                keyPress = HWKeypress;
        }
        
        // Was it a letter, or a special?
        if (keyPress >= ' ')
        {
            // Letter
            // if ( !state->measureProc || (*state->measureProc)(state->keyLayout->fields[state->keyLayout->activeField].buffer) )
            //    TextEditAppend(state->teRefs[state->keyLayout->activeField], 1, &keyPress);


            // putChar(keyPress);

			drawCharToWindow(console.self, keyPress);

        }
        else
        {
            // Special
            switch (keyPress)
            {
                case keyDelete:
                        /*
                        PlayDBFX( kTextDeleteSnd, 0, 0 );
                        TextEditDelete(state->teRefs[state->keyLayout->activeField], 1);
                        */
                        break;
                        
                case keyCancel:
                        /*
                        PlayDBFX( kTextInputSnd, 0, 0 );
                        result = -1;
                        */
                        break;
                
                case keyShift:

                        break;
                        
                case keyReturn:
                        // PlayDBFX( kTextReturnSnd, 0, 0 );
                        keyPress = '\n';
                        // TextEditAppend(state->teRefs[state->keyLayout->activeField], 1, &keyPress);
                        break;
                        
                case keySwitch:
                        /*
                        PlayDBFX( kTextInputSnd, 0, 0 );
                        TextEditDeactivate(state->teRefs[state->keyLayout->activeField]);
                        if (state->hasJizzlers)
                            StopTextBoxAnimation(JIZZLE_FIELD(state, state->keyLayout->activeField));
                        
                        SetFocusField(state->keyLayout,
                            (state->keyLayout->activeField + 1) % state->keyLayout->fieldCount);
                        TextEditActivate(state->teRefs[state->keyLayout->activeField]);
                        if (state->hasJizzlers)
                            StartTextBoxAnimation(JIZZLE_FIELD(state, state->keyLayout->activeField), 5);
                        */
                        break;
                
                case keyDone:
                        // PlayDBFX( kTextInputSnd, 0, 0 );
                        // result = 1;
                        break;
            }
        }
    }

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

        keyboardFoundDebug = 1;

        ReadESKeyboard();
        WriteESKeyboard();
        EmulateJoypadWithKeyboard();
    }
	else {
		keyboardFoundDebug = 0;
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
        // FAIL f2
        setDebugFlag(&f2Debug, 0);
        *reg = kTH + kTR;                               // make sure we leave with TH & TR hi
        return ( 0 );
    }

    // PASS f2
    setDebugFlag(&f2Debug, 1);

    readScan++;

    hshkState = 0;                                      // start flipping TR
    *readScan++ = GetHandshakeNibblePort2(&hshkState, &f3Debug);  // 3rd nybble = local ID
    *readScan = GetHandshakeNibblePort2(&hshkState, &f4Debug);    // 4th nybble = local ID

    *reg |= kTH;                                        // abort the transaction

    for ( timeout = 0; timeout != 50; timeout++ );      // spin a bit

    *reg = kTH + kTR;                                   // make sure we leave with TH & TR hi

    if ( *(ULong *) readBuf == kbID )                   // found a good Eric Smith Keyboard
    {
        hexToAscii(readBuf);
        return ( 1 );
    }
    else
    {
        hexToAscii(readBuf);
        return ( 0 );
    }
}





#define kESKeycodeData          0
#define kESControllerEcho       1
#define kESControllerVersion    2

void ReadESKeyboard ( void )
{
    unsigned char packetDumpIndex = 0;

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
		// FAIL r2
        setDebugFlag(&r2Debug, 0);
        *reg = kTH + kTR;                               // make sure we leave with TH & TR hi
        return;
    }

	// PASS r2
    setDebugFlag(&r2Debug, 1);
	
    readScan++;

    hshkState = 0;                                          // start flipping TR
    *readScan++ = GetHandshakeNibblePort2(&hshkState, &r3Debug);      // 3rd nybble = local ID
    *readScan++ = GetHandshakeNibblePort2(&hshkState, &r4Debug);      // 4th nybble = local ID

    if ( *(ULong *) readBuf == kbID )                       // found a good Eric Smith Keyboard
    {
        len = GetHandshakeNibblePort2(&hshkState, &rCDebug);          // 5th nybble = BYTE count, 0-15

        if (len)
        {
            temp = GetHandshakeNibblePort2(&hshkState, &rT1Debug);             // get data type
            temp <<= 4;
            temp |= GetHandshakeNibblePort2(&hshkState, &rT2Debug);

            packetDumpArray[packetDumpIndex] = temp;
            packetDumpIndex++;

            len--;                                                  // len includes data type byte

            if ( temp == kESKeycodeData )
            {                                                       // Key Codes follow
                // readScan = REFGLOBAL( controls, keycodeBuf );
                readScan = ControlGlobalz.keycodeBuf;
                while ( len )
                {
                    // REFGLOBAL( controls, keycodeHead )++;                        // bump head
                    ControlGlobalz.keycodeHead ++;
                    // REFGLOBAL( controls, keycodeHead ) &= kKeybdDataFifoMask;    // circular buf
                    ControlGlobalz.keycodeHead &= kKeybdDataFifoMask;
                    temp = GetHandshakeNibblePort2(&hshkState, &rKC1Debug);
                    temp <<= 4;
                    temp |= GetHandshakeNibblePort2(&hshkState, &rKC2Debug);

                    packetDumpArray[packetDumpIndex] = temp;
                    packetDumpIndex++;

                    // readScan[REFGLOBAL( controls, keycodeHead )] = temp;
                    readScan[ControlGlobalz.keycodeHead] = temp;
                    len--;
                }
            }
            else
            {                                                       // Status Bytes follow
                // readScan = REFGLOBAL( controls, statusBuf );
                readScan = ControlGlobalz.statusBuf;
                while ( len )
                {
                    // REFGLOBAL( controls, statusHead )++;                            // bump head
                    ControlGlobalz.statusHead ++;
                    // REFGLOBAL( controls, statusHead ) &= kKeybdCmdStatusFifoMask;   // circular buf
                    ControlGlobalz.statusHead &= kKeybdCmdStatusFifoMask;
                    temp = GetHandshakeNibblePort2(&hshkState, &rSB1Debug);
                    temp <<= 4;
                    temp |= GetHandshakeNibblePort2(&hshkState, &rSB2Debug);

                    packetDumpArray[packetDumpIndex] = temp;
                    packetDumpIndex++;

                    // readScan[REFGLOBAL( controls, statusHead )] = temp;
                    readScan[ControlGlobalz.statusHead] = temp;
                    len--;
                }
            }

        packetDumpArray[packetDumpIndex] = 0xFF;

        drawHexStringToWindow(packetDump.self, packetDumpArray);

        }

        *reg = kTH + kTR;                               // make sure we leave with TH & TR hi
    }
}




/*
    
    3   C   6   0   <len>   <hi nyb>   <low nyb> ....
                ^   \____________________________..../
                |        Same Format as Dat/Status
                |
                Driven to tell Eric I have something for him
                
*/

    
void
WriteESKeyboard ( void )
{
                    UChar       readBuf[4];
register            UChar*      readScan = readBuf;
volatile register   UChar*      reg = (UChar*) kData2;          // only support keyboard on PORT 2!
                    short       hshkState;
register            long        timeout = 100;
register            ULong       kbID = 0xC030609;
                    UChar       byteToSend;
    
    // if ( REFGLOBAL( controls, keyboardPresent ) && (REFGLOBAL( controls, cmdTail ) != REFGLOBAL( controls, cmdHead )) )
    if( ControlGlobalz.cmdTail != ControlGlobalz.cmdHead )
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
			// FAIL w2
			setDebugFlag(&w2Debug, 1);
            *reg = kTH + kTR;                                   // make sure we leave with TH & TR hi
            return;
        }
		
		// PASS w2
		setDebugFlag(&w2Debug, 0);
    
        readScan++;                         
    
        hshkState = 0;                                          // start flipping TR
        *readScan++ = GetHandshakeNibblePort2(&hshkState, &w3Debug);      // 3rd nybble = local ID


        if ( (*(ULong *) readBuf & 0xFFFFFF00) == (kbID & 0xFFFFFF00) )     // found a good Eric Smith Keyboard?
        {
            *reg &= 0xF0;                                       // ensure data lines are 0
            *(char *)kCtl2 |= kDataLines;                       // 4 data lines are outputs now

            // REFGLOBAL( controls, cmdTail )++;
            ControlGlobalz.cmdTail ++;
            // REFGLOBAL( controls, cmdTail ) &= kKeybdCmdStatusFifoMask;
            ControlGlobalz.cmdTail &= kKeybdCmdStatusFifoMask;
            // byteToSend = REFGLOBAL( controls, cmdBuf )[REFGLOBAL( controls, cmdTail )];
            byteToSend = ControlGlobalz.cmdBuf[ControlGlobalz.cmdTail];

            PutHandshakeNibblePort2(&hshkState, 0, &eraseThis);             // 4th nybble = 0 ==> I'm talking to him
            PutHandshakeNibblePort2(&hshkState, 2, &eraseThis);             // 2 bytes follow; type & data
            
            PutHandshakeNibblePort2(&hshkState, ((kESKeycodeData & 0xF0)>>4), &eraseThis);      
            PutHandshakeNibblePort2(&hshkState, (kESKeycodeData & 0x0F), &eraseThis);           // 1st byte = type
            
            PutHandshakeNibblePort2(&hshkState, ((byteToSend & 0xF0)>>4), &eraseThis);      
            PutHandshakeNibblePort2(&hshkState, (byteToSend & 0x0F), &eraseThis);               // 2nd byte = data

            *(char *)kCtl2 &= ~kDataLines;                      // 4 data lines are back to being inputs
            *reg = kTH + kTR;                                   // make sure we leave with TH & TR hi
        }
    }
        
}





void SendCmdToESKeyboard( unsigned char *cmdBuf, unsigned char cmdLen )
{
unsigned short oldSR;

    while ( cmdLen )
    {

        // asm {   move.w  sr,oldSR
        //         move.w  #0x2700,sr
        //     }

        // __asm__ __volatile__ ("move.w  sr, oldSR\n\t"
        //                       "move.w  #0x2700, sr\n\t");

        // REFGLOBAL( controls, cmdHead )++;
        ControlGlobalz.cmdHead ++;
        // REFGLOBAL( controls, cmdHead ) &= kKeybdCmdStatusFifoMask;
        ControlGlobalz.cmdHead &= kKeybdCmdStatusFifoMask;
        // REFGLOBAL( controls, cmdBuf )[REFGLOBAL( controls, cmdHead )] = *cmdBuf;
        ControlGlobalz.cmdBuf[ControlGlobalz.cmdHead] = *cmdBuf;
        // asm {   move.w  oldSR,sr    }
        // __asm__ __volatile__ ("move.w  oldSR, sr\n\t");

        cmdBuf++;       
        cmdLen--;
    }
}





unsigned char GetNextHardwareKeyboardChar( void )
{
    // if ( REFGLOBAL( controls, keyboardPresent ) && (REFGLOBAL( controls, sysKeysTail ) != REFGLOBAL( controls, sysKeysHead )) )
    if( ControlGlobalz.sysKeysTail != ControlGlobalz.sysKeysHead )
    {
        // REFGLOBAL( controls, sysKeysTail )++;
        ControlGlobalz.sysKeysTail ++;
        // REFGLOBAL( controls, sysKeysTail ) &= kSysKeysFifoMask;
        ControlGlobalz.sysKeysTail &= kSysKeysFifoMask;
        // return( REFGLOBAL( controls, sysKeysBuf )[REFGLOBAL( controls, sysKeysTail )] );
        return ControlGlobalz.sysKeysBuf[ControlGlobalz.sysKeysTail];
    }
    else
        return( kNoKey );
}





void EmulateJoypadWithKeyboard( void )
{
    unsigned char key;

    key = GetNextESKeyboardChar();

    // REFGLOBAL( controls, sysKeysHead )++;       /* not pad-like, stick in key buf */
    ControlGlobalz.sysKeysHead ++;
    // REFGLOBAL( controls, sysKeysHead ) &= kSysKeysFifoMask;
    ControlGlobalz.sysKeysHead &= kSysKeysFifoMask;
    // REFGLOBAL( controls, sysKeysBuf )[REFGLOBAL( controls, sysKeysHead )] = key;
    ControlGlobalz.sysKeysBuf[ControlGlobalz.sysKeysHead] = key;

}




long GetHardwareKeyboardFlags( void )
{
    // return( REFGLOBAL( controls, keyboardFlags ) );
    return ControlGlobalz.keyboardFlags;
}



void SetHardwareKeyboardFlags( long flags )
{
unsigned char fuck[2];

    // REFGLOBAL( controls, keyboardFlags ) = flags;
    ControlGlobalz.keyboardFlags = flags;
    fuck[0] = 0xED;                     // hit the LED reg
    fuck[1] = flags & 0x7;              // bits [2:0] are caps/num/scroll lock
    SendCmdToESKeyboard( fuck, 2 );
}




void BackUpKeycodeTail( void )
{
    /*
    if ( REFGLOBAL( controls, keycodeTail ) == 0 )              // back up; we'll try again later
        REFGLOBAL( controls, keycodeTail ) = kKeybdDataFifoMask;
    else
        REFGLOBAL( controls, keycodeTail )--;
    */

    if( ControlGlobalz.keycodeTail == 0 )
        ControlGlobalz.keycodeTail = kKeybdDataFifoMask;
    else
        ControlGlobalz.keycodeTail --;
}




unsigned char GetNextESKeyboardChar( void )
{
unsigned char raw;
unsigned char map;
unsigned long offset;
unsigned char fuck[2];
char       specialPending;
    
    specialPending = 0;
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
                    ControlGlobalz.keyboardFlags &= ~kShiftDown;

                if ( map == kAltKey )               
                    // REFGLOBAL( controls, keyboardFlags ) &= ~kAltDown;
                    ControlGlobalz.keyboardFlags &= ~kAltDown;

                if ( map == kControlKey )               
                    // REFGLOBAL( controls, keyboardFlags ) &= ~kControlDown;
                    ControlGlobalz.keyboardFlags &= ~kControlDown;

                if ( map == kCapsLockKey )          
                    // REFGLOBAL( controls, keyboardFlags ) &= ~kCapsLockDown;     // caps lock up
                    ControlGlobalz.keyboardFlags &= ~kCapsLockDown;
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
                specialPending = 1;              // flag it special
            }
        }
        
        //
        // NORMAL CODES
        //
        
        // if ( raw < (REFGLOBAL( controls, keyMapTable )->tblSectionSize) )       // valid code?
        if( raw < scancodeTableSize )
        {
            // map = (REFGLOBAL( controls, keyMapTable )->mapTbl)[raw];            // map to lower case
            map = scancodeToAscii[raw];

            // if ( REFGLOBAL( controls, keyboardFlags ) & kShiftDown )            // shift key -> ALL shifted
            if( ControlGlobalz.keyboardFlags & kShiftDown )
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
                if( ControlGlobalz.keyboardFlags & kCapsLocked )
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
                    ControlGlobalz.keyboardFlags |= kShiftDown;
                    return( kNoKey );
                    break;
        
            case kAltKey:
                    // REFGLOBAL( controls, keyboardFlags ) |= kAltDown;
                    ControlGlobalz.keyboardFlags |= kAltDown;
                    return( kNoKey );
                    break;
        
            case kControlKey:
                    // REFGLOBAL( controls, keyboardFlags ) |= kControlDown;
                    ControlGlobalz.keyboardFlags |= kControlDown;
                    return( kNoKey );
                    break;
        }
        
        // if ( (map == kCapsLockKey) && !(REFGLOBAL( controls, keyboardFlags ) & kCapsLockDown) )
        if ( (map == kCapsLockKey) && !(ControlGlobalz.keyboardFlags & kCapsLockDown) )
        {
            // REFGLOBAL( controls, keyboardFlags ) |= kCapsLockDown;
            ControlGlobalz.keyboardFlags |= kCapsLockDown;
            // REFGLOBAL( controls, keyboardFlags ) ^= kCapsLocked;            // flip caps lock state
            ControlGlobalz.keyboardFlags ^= kCapsLocked;
            fuck[0] = 0xED;                                                 // hit the LED reg
            // fuck[1] = REFGLOBAL( controls, keyboardFlags ) & kCapsLocked;   // bits [2:0] are caps/num/scroll lock
            fuck[1] = ControlGlobalz.keyboardFlags & kCapsLocked;
            SendCmdToESKeyboard( fuck, 2 );
            return( kNoKey );
        }

        //
        // Special Keys
        //

        // if ( !(REFGLOBAL( controls, keyboardFlags ) & kReturnIsLF) && (map == 0x0A) )
        if( map == 0x0A )
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
    if( ControlGlobalz.keycodeTail != ControlGlobalz.keycodeHead )
    {
        // REFGLOBAL( controls, keycodeTail )++;
        ControlGlobalz.keycodeTail ++;
        // REFGLOBAL( controls, keycodeTail ) &= kKeybdDataFifoMask;
        ControlGlobalz.keycodeTail &= kKeybdDataFifoMask;
        // return( REFGLOBAL( controls, keycodeBuf )[REFGLOBAL( controls, keycodeTail )] );
        return ControlGlobalz.keycodeBuf[ControlGlobalz.keycodeTail];
    }
    else
        return( 0xFF );
}




short GetHandshakeNibblePort2( short* hshkState, char *debugFlag )
{
    register            long        timeout = 100;
    volatile register   UChar*      reg = (UChar*) kData2;

    if (*hshkState == -1)   // timed out, abort (see below)
	{
		setDebugFlag(debugFlag, 0);
        return 0x0F;
	}

    if ((*hshkState ^= 1) == 0)
    {
        *reg |= kTR;            // raise TR, wait for TL high
        nop; nop;
        do {}
        while (!(*reg & kTL) && --timeout);
        if (timeout)
        {
			setDebugFlag(debugFlag, 1);
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
			setDebugFlag(debugFlag, 1);
            nop; nop;
            return *reg & 0x0F;
        }
    }

    // if we got this far, we've timed out. return 0xFFs to abort.

    *hshkState = -1;
	setDebugFlag(debugFlag, 0);
    return 0xFF;
}





/* By the time we get here, the 4 data lines have been set up as outputs.
   The caller must ensure that there is nothing in the upper nybble of byteToSend.
 */

void PutHandshakeNibblePort2( short* hshkState, unsigned char byteToSend, char *debugFlag )
{
    register            long        timeout = 100;
    volatile register   UChar*      reg = (UChar*) kData2;

    if (*hshkState == -1)   // timed out, abort (see below)
	{
		setDebugFlag(debugFlag, 0);
        return;
	}

    *reg = (*reg & 0xF0) | byteToSend;  // up to caller to be sure nothing is in hi nybble

    if ((*hshkState ^= 1) == 0)
    {
        *reg |= kTR;                    // raise TR, wait for TL high
        nop; nop;
        do {}
        while (!(*reg & kTL) && --timeout);
        if ( timeout )
		{
			setDebugFlag(debugFlag, 1);
            return;
		}
    }
    else
    {
        *reg &= ~kTR;                   // lower TR, wait for TL low
        nop; nop;
        do {}
        while ((*reg & kTL) && --timeout);
        if ( timeout )
		{
			setDebugFlag(debugFlag, 1);
            return;
		}
    }

    // if we got this far, we've timed out. return 0xFFs to abort.
    *hshkState = -1;
	setDebugFlag(debugFlag, 0);
}



void _vint_callback()
{
    readControllers();
    readKeyboard();

	if( globalCounter >= 60 ) globalCounter = 0;
	else globalCounter++;
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

	if( xPosition > 36 )
	{
		xPosition = 3;
		yPosition++;
		
		if( yPosition > 24 )
		{
			yPosition = 19;
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
	xPosition = 3;
	yPosition++;
	
	if( yPosition > 24 )
	{
		yPosition = 19;
	}
}


void setDebugFlag(char *debugFlag, char pass)
{
	if( &(*debugFlag) == &f2Debug ) f2Debug = pass;
	if( &(*debugFlag) == &f3Debug ) f3Debug = pass;
	
	
	
}


void hexToAscii(unsigned char *sandwiches)
{
    // C369  <---- keyboard id

    char i;
	
    for( i = 0; i < 5; i++ )
    {
        if( i == 4 )
            keyboardIdString[i] = '\0';
        else
        {	
            if( sandwiches[i] >= 0x00 && sandwiches[i] <= 0x09 )
                keyboardIdString[i] = sandwiches[i] + 0x30;
            else if( sandwiches[i] >= 0x0A && sandwiches[i] <= 0x0F )
                keyboardIdString[i] = sandwiches[i] + 0x37;
            else
                keyboardIdString[i] = '-';
        }
    }
}


