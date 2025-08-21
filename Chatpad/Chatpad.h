#ifndef CHATPAD_H
#define CHATPAD_H

#include <Arduino.h>
#include <Keyboard.h>

//-----------------------------------------------------------------
// Class: Chatpad
// Purpose: Interface with an Xbox Chatpad, process incoming key events,
//          convert raw keycodes into ASCII values, and dispatch key events
//          via a user-provided callback.
//-----------------------------------------------------------------
class Chatpad {
public:
    //-----------------------------------------------------------------
    // Enumeration: keycode_t
    // Purpose: List of raw keycodes as defined by the Chatpad hardware.
    //-----------------------------------------------------------------
    enum keycode_t {
        Key1 = 0x17, Key2 = 0x16, Key3 = 0x15, Key4 = 0x14, Key5 = 0x13,
        Key6 = 0x12, Key7 = 0x11, Key8 = 0x67, Key9 = 0x66, Key0 = 0x65,
        KeyQ = 0x27, KeyW = 0x26, KeyE = 0x25, KeyR = 0x24, KeyT = 0x23,
        KeyY = 0x22, KeyU = 0x21, KeyI = 0x76, KeyO = 0x75, KeyP = 0x64,
        KeyA = 0x37, KeyS = 0x36, KeyD = 0x35, KeyF = 0x34, KeyG = 0x33,
        KeyH = 0x32, KeyJ = 0x31, KeyK = 0x77, KeyL = 0x72, KeyComma = 0x62,
        KeyZ = 0x46, KeyX = 0x45, KeyC = 0x44, KeyV = 0x43, KeyB = 0x42,
        KeyN = 0x41, KeyM = 0x52, KeyPeriod = 0x53, KeyEnter = 0x63,
        KeyLeft = 0x55, KeySpace = 0x54, KeyRight = 0x51, KeyBackspace = 0x71,
        KeyShift = 0x81, KeyGreen = 0x82, KeyPeople = 0x83, KeyOrange = 0x84
    };

    //-----------------------------------------------------------------
    // Enumeration: eventtype_t
    // Purpose: Indicates whether a key event is a key press (Down) or release (Up).
    //-----------------------------------------------------------------
    enum eventtype_t {
        Up = 0,
        Down = 1
    };

    //-----------------------------------------------------------------
    // Typedef: callback_t
    // Purpose: Define a function pointer type for key event callbacks.
    //          The callback receives a reference to the Chatpad, the keycode,
    //          and the event type.
    //-----------------------------------------------------------------
    typedef void (*callback_t)(Chatpad &, keycode_t, eventtype_t);

    //-----------------------------------------------------------------
    // Function: init
    // Purpose: Initialize the Chatpad with a given HardwareSerial interface and
    //          register the callback function for key events.
    //-----------------------------------------------------------------
    void init(HardwareSerial &serial, callback_t callback);

    //-----------------------------------------------------------------
    // Function: poll
    // Purpose: Poll the underlying serial interface for incoming data,
    //          process complete packets, and dispatch key events.
    //-----------------------------------------------------------------
    void poll();

    //-----------------------------------------------------------------
    // Getter Functions:
    // Purpose: Check the current state of modifier keys.
    //-----------------------------------------------------------------
    bool isShiftDown() const;
    bool isGreenDown() const;
    bool isOrangeDown() const;
    bool isPeopleDown() const;
    bool isPeopleModeToggled() const;

    //-----------------------------------------------------------------
    // Function: toAscii
    // Purpose: Convert a raw keycode to its corresponding ASCII character
    //          based on the current modifier state.
    // Parameters:
    //    - keycode: A raw keycode value from the Chatpad.
    // Returns:
    //    - The ASCII character for that keycode, or 0 if unmapped.
    //-----------------------------------------------------------------
    char toAscii(keycode_t keycode);

private:
    //-----------------------------------------------------------------
    // Private Members:
    // _serial       : Pointer to the HardwareSerial interface used for communication.
    // _callback     : Function pointer for dispatching key events.
    // _buffer[8]    : Buffer for reading 8-byte packets from the Chatpad.
    // _last_modifiers: Last known modifier state.
    // _last_key0    : Last keycode received in slot 0.
    // _last_key1    : Last keycode received in slot 1.
    // _last_ping    : Timestamp of the last keep-alive message sent.
    // _peopleToggleMode: Tracks whether People mode is toggled.
    // _shiftToggled : Tracks whether Shift mode is toggled.
    //-----------------------------------------------------------------
    HardwareSerial *_serial;
    callback_t _callback;
    uint8_t _buffer[8];

    uint8_t _last_modifiers;
    uint8_t _last_key0;
    uint8_t _last_key1;
    uint32_t _last_ping;

    bool _peopleToggleMode = false;
    bool _shiftToggled = false;

    //-----------------------------------------------------------------
    // Function: dispatch
    // Purpose: Dispatch a key event to the registered callback.
    // Parameters:
    //    - keycode: The keycode to dispatch.
    //    - is_down: A non-zero value indicates a key press; zero indicates release.
    //-----------------------------------------------------------------
    void dispatch(uint8_t keycode, int is_down);
};

#endif // CHATPAD_H
