#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <driver/keyboard.h>
#include <utils.h>
#include <interrupts.h>

//! a ring buffer to store input characters 
#define KBD_RING_BUF_SIZE 32

//! private data

static KBD_ENTRY _kbd_ring_buffer[KBD_RING_BUF_SIZE];	// this buffer only stores the valid keycodes
static uint32_t  _kbd_ring_buffer_head = 0; // head of the ring buffer
static uint32_t  _kbd_ring_buffer_tail = 0; // tail of the ring buffer

//! private functions for ring buffer management
static bool 	 _kbd_ring_buffer_full(void);
static bool 	 _kbd_ring_buffer_empty(void);
static void 	 _kbd_ring_buffer_push(KBD_KEYCODE key);
static KBD_ENTRY _kbd_ring_buffer_pop(void);

// helpers
void 			 _kbd_ring_buffer_clear(void);
void 			 clear_all_modifiers(void);

// private self-defined variables
static bool shift_pressed_state = false;
static bool ctrl_pressed_state = false;
static bool alt_pressed_state = false;
static bool capslock_toggled_state = false;
static bool numlock_toggled_state = false;
static bool scrolllock_toggled_state = false;

//! private function prototypes


//! standard keyboard scancode set. array index represents the make code
static const KBD_KEYCODE _kbd_scancode_set[] = {
	KEY_UNKNOWN, KEY_ESCAPE,									// 0x00 - 0x01
	KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,							// 0x02 - 0x06
	KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,							// 0x07 - 0x0b
	KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_TAB,				// 0x0c - 0x0f
	KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T,							// 0x10 - 0x14
	KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,							// 0x15 - 0x19
	KEY_LEFTBRACKET, KEY_RIGHTBRACKET, KEY_RETURN, KEY_LCTRL,   // 0x1a - 0x1d
	KEY_A, KEY_S, KEY_D, KEY_F, KEY_G,							// 0x1e - 0x22
	KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,					// 0x23 - 0x27
	KEY_QUOTE, KEY_GRAVE, KEY_LSHIFT, KEY_BACKSLASH, 			// 0x28 - 0x2b
	KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M,			// 0x2c - 0x32
	KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RSHIFT,					// 0x33 - 0x36
	KEY_KP_ASTERISK, KEY_RALT, KEY_SPACE, KEY_CAPSLOCK,			// 0x37 - 0x3a
	KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, 					// 0x3b - 0x3f
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,					// 0x40 - 0x44
	KEY_KP_NUMLOCK, KEY_SCROLLLOCK, KEY_HOME,					// 0x45 - 0x47
	KEY_KP_8, KEY_PAGEUP,										// 0x48 - 0x49
	KEY_KP_2, KEY_KP_3, KEY_KP_0, KEY_KP_DECIMAL,				// 0x50 - 0x53
	KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, 						// 0x54 - 0x56
	KEY_F11, KEY_F12											// 0x57 - 0x58
};

#define KBD_SCANCODE_BREAK 0x80 // break code offset

//! check if the keyboard ring buffer is full
bool _kbd_ring_buffer_full (void) {
	return ((_kbd_ring_buffer_head + 1) % KBD_RING_BUF_SIZE) == _kbd_ring_buffer_tail;
}
//! check if the keyboard ring buffer is empty
bool _kbd_ring_buffer_empty (void) {
	return _kbd_ring_buffer_head == _kbd_ring_buffer_tail;
}

//! push a key to the keyboard ring buffer, to be retrieved by higher level functions
void _kbd_ring_buffer_push(KBD_KEYCODE key) {
	if (_kbd_ring_buffer_full()) {
		return; // buffer is full, drop the key
	}
	_kbd_ring_buffer[_kbd_ring_buffer_head].keycode = key; // push the key to the buffer
	_kbd_ring_buffer[_kbd_ring_buffer_head].ascii   = kbd_keycode_to_ascii(key); // store ascii translation
	_kbd_ring_buffer_head = (_kbd_ring_buffer_head + 1) % KBD_RING_BUF_SIZE; // update head
}

//! pop a key from the keyboard ring buffer, to be retrieved by higher level functions
KBD_ENTRY _kbd_ring_buffer_pop (void) {

	// had to add da janky fix

	if (_kbd_ring_buffer_empty()) {
		return (KBD_ENTRY) {.keycode=KEY_UNKNOWN, .ascii='\0'}; // buffer is empty, return invalid key
	}
	KBD_ENTRY key = _kbd_ring_buffer[_kbd_ring_buffer_tail]; // pop the key from the buffer
	_kbd_ring_buffer_tail = (_kbd_ring_buffer_tail + 1) % KBD_RING_BUF_SIZE; // update tail
	return key;
}

