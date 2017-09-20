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
    Boolean         keyboardPresent;
    char            pad;
    KeyMappingTable *keyMapTable;
    unsigned long   keyboardID;
    unsigned long   keyboardFlags;
    long            samplePhase;            // not everything each VBL
    ControllerType  curType;                // routine for reading appropriate multiType
} ControlGlobals;