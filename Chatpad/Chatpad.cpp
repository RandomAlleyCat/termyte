#include "Chatpad.h"  // Include the Chatpad library header

// Define bit masks for various modifier flags (each one represents a specific button)
static const uint8_t kShiftMask  = (1 << 0);  // Bit 0: Shift key mask
static const uint8_t kGreenMask  = (1 << 1);  // Bit 1: Green button mask
static const uint8_t kOrangeMask = (1 << 2);  // Bit 2: Orange button mask
static const uint8_t kPeopleMask = (1 << 3);  // Bit 3: People button mask

// Define messages (as byte arrays) to be sent to the Chatpad for initialization and to keep it awake
static const uint8_t kInitMessage[]  = { 0x87, 0x02, 0x8C, 0x1F, 0xCC };
static const uint8_t kAwakeMessage[] = { 0x87, 0x02, 0x8C, 0x1B, 0xD0 };


//-----------------------------------------------------------------
// Function: Chatpad::init
// Purpose: Initialize the Chatpad communication and set up initial state.
// Parameters:
//    - serial: Reference to a HardwareSerial object for communication
//    - callback: Function pointer to call when a key event occurs
//-----------------------------------------------------------------
void Chatpad::init(HardwareSerial &serial, Chatpad::callback_t callback) {
    _serial = &serial;          // Store pointer to the provided serial port
    _callback = callback;       // Store the key event callback
    _last_modifiers = 0;        // Clear last modifiers state
    _last_key0 = 0;             // Clear last key (first slot)
    _last_key1 = 0;             // Clear last key (second slot)
    _last_ping = 0;             // Initialize last ping time

    _serial->begin(19200);      // Start serial communication at 19200 baud
    _serial->write(kInitMessage, sizeof(kInitMessage));  // Send initialization message to Chatpad
}