//! translates a keycode to its ascii representation, taking into account the
//! state of the modifier keys (shift, capslock)
char kbd_keycode_to_ascii (KBD_KEYCODE key) {
	
	// get state of capslock and shift
	bool shift_state = kbd_get_shift();
	bool capslock_state = kbd_get_capslock();

	// handle alphabets (a-z)
	if (key >= KEY_A && key <= KEY_Z)
	{
		// check if it needs to be capitalized (shift XOR caps)
		bool capitalize = (shift_state ^ capslock_state);
		return (!capitalize ? (char)(key) : (char)(key - 32));
	}

	// handle number row
	if (key >= KEY_0 && key <= KEY_9)
	{
		if (shift_state)
		{
			// return shifted symbols for number row
			switch (key)
			{
				case KEY_0: 
					return ')';
				case KEY_1: 
					return '!';
				case KEY_2: 
					return '@';
				case KEY_3: 
					return '#';
				case KEY_4: 
					return '$';
				case KEY_5: 
					return '%';
				case KEY_6: 
					return '^';
				case KEY_7: 
					return '&';
				case KEY_8: 
					return '*';
				case KEY_9: 
					return '(';
				default: 
					return '\0';
			}
		}
		else
		{
			// return numbers
			switch (key)
			{
				case KEY_0:
					return '0';
				case KEY_1:
					return '1';
				case KEY_2:
					return '2';
				case KEY_3:
					return '3';
				case KEY_4:
					return '4';
				case KEY_5:
					return '5';
				case KEY_6:
					return '6';
				case KEY_7:
					return '7';
				case KEY_8:
					return '8';
				case KEY_9:
					return '9';
			}
		}
	}

	// handle special symbols - check shift state first
	if (shift_state) 
	{
		switch (key) 
		{
			case KEY_SEMICOLON: 
				return ':';
			case KEY_QUOTE: 
				return '"';
			case KEY_COMMA: 
				return '<';
			case KEY_DOT: 
				return '>';
			case KEY_SLASH: 
				return '?';
			case KEY_MINUS: 
				return '_';
			case KEY_EQUAL: 
				return '+';
			case KEY_LEFTBRACKET: 
				return '{';
			case KEY_RIGHTBRACKET: 
				return '}';
			case KEY_BACKSLASH: 
				return '|';
			case KEY_GRAVE: 
				return '~';
		}
	}

	// handle unshifted symbols (normal state)
	switch (key) 
	{
		case KEY_SEMICOLON: 
			return ';';
		case KEY_QUOTE: 
			return '\'';
		case KEY_COMMA: 
			return ',';
		case KEY_DOT: 
			return '.';
		case KEY_SLASH: 
			return '/';
		case KEY_MINUS: 
			return '-';
		case KEY_EQUAL: 
			return '=';
		case KEY_LEFTBRACKET: 
			return '[';
		case KEY_RIGHTBRACKET: 
			return ']';
		case KEY_BACKSLASH: 
			return '\\';
		case KEY_GRAVE: 
			return '`';
	}

	// handle control characters
	switch (key)
	{
		case KEY_RETURN:
			return '\n';
		case KEY_KP_ENTER:
			return '\n';
		case KEY_TAB:
			return '\t';
		case KEY_BACKSPACE:
			return '\b';
		case KEY_SPACE:
			return ' ';
	}

	// handle numpad keys
	switch (key)
	{
		case KEY_KP_0: 
			return '0';
		case KEY_KP_1: 
			return '1';
		case KEY_KP_2: 
			return '2';
		case KEY_KP_3: 
			return '3';
		case KEY_KP_4: 
			return '4';
		case KEY_KP_5: 
			return '5';
		case KEY_KP_6: 
			return '6';
		case KEY_KP_7: 
			return '7';
		case KEY_KP_8: 
			return '8';
		case KEY_KP_9: 
			return '9';
		case KEY_KP_PLUS: 
			return '+';
		case KEY_KP_MINUS: 
			return '-';
		case KEY_KP_DECIMAL: 
			return '.';
		case KEY_KP_DIVIDE: 
			return '/';
		case KEY_KP_ASTERISK: 
			return '*';
	}

	// catch-all for unknown keys
	return '\0';
}

// helper functions
void _kbd_ring_buffer_clear(void)
{
	// clear the buffer
	for (int i = 0; i < KBD_RING_BUF_SIZE; i++)
	{
		_kbd_ring_buffer[i].keycode = KEY_UNKNOWN;
		_kbd_ring_buffer[i].ascii = '\0';
	}

	_kbd_ring_buffer_tail = 0;
	_kbd_ring_buffer_head = 0;
}

void clear_all_modifiers(void)
{
	shift_pressed_state = false;
	ctrl_pressed_state = false;
	alt_pressed_state = false;
	capslock_toggled_state = false;
	numlock_toggled_state = false;
	scrolllock_toggled_state = false;
}


