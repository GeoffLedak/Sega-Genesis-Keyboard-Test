typedef struct {
	unsigned char x;
	unsigned char y;
	unsigned char width;
	unsigned char height;
	unsigned char cursor;
	struct textbox_t* self;
	unsigned char *charBuffer;
} textbox_t;