//-----------------------------------------------------------------
// Function: Chatpad::poll
// Purpose: Poll the Chatpad for incoming data, validate and process packets,
//          handle modifier changes, dispatch key events, and send periodic keep‐alive pings.
//-----------------------------------------------------------------
void Chatpad::poll() {
    // Check if there is any data available on the serial port
    if (_serial->available()) {
        uint8_t byte = _serial->read();  // Read one byte from the serial stream

        // Look for valid packet start bytes (0xA5 or 0xB4)
        if (byte == 0xA5 || byte == 0xB4) {
            _buffer[0] = byte;  // Store the first byte in the buffer

            // Attempt to read the remaining 7 bytes with a short timeout instead of busy-waiting.
            _serial->setTimeout(10);  // Timeout in milliseconds
            size_t received = _serial->readBytes(reinterpret_cast<char*>(&_buffer[1]), 7);
            if (received != 7) {
                return;  // Incomplete packet
            }
        } else {
            return;  // If the first byte is not valid, exit the poll function
        }

        // If the packet starts with 0xA5, ignore it and return (it might be a status message)
        if (_buffer[0] == 0xA5) return;

        // Validate packet header: Packet must start with 0xB4 followed by 0xC5
        if (_buffer[0] != 0xB4 || _buffer[1] != 0xC5) {
            Serial.println("Unexpected packet type or header:");
            Serial.print("Packet: ");
            // Print the entire packet for debugging
            for (int i = 0; i < 8; i++) {
                Serial.print("0x");
                Serial.print(_buffer[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
            return;
        }

        // Compute checksum: sum of bytes 0 through 6, then take the negative (two's complement)
        uint8_t checksum = _buffer[0];
        for (int i = 1; i < 7; i++) {
            checksum += _buffer[i];
        }
        checksum = -checksum;

        // Compare computed checksum with the received checksum (byte 7)
        if (checksum != _buffer[7]) {
            Serial.println("Checksum failure");
            return;
        }

        // Extract modifier and key information from the packet
        uint8_t modifiers = _buffer[3];  // Byte 3 contains modifier flags
        uint8_t key0 = _buffer[4];       // Byte 4 contains first key code
        uint8_t key1 = _buffer[5];       // Byte 5 contains second key code

        // Determine which modifier bits have changed since last packet
        uint8_t modifier_changes = modifiers ^ _last_modifiers;

        // Check if the People button state has changed
        if (modifier_changes & kPeopleMask) {
            if (modifiers & kPeopleMask) {
                _peopleToggleMode = !_peopleToggleMode;  // Toggle People mode on press
                Serial.print("People button toggled mode: ");
                Serial.println(_peopleToggleMode ? "ON" : "OFF");
            }
        }
        
        // If both Shift and Orange modifiers are active, toggle the Shift mode
        if ((modifiers & kShiftMask) && (modifiers & kOrangeMask)) {
            _shiftToggled = !_shiftToggled;  // Toggle Shift mode
            Serial.print("Shift mode toggled: ");
            Serial.println(_shiftToggled ? "ON" : "OFF");
        }
        
        // Update the stored modifier state for the next poll cycle
        _last_modifiers = modifiers;

        // Dispatch key events based on changes between current keys and last keys:
        // If a new key (key0) is pressed and it is different from previous keys, send a "Down" event.
        if (key0 && key0 != _last_key0 && key0 != _last_key1) {
            dispatch(key0, Down);
        }
        // Do the same for key1.
        if (key1 && key1 != _last_key0 && key1 != _last_key1) {
            dispatch(key1, Down);
        }
        // If a previously pressed key (stored in _last_key0) is no longer present, send an "Up" event.
        if (_last_key0 && _last_key0 != key0 && _last_key0 != key1) {
            dispatch(_last_key0, Up);
        }
        // Similarly for _last_key1.
        if (_last_key1 && _last_key1 != key0 && _last_key1 != key1) {
            dispatch(_last_key1, Up);
        }

        // Update the stored key states for the next poll cycle
        _last_key0 = key0;
        _last_key1 = key1;
    }

    // Periodically send an "awake" message to the Chatpad every 1000 ms
    uint32_t time = millis();
    if (time - _last_ping > 1000) {
        _last_ping = time;
        _serial->write(kAwakeMessage, sizeof(kAwakeMessage));
    }
}


//-----------------------------------------------------------------
// Function: isPeopleModeToggled
// Purpose: Return whether People mode is toggled on
//-----------------------------------------------------------------
bool Chatpad::isPeopleModeToggled() const {
    return _peopleToggleMode;
}


//-----------------------------------------------------------------
// Function: isShiftDown
// Purpose: Return whether Shift is currently active (either toggled or held)
//-----------------------------------------------------------------
bool Chatpad::isShiftDown() const {
    return _shiftToggled || (_last_modifiers & kShiftMask);
}


//-----------------------------------------------------------------
// Function: isGreenDown
// Purpose: Return whether the Green button is currently pressed
//-----------------------------------------------------------------
bool Chatpad::isGreenDown() const {
    return _last_modifiers & kGreenMask;
}


//-----------------------------------------------------------------
// Function: isOrangeDown
// Purpose: Return whether the Orange button is currently pressed
//-----------------------------------------------------------------
bool Chatpad::isOrangeDown() const {
    return _last_modifiers & kOrangeMask;
}


//-----------------------------------------------------------------
// Function: isPeopleDown
// Purpose: Return whether the People button is currently pressed
//-----------------------------------------------------------------
bool Chatpad::isPeopleDown() const {
    return _last_modifiers & kPeopleMask;
}


//-----------------------------------------------------------------
// Function: dispatch
// Purpose: Call the callback function (if set) with a key event.
// Parameters:
//    - keycode: The key code for the event
//    - is_down: Non-zero value indicates a key press (Down), zero indicates release (Up)
//-----------------------------------------------------------------
void Chatpad::dispatch(uint8_t keycode, int is_down) {
    if (_callback) {  // If a callback function is registered
        _callback(*this, static_cast<keycode_t>(keycode), is_down ? Down : Up);
    }
}


//-----------------------------------------------------------------
// Function: toAscii
// Purpose: Convert a Chatpad keycode into an ASCII character using lookup tables.
//          The table used depends on the current modifier state (Shift, Green, Orange, or People mode).
// Parameters:
//    - keycode: The raw keycode from the Chatpad
// Returns:
//    - The corresponding ASCII character, or 0 if no mapping exists.
//-----------------------------------------------------------------
char Chatpad::toAscii(keycode_t keycode) {
    // Lookup tables stored in program memory (PROGMEM) for different modifier states.
    static const char kAsciiTable[] PROGMEM = {
        '7', '6', '5', '4', '3', '2', '1', 0,
        'u', 'y', 't', 'r', 'e', 'w', 'q', 0,
        'j', 'h', 'g', 'f', 'd', 's', 'a', 0,
        'n', 'b', 'v', 'c', 'x', 'z', 0, 0,
        128, 'm', '.', ' ', 130, 0, 0, 0,
        0, ',', '\n', 'p', '0', '9', '8', 0,
        '\b', 'l', 0, 0, 'o', 'i', 'k', 0,
    };

    static const char kAsciiTable_Shifted[] PROGMEM = {
        '&', '^', '%', '$', '#', '@', '!', 0,
        'U', 'Y', 'T', 'R', 'E', 'W', 'Q', 0,
        'J', 'H', 'G', 'F', 'D', 'S', 'A', 0,
        'N', 'B', 'V', 'C', 'X', 'Z', 0, 0,
        0, 'M', '?', ' ', 0, 0, 0, 0,
        0, 58, '\n', 'P', ')', '(', '*', 0,
        '\b', 'L', 0, 0, 'O', 'I', 'K', 0,
    };

    static const char kAsciiTable_Green[] PROGMEM = {
        '&', '^', '%', '$', '#', '@', '!', 0,
        '&', '^', '%', '#', 128, '@', '!', 0,
        39, 47, 168, 125, 123, 138, '~', 0,
        '<', '|', '-', 187, 171, 96, 0, 0,
        0, '>', '?', ' ', 0, 0, 0, 0,
        0, 58, '\n', ')', ')', '(', '*', 0,
        '\b', 93, 0, 0, '(', '*', 91, 0,
    };

    static const char kAsciiTable_Orange[] PROGMEM = {
       KEY_F7, KEY_F6, KEY_F5, KEY_F4, KEY_F3, KEY_F2, KEY_F1, 0,
       'U', 'Y', 'T', 'R', 'E', 'W', 'Q', 0,
       34, 92, 'G', 'F', 'D', 'S', 'A', 0,
       'N', 'B', '_', 'C', 'X', 'Z', 0, 0,
       9, 'M', '.', ' ', 177, 0, 0, 0,
       0, 59, '\n', '=', KEY_F10, KEY_F9, KEY_F8, 0,
       212, 'L', 0, 0, 'O', 'I', 'K', 0,
    };

    static const char kAsciiTable_PeopleMode[] PROGMEM = {
        KEY_F7, KEY_F6, KEY_F5, KEY_F4, KEY_F3, KEY_F2, KEY_F1, 0,
        'u', 'y', 't', 'r', 'e', 218, 'q', 0,
        'j', 'h', 'g', 'f', 215, 217, 216, 0,
        'n', 'b', 'v', 'c', 'x', 'z', 0, 0,
        9, 'm', '.', ' ', 177, 0, 0, 0,
        0, ',', '\n', 'p', KEY_F10, KEY_F9, KEY_F8, 0,
        '\b', 'l', 0, 0, 'o', 'i', 'k', 0,
    };

    // Compute the lookup table index from the raw keycode.
    // The calculation maps the keycode into an index that selects the correct character.
    uint8_t index = (((keycode - 0x11) & 0x70) >> 1) | ((keycode - 0x11) & 0x7);
    // Ensure the index is within bounds; if not, return 0 (no mapping).
if (index >= (sizeof(kAsciiTable) / sizeof(kAsciiTable[0])))
    return 0;

    // Select the appropriate lookup table based on the current modifier state:
    if (isPeopleModeToggled()) {
        return pgm_read_byte(&kAsciiTable_PeopleMode[index]);
    } else if (isOrangeDown()) {
        return pgm_read_byte(&kAsciiTable_Orange[index]);
    } else if (isGreenDown()) {
        return pgm_read_byte(&kAsciiTable_Green[index]);
    } else if (isShiftDown()) {
        return pgm_read_byte(&kAsciiTable_Shifted[index]);
    } else {
        return pgm_read_byte(&kAsciiTable[index]);
    }
}