// API definitions start here
void kbd_init(void)
{
	// clear the ring buffer
	_kbd_ring_buffer_clear();
	
	// clear all modifiers
	clear_all_modifiers();
	
	// register kbd_interrupt_handler
	register_interrupt_handler(IRQ1_KEYBOARD, (interrupt_service_t)kbd_interrupt_handler);
	
}

KBD_ENTRY kbd_getlastkey_buf()
{
    // wait until there is a key in the buffer
    while (_kbd_ring_buffer_empty()) 
	{
		
    }

	// return the last entered key from buffer and remove it
	return _kbd_ring_buffer_pop();
}

bool kbd_get_numlock()
{
	return numlock_toggled_state;
}

bool kbd_get_capslock()
{
	return capslock_toggled_state;
}

bool kbd_get_scrolllock()
{
	return scrolllock_toggled_state;
}

bool kbd_get_shift()
{
	return shift_pressed_state;
}

bool kbd_get_ctrl()
{
	return ctrl_pressed_state;
}

bool kbd_get_alt()
{
	return alt_pressed_state;
}



void kbd_interrupt_handler(interrupt_context_t *context)
{
	// get scancode from port
	uint8_t scancode = inb(KBD_ENC_INPUT_BUF);

	if (scancode == KBD_INVALID_SCANCODE)
		return;
	

		// toggled keys v/s pressed keys
		// 1. pressed: make code rcvd -> set status to true
		//			   break code rcvd -> set status to false
		// 2. toggled: make code rcvd AND status is false -> set to true
		// 			   break code rcvd -> do nothing
		//			   makw code rcvd AND status is true -> set status to false
		
		// get make or break status
		// break = make + 0x80 -> the highest bit is changed!
		// make -> highest bit is not set
		// break -> highest bit is set
		
	uint8_t bit_set = (scancode & KBD_SCANCODE_BREAK);
	bool released = (bit_set != 0); 

	// track modifier status

	// we're tracking the make bit, so if we get a break bit, we need to get the original scancode from it
	if (released)
	{
		// break code, get the original scancode
		scancode = (scancode & 0x7F);	// checked from LLM
	}

	// convert to keycode
	KBD_KEYCODE keycode = _kbd_scancode_set[scancode];

	if (keycode == KEY_UNKNOWN)
		return;

	// we now need to track the modifier status, based on the make/break principle
	if (released)
	{
		// break code, modify the press-keys' status
		switch (keycode)
		{
			// handle press keys: shift, ctrl, alt

			// handling shift
			case KEY_LSHIFT:
				shift_pressed_state = false;
				break;

			case KEY_RSHIFT:
				shift_pressed_state = false;
				break;

			// handle ctrl
			case KEY_LCTRL:
				ctrl_pressed_state = false;
				break;

			case KEY_RCTRL:
				ctrl_pressed_state = false;
				break;
			
			// handling alt
			case KEY_LALT:
				alt_pressed_state = false;
				break;

			case KEY_RALT:
				alt_pressed_state = false;
				break;

			// handle toggle keys - do nothing on break code
			case KEY_KP_NUMLOCK:
				break;

			case KEY_CAPSLOCK:
				break;

			case KEY_SCROLLLOCK:
				break;
		}
	}

	else
	{
		// make code:
		switch (keycode)
		{
			// a) modify press keys
			
			// handling shift
			case KEY_LSHIFT:
				shift_pressed_state = true;
				break;
				
			case KEY_RSHIFT:
				shift_pressed_state = true;
				break;
			
			// handling ctrl
			case KEY_LCTRL:
				ctrl_pressed_state = true;
				break;

			case KEY_RCTRL:
				ctrl_pressed_state = true;
				break;
			
			// handling alt
			case KEY_LALT:
				alt_pressed_state = true;
				break;

			case KEY_RALT:
				alt_pressed_state = true;
				break;
			
			// b) modify toggle keys -> invert their state using !
			
			// handling numlock
			case KEY_KP_NUMLOCK:
				numlock_toggled_state = !numlock_toggled_state;
				_kbd_ring_buffer_push(keycode);
				break;
			
			// handling capslock
			case KEY_CAPSLOCK:
				capslock_toggled_state = !capslock_toggled_state;
				_kbd_ring_buffer_push(keycode);
				break;
			
			// handling scrollock
			case KEY_SCROLLLOCK:
				scrolllock_toggled_state = !scrolllock_toggled_state;
				_kbd_ring_buffer_push(keycode);
				break;
			

			// default case -> psuh to buffer
			default:
				_kbd_ring_buffer_push(keycode);
				break;

			}
		}


}

void kbd_clear_buffer(void)
{
    _kbd_ring_buffer_clear();
}