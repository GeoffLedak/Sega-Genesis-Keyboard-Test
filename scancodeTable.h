
// --------------------------------------
// general OS user-interface code calls this one to read
//  a hardware keyboard, if we have one.  Returns 0xFF if no char available.

#define kUpArrowKey         0xFD
#define kDownArrowKey       0xFC
#define kRightArrowKey      0xFB
#define kLeftArrowKey       0xFA
#define kAltKey             0xF9
#define kControlKey         0xF8
#define kEnterKey           0xF7
#define kNoKey              0x00

#define kAKey               0x1C
#define kBKey               0x32
#define kCKey               0x21
#define kSKey               0x1B

/* keybd state flags */
#define kReturnIsLF         0x01000000
#define kControlDown        0x00000040
#define kAltDown            0x00000020
#define kShiftDown          0x00000010
#define kCapsLockDown       0x00000008
#define kCapsLocked         0x00000004
#define kNumLocked          0x00000002
#define kScrollLocked       0x00000001



#define kKeybdDataFifoMask      0xF
#define kSysKeysFifoMask        0xF
#define kKeybdCmdStatusFifoMask 0x7

/* special keymap vals */
#define kShiftKey           0xFF
#define kCapsLockKey        0xFE



#define keyDelete               1
#define keyCancel               2
#define keyShift                3
#define keyReturn               4
#define keySwitch               5
#define keyDone                 6



#define kUP                     0x01
#define kDOWN                   0x02
#define kLEFT                   0x04
#define kRIGHT                  0x08
#define kButtonB                0x10        // note B & C are reversed
#define kButtonC                0x20
#define kButtonA                0x40
#define kStart                  0x80
#define kButtonZ                0x100
#define kButtonY                0x200
#define kButtonX                0x400
#define kMode                   0x800



typedef
struct ControlGlobals
{
    short           controlValues[5];       // 4 controllers, isMulti
    unsigned char   sysKeysBuf[16];
    unsigned char   sysKeysHead,sysKeysTail;
    unsigned char   keycodeBuf[16];
    unsigned char   keycodeHead,keycodeTail;
    unsigned char   statusBuf[8];
    unsigned char   statusHead,statusTail;
    unsigned char   cmdBuf[8];
    unsigned char   cmdHead,cmdTail;
    char            keyboardPresent;
    char            pad;
    // KeyMappingTable *keyMapTable;
    unsigned long   keyboardID;
    unsigned long   keyboardFlags;
    long            samplePhase;            // not everything each VBL
    // ControllerType  curType;                // routine for reading appropriate multiType
} ControlGlobals;




static const unsigned char scancodeToAscii[] =
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
    kShiftKey,     // 12, L SHFT
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
    kShiftKey,     // 59, R SHFT
    0x0A,     // 5A, ENTER
    ']',   // 5B, ]
    0,     // 5C, unused
    '\\',  // 5D, \ backslash
    0,     // 5E, unused
    0,     // 5F, unused
    0,     // 60
    0,     // 61
    0,     // 62
    0,     // 63
    0,     // 64
    0,     // 65
    0x08,  // 66, BKSP
    0,     // 67
    0,     // 68
    0,     // 69
    0,     // 6A
    0,     // 6B
    0,     // 6C
    0,     // 6D
    0,     // 6E
    0,     // 6F
    0,     // 70
    0,     // 71
    0,     // 72
    0,     // 73
    0,     // 74
    0,     // 75
    0x1B,  // 76, ESC
    0,     // 77
    0,     // 78
    0,     // 79
    0,     // 7A
    0,     // 7B
    0,     // 7C
    0,     // 7D
    0,     // 7E
    0,     // 7F
    0,     // 80
    0,     // 81
    0,     // 82
    0      // 83


    // keypad 1 starts at 69, goes up into 70?
    // esc is 76
    // numlock is 77
    // F11 is 78
    // scroll lock is 7E

    // 83 is F7

    // 83 is the largest value

};



static const unsigned char scancodeToAsciiShifted[] =
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
    0x7E,  // 0E, `
    0,     // 0F, unused
    0,     // 10, unused
    0,     // 11, L ALT
    kShiftKey,     // 12, L SHFT
    0,     // 13, unused
    0,     // 14, L CTRL
    'Q',   // 15, q
    '!',   // 16, 1
    0,     // 17, unused
    0,     // 18, unused
    0,     // 19, unused
    'Z',   // 1A, z
    'S',   // 1B, s
    'A',   // 1C, a
    'W',   // 1D, w
    '@',   // 1E, 2
    0,     // 1F, unused
    0,     // 20, unused
    'C',   // 21, c
    'X',   // 22, x
    'D',   // 23, d
    'E',   // 24, 
    '$',   // 25, 
    '#',   // 26, 
    0,     // 27, unused
    0,     // 28, unused
    ' ',   // 29, space
    'V',   // 2A, v
    'F',   // 2B, f
    'T',   // 2C, t
    'R',   // 2D, r
    '%',   // 2E, 5
    0,     // 2F, unused
    0,     // 30, unused
    'N',   // 31, n
    'B',   // 32, b
    'H',   // 33, h
    'G',   // 34, g
    'Y',   // 35, y
    '^',   // 36, 6
    0,     // 37, unused
    0,     // 38, unused
    0,     // 39, unused
    'M',   // 3A, m
    'J',   // 3B, j
    'U',   // 3C, u
    '&',   // 3D, 7
    '*',   // 3E, 8
    0,     // 3F, unused
    0,     // 40, unused
    '<',   // 41, ,
    'K',   // 42, k
    'I',   // 43, i
    'O',   // 44, o
    ')',   // 45, 0
    '(',   // 46, 9
    0,     // 47, unused
    0,     // 48, unused
    '>',   // 49, .
    '?',   // 4A, /
    'L',   // 4B, l
    ':',   // 4C, ;
    'P',   // 4D, p
    '_',   // 4E, -
    0,     // 4F, unused
    0,     // 50, unused
    0,     // 51, unused
    '\"',  // 52, '
    0,     // 53, unused
    '{',   // 54, [
    '+',   // 55, =
    0,     // 56, unused
    0,     // 57, unused
    0,     // 58, caps capslock
    kShiftKey,     // 59, R SHFT
    0x0A,     // 5A, ENTER
    '}',   // 5B, ]
    0,     // 5C, unused
    '|',  // 5D, \ backslash
    0,     // 5E, unused
    0,     // 5F, unused
    0,     // 60
    0,     // 61
    0,     // 62
    0,     // 63
    0,     // 64
    0,     // 65
    0x08,  // 66, BKSP
    0,     // 67
    0,     // 68
    0,     // 69
    0,     // 6A
    0,     // 6B
    0,     // 6C
    0,     // 6D
    0,     // 6E
    0,     // 6F
    0,     // 70
    0,     // 71
    0,     // 72
    0,     // 73
    0,     // 74
    0,     // 75
    0x1B,  // 76, ESC
    0,     // 77
    0,     // 78
    0,     // 79
    0,     // 7A
    0,     // 7B
    0,     // 7C
    0,     // 7D
    0,     // 7E
    0,     // 7F
    0,     // 80
    0,     // 81
    0,     // 82
    0      // 83

    // keypad 1 starts at 69, goes up into 70?
    // esc is 76
    // numlock is 77
    // F11 is 78
    // scroll lock is 7E

    // 83 is F7

    // 83 is the largest value

};


