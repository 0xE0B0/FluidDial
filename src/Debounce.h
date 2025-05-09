/*
 * Debounce.h
 *
 * Created: 23.01.2024
 *  Author: Emanuel Balz
 *  based on debounce routine by Peter Dannegger (http://www.mikrocontroller.net/topic/48465)
 */

// Mode 0 ( get_key_press or  get_key_repeat)
// Mode 1 ( get_key_press with  get_key_repeat)
// =================================================
//              time ---->
//                   __      _________________      __
// key_state    ____/  \____/                 \____/  \____
// key_press    ----X-------X----------------------X-------
// key_repeat   --------------------X--X--X--X-------------
// key_release  -------X----------------------X-------X----
//                          |       |  |  |
//                          |       |__|__|
//                          |       | \ /
//                          |_______|  repeatNext
//                                |
//                                 repeatStart
// Mode 2 ( get_key_short with  get_key_long)
// =================================================
//                   __      _________________      __
// key_state    ____/  \____/                 \____/  \____
// key_short    -------X------------------------------X----
// key_long     --------------------X----------------------
// key_release  -------X----------------------X-------X----
// Mode 3 ( get_key_short with  get_key_long_r and  get_key_repeat_l)
// =========================================================================
//                   __      _________________      __
// key_state    ____/  \____/                 \____/  \____
// key_short    -------X------------------------------X----
// key_long_rpt --------------------X----------------------
// key_rpt_long -----------------------X--X--X-------------
// key_release  -------X----------------------X-------X----
// Note:  get_key_long_r and  get_key_rpt_l MUST always be used in
//       conjunction, even if only one of both functionalities is being used.
// The graphics were taken from http://www.mikrocontroller.net/topic/48465#1844458

#include <Arduino.h>

#pragma once

template <class T>
class Debounce {

    // number of ticks() until a long or repeated press is detected
    static constexpr uint16_t c_repeatStart = 500;
    // number of ticks() until the next repeated press is detected
    static constexpr uint16_t c_repeatNext = 200;

public:

    Debounce(T defaultState, T repeatMask, bool highActive=false) :
        _keyState(defaultState), _repeatMask(repeatMask), _polarity(highActive) { 
    };

    // debounce routine
    // note: must be called in a 100us...10ms interval
    // with the raw key input state as argument
    void tick(T rawKeys) {
        static T ct0 = -1, ct1 = -1;
        static uint16_t rpt;
        T in;
        if (_polarity)
            in = _keyState ^ rawKeys;
        else
            in = _keyState ^ ~rawKeys;
        ct0 = ~(ct0 & in);
        ct1 = ct0 ^ (ct1 & in);
        in &= ct0 & ct1;
        _keyState ^= in;
        _keyPress |= _keyState & in;
        _keyRelease |= ~_keyState & in;
        if ((_keyState & _repeatMask) == 0) {
            rpt = c_repeatStart;
        }
        if (--rpt == 0) {
            rpt = c_repeatNext;
            _keyRpt |= _keyState & _repeatMask;
        }
    }

    // read the current state of the debounced signals
    T getKeyState(T keyMask) {
        cli();
        keyMask &= _keyState;
        sei();
        return keyMask;
    }

    // check if a key was pressed since the last call of this function
    T getKeyPress(T keyMask) {
        cli();
        keyMask &= _keyPress;
        _keyPress ^= keyMask;
        sei();
        return keyMask;
    }

    // check if a key was released since the last call of this function
    T getKeyRelease(T keyMask) {
        cli();
        keyMask &= _keyRelease;
        _keyRelease ^= keyMask;
        sei();
        return keyMask;
    }

    // check if a key is held down/ pressed repeatedly.
    T getKeyRepeat(T keyMask) {
        cli();
        keyMask &= _keyRpt;
        _keyRpt ^= keyMask;
        sei();
        return keyMask;
    }

    // check if a short/ normal key press occurred
    T getKeyShort(T keyMask) {
        cli();
        keyMask = getKeyPress(~_keyState & keyMask);
        sei();
        return keyMask;
    }

    // check if a long key press occurred
    T getKeyLong(T keyMask) {
        return getKeyPress(getKeyRepeat(keyMask));
    }

    // check if two keys are being pressed simultaneously
    T getKeyCommon(T keyMask) {
        return getKeyPress((_keyPress & keyMask) == keyMask ? keyMask : 0);
    }

    // check if a key is held down/ pressed repeatedly
    // note: must be used in conjunction with getKeyLong_rpt()
    // if (getKeyLong_rpt(1<<KEY0)) LED1(ON);
    // if (getKeyRepeat_long(1<<KEY0))  LED2(ON);
    T getKeyLong_rpt(T keyMask) {
        return getKeyPress(getKeyRepeat(_keyPress & keyMask));
    }
    T getKeyRepeat_long(T keyMask) {
        return getKeyRepeat(~_keyPress & keyMask);
    }

private:
    // holds debounced and inverted state (key is pressed if bit is high)
    volatile T _keyState;
    // hold mask which keys should perform repeat feature
    volatile T _repeatMask;
    // holds key pressed state (0 => 1 occured if bit is high)
    volatile T _keyPress = 0;
    // holds key released state (1 => 0 occured if bit is high)
    volatile T _keyRelease = 0;
    // long key press and repetition state
    volatile T _keyRpt = 0;
    // polarity of key wiring (low = lowActive)
    bool _polarity;

};
