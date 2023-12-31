/* #region    || — — — — — — — — — — ||             INCLUDES            || — — — — — — — — — — — || */

#include <Arduino.h>
#include <Control_Surface.h>
#include <MIDI_Interfaces/USBHostMIDI_Interface.hpp>
// #include "MegunoLink.h"
// #include "Filter.h"

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||             GLOBALS             || — — — — — — — — — — — || */

/// Keypressure filter
// ExponentialFilter<long> keypressure_filter(50, 0);

/// Create ports for USB devices plugged into Teensy's 2nd USB port (via hubs):
USBHost usb;
USBHub hub{usb};

/// Create MIDI interfaces:
USBDebugMIDI_Interface dbgmidi = 115200;                    // Serial Debug
USBMIDI_Interface usbmidi;                                  // Device MIDI
USBHostMIDI_Interface hstmidi{usb};                         // USB Host MIDI
HardwareSerialMIDI_Interface dinmidi = {Serial1, 31250};    // 5-Pin DIN

/// Device inquiry:
// Device inquiry:
uint8_t lpx_device_inquiry[6] = { 240, 126, 127, 6, 1, 247 };  
// uint8_t respons_if_lpx_is_connected[] = { 240, 126, 0, 6, 2, 0, 32, 41, 19, 1, 0, 0, X, X, X, X, 247 };
// F0h 7Eh 00h 06h 02h 00h 20h 29h 13h 01h 00h 00h <app_version> F7h
uint8_t respons_if_lpx_is_connected_test[12] = { 240, 126, 0, 6, 2, 0, 32, 41, 19, 1, 0, 0 };
// uint8_t respons_if_lpx_is_connected_test[] = { F0, 7E, 00, 06, 02, 00, 20, 29, 13, 01, 00, 00, };
/// Mode:
bool    mode_keyboard                 = true;
bool    mode_drum                     = false;

/// Pad refresh bool...
bool refresh_pads = false;

int layout_difference = 0;

/// Constants:
uint8_t cc_buttons[]                   = { 91, 92, 96, 97 };
uint8_t transport_buttons[]            = { 89, 79, 69, 59, 49, 39, 29, 19 };
uint8_t lpx_color_white[3]             = { 89, 100, 127 };

// // Test if we really need ALL pads in here... Could we do without the pals in this array for example?
uint8_t every_single_pad[64]                        = { 11, 12, 13, 14, 15, 16, 17, 18, 21, 22, 23, 24, 25, 26, 27, 28, 31, 32, 33, 34, 35, 36, 37, 38, 41, 42, 43, 44, 45, 46, 47, 48, 51, 52, 53, 54, 55, 56, 57, 58, 61, 62, 63, 64, 65, 66, 67, 68, 71, 72, 73, 74, 75, 76, 77, 78, 81, 82, 83, 84, 85, 86, 87, 88 };
uint8_t every_pad_without_pals[43]                  = { 11, 12, 13, 14, 15, 21, 22, 23, 24, 25, 31, 32, 33, 34, 35, 41, 42, 43, 44, 45, 51, 52, 53, 54, 55, 61, 62, 63, 64, 65, 71, 72, 73, 74, 75, 81, 82, 83, 84, 85, 86, 87, 88 };
Channel all_midi_channels[16]          = { CHANNEL_1, CHANNEL_2, CHANNEL_3, CHANNEL_4, CHANNEL_5, CHANNEL_6, CHANNEL_7, CHANNEL_8, CHANNEL_9, CHANNEL_10, CHANNEL_11, CHANNEL_12, CHANNEL_13, CHANNEL_14, CHANNEL_15, CHANNEL_16 };

/// Variables:
uint8_t white_keys[40]                 = { 11, 12, 14, 16, 18, 21, 23, 24, 26, 28, 31, 33, 34, 36, 38, 41, 43, 45, 46, 48, 51, 53, 55, 56, 58, 61, 63, 65, 67, 68, 72, 73, 75, 77, 78, 82, 83, 85, 87,255 };

/// Pool
uint8_t pad_pool[16]                   = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t note_pool[16]                  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t velocity_pool[16]              = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t pad_transposed[16]             = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t drum_pool[16]                  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

uint8_t lpx_midi_channel               = 1;

int     octave_shift_amount_selector   = 3;
int     octave_shift[7]                = { -36, -24, -12, 0, 12, 24, 36 };

uint8_t key_transpose_selector_pad     = 0;
int     key_transpose                  = 0;

int     pad_layout_shift               = 4;
char    lowest_note_char[7]            = { 'E', ' ', ' ', ' ', ' ', ' ', ' ' };

/// Color button:
bool    color_palette_button_pressed   = false;
int     color_on_selector_value        = 2;
uint8_t color_on_selector_pad          = 71;
bool    color_test_button_pressed      = false;

/// Transpose buttons:
bool left_arrow_pressed                = false;
bool right_arrow_pressed               = false;
bool up_arrow_pressed                  = false;
bool down_arrow_pressed                = false;
bool key_transpose_button_pressed      = false;
bool custom_button_pressed             = false;

/// Timeing:
unsigned long myTime_1;
unsigned long myTime_2;

/// Channel selection
bool    midi_channel_selector_button_pressed = false;
uint8_t midi_channel_selector_int = 3;

/// Drum pads & Drum colors:

// Snare
uint8_t drum_pad_color_1_w[] = { 63, 64, 53, 54, 65, 66, 55, 56 };
uint8_t drum_color_1_w[] =     { 127, 127, 127 };

// Clap
uint8_t drum_pad_color_2_w[] = { 61, 62, 51, 52, 67, 68, 57, 58 };
uint8_t drum_color_2_w[] =     { 127, 127, 64 };

// Cymbals
uint8_t drum_pad_color_3_y[] = { 87, 88, 77, 78,     41, 42, 31, 32, 47, 48, 37, 38,     21, 22, 11, 12, 27, 28, 17, 18};
uint8_t drum_color_3_y[] =     {127, 127, 0};

// Closed HiHat
uint8_t drum_pad_color_4_y[] = { 43, 44, 33, 34, 45, 46, 35, 36 };
uint8_t drum_color_4_y[] =     { 127, 80, 0 };

// Tom 1
uint8_t drum_pad_color_5_o[] = { 85, 86, 75, 76 };
uint8_t drum_color_5_o[] =     { 127, 64, 0 };

// Tom 2
uint8_t drum_pad_color_6_o[] = { 83, 84, 73, 74 };
uint8_t drum_color_6_o[] =     { 127, 32, 0 };

// Tom 3
uint8_t drum_pad_color_7_o[] = { 81, 82, 71, 72 };
uint8_t drum_color_7_o[] =     { 127, 16, 0 };

// Kick
uint8_t drum_pad_color_8_r[] = { 23, 24, 13, 14, 25, 26, 15, 16 };
uint8_t drum_color_8_r[] =     { 127, 0, 0 };

/// Animation:
const unsigned long a_time             = 16;
bool animation_in_progress             = false;
bool animation_1_in_progress           = false;
bool animation_2_in_progress           = false;
bool animation_3_in_progress           = false;
bool animation_4_in_progress           = false;
bool animation_5_in_progress           = false;
bool animation_6_in_progress           = false;

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||            FUNCTIONS            || — — — — — — — — — — — || */

// TODO: Remap mc-101 filter (9-o-clock is filter off atm and 15-o-clock is filter completely open...)

void pot_1(){

  uint8_t inststrument_filter_cc = 127 - (analog16.getRawValue() * 128 / analog16.getMaxRawValue());

  Serial << "   midi out: " << inststrument_filter_cc << "   analog.getValue: " << analog16.getValue() << "     raw: " << analog16.getRawValue() << endl;
  Serial << endl;

  if (analog16.getRawValue() > 65400) { inststrument_filter_cc = 0; }

  MIDIAddress midifiltinst{ 74 , get_current_midi_channel() };

  usbmidi.sendControlChange(midifiltinst, inststrument_filter_cc); // send to DAW
  dinmidi.sendControlChange(midifiltinst, inststrument_filter_cc); // send to DIN

}

void pot_2(){

  uint8_t master_filter_cc = 127 - (analog17.getRawValue() * 128 / analog17.getMaxRawValue());

  MIDIAddress midifiltmast{ 74 , CHANNEL_16 };
  usbmidi.sendControlChange(midifiltmast, master_filter_cc); // send to DAW
  dinmidi.sendControlChange(midifiltmast, master_filter_cc); // send to DIN

}

void pot_3(){

  uint16_t pitch_bend_full = (analog15.getRawValue() * 16385 / analog15.getMaxRawValue()) + (analog14.getRawValue() * 1366 / analog14.getMaxRawValue());
  // MIDIAddress address{ pb , get_current_midi_channel() };
  usbmidi.sendPitchBend(get_current_midi_channel(), pitch_bend_full); // send to DAW
  dinmidi.sendPitchBend(get_current_midi_channel(), pitch_bend_full); // send to DIN

}

void refresh_all_pads_function(){

  if (refresh_pads == true) {

    if (key_transpose_button_pressed == false && custom_button_pressed == false) {
      // Light the control change buttons, accordingly to their default state:
      lpx_sysex_light_cc_buttons();

      // Color all 9 buttons off.
      uint8_t cc_9_pads[8] = { 19,29,39,49,59,69,79,89 };
      for (uint8_t i = 0; i < 8; i++) {
        uint8_t lpx_pad_sysex[] = { 240, 0, 32, 41, 2, 12, 3, 0, cc_9_pads[i], 0, 247 };
        hstmidi.sendSysEx(lpx_pad_sysex);
      }
    }


    /* #region    || — — — — — — — — — — ||           REFRESH KEYBOARD          || — — — — — — — — — — — || */

    if (mode_keyboard == true) {

      // Get / Set the current layout for the white keys:
      white_key_layouts();

      // Are there any pads in the pool? If so, correct them to the new layout:
      for (uint8_t i = 0; i < 16; i++) {
        if (pad_pool[i] != 0) {
          // Change the value of the pals in the pool so they will react to any pads being released
          if (pad_transposed[i] == 0) pad_transposed[i] = pad_pool[i] + -layout_difference;
          else pad_transposed[i] = pad_transposed[i] + -layout_difference;

          /* #region    || — — — — — — — — — — ||    PAD_TRANSPOSED ROW-WRAP      || — — — — — — — — — — — || */

          if (pad_transposed[i] == 100) { pad_transposed[i] = 88; }
          if (pad_transposed[i] == 80)  { pad_transposed[i] = 75; }
          if (pad_transposed[i] == 70)  { pad_transposed[i] = 65; }
          if (pad_transposed[i] == 60)  { pad_transposed[i] = 55; }
          if (pad_transposed[i] == 50)  { pad_transposed[i] = 45; }
          if (pad_transposed[i] == 40)  { pad_transposed[i] = 35; }
          if (pad_transposed[i] == 30)  { pad_transposed[i] = 25; }
          if (pad_transposed[i] == 20)  { pad_transposed[i] = 15; }

          if (pad_transposed[i] == 89) { pad_transposed[i] = 101; }
          if (pad_transposed[i] == 79) { pad_transposed[i] = 84;  }
          if (pad_transposed[i] == 69) { pad_transposed[i] = 74;  }
          if (pad_transposed[i] == 59) { pad_transposed[i] = 64;  }
          if (pad_transposed[i] == 49) { pad_transposed[i] = 54;  }
          if (pad_transposed[i] == 39) { pad_transposed[i] = 44;  }
          if (pad_transposed[i] == 29) { pad_transposed[i] = 34;  }
          if (pad_transposed[i] == 19) { pad_transposed[i] = 24;  }

          /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

          // Color the "new" on-pads:
          send_led_sysex_to_launchpad(pad_transposed[i], lpx_r(velocity_pool[i]), lpx_g(velocity_pool[i]), lpx_b(velocity_pool[i]));

          }
        }

      // If the "note transpose screen" is active, don't "color over" that the screen...
      // int pads_to_check = 64;
      // if (key_transpose_button_pressed == true) pads_to_check = 40;
      int pads_to_check = 43;
      if (key_transpose_button_pressed == true) pads_to_check = 25;
      // if (key_transpose_button_pressed == true) pads_to_check = 28;

      // Color the pads after the new layout // Go through all of the 64 pads... (or 40 pads if any 3-row-”screen” is open...) Not 64, we already check for pals in the coloring function, 43.
      for (uint8_t i = 0; i < pads_to_check; i++) {

        // Check if the note is "out of bounds" (If the note is not within midi range):
        bool note_in_midi_range = true;
        if (pad_to_midi_processing_table(every_pad_without_pals[i]) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose < 0 || pad_to_midi_processing_table(every_pad_without_pals[i]) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose > 127) {
          note_in_midi_range = false;
        }

        // Check if any of them are in one of the 16 note pool slots •
        bool pad_in_pool = false;

        // Check if any of them are in one of the 16 note pool slots •
        for (uint8_t j = 0; j < 16; j++) {
          if (every_pad_without_pals[i] == pad_transposed[j]) {
            pad_in_pool = true;
            break; // There can be only one...
          }
        }

        // Check if any of them is a pad pal...
        for (uint8_t j = 0; j < 16; j++) {
          if (every_pad_without_pals[i] == pad_pals(pad_transposed[j])) {
            pad_in_pool = true;
            break; // We only need one match...
          }
        }

        // If the pad is not in the pool, and it is in midi range, go ahead and color:
        if (pad_in_pool == false && note_in_midi_range == true) {

          // Check if it is a white or black key, if it is one of the 40 white keys
          bool pad_is_white = false;
          for (uint8_t j = 0; j < 40; j++) {
            if (every_pad_without_pals[i] == white_keys[j]) {
              pad_is_white = true;
              break; // No need to check for more then one match:
            }
          }

          if (pad_is_white == true) {
            // Maybe we don't need to color EVERY 64 pad, we're already checking for pals in the coloring funcion.
            send_led_sysex_to_launchpad(every_pad_without_pals[i], lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]);
          } else {
            // Color the black pads black
            send_led_sysex_to_launchpad(every_pad_without_pals[i], 0, 0, 0);
          }
        }

        // If the note is "out of bounds" (not in midi range), color it with a "warning color":
        if (note_in_midi_range == false) {
          send_led_sysex_to_launchpad(every_pad_without_pals[i], 8, 0, 0);
        }
      }
    }
    /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

    /* #region    || — — — — — — — — — ||           DRUM MODE COLORING            || — — — — — — — || */

    if (mode_drum == true) {

      int pads_to_check = 64;
      // If the "note transpose screen" is active, don't "color over" that the screen...
      if (key_transpose_button_pressed == true) pads_to_check = 40;

      for (uint8_t i = 0; i < pads_to_check; i++) {
        led_drum_note_off(every_single_pad[i]);
      }
    }
    /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */
  }
}


void clear_pool(){
  for (uint8_t i = 0; i < 16; i++) {
    pad_pool[i]       = 0;
    note_pool[i]      = 0;
    velocity_pool[i]  = 0;
    pad_transposed[i] = 0;
    drum_pool[i]      = 0;
  }
}

bool mode_toggle(){

  if (mode_drum == false) {
    mode_drum = true;
    mode_keyboard = false;
    send_led_sysex_to_one_pad(29, 127, 64, 0);
    return true;
  }

  if (mode_drum == true) {
    mode_drum = false;
    mode_keyboard = true;
    send_led_sysex_to_one_pad(29, 8, 2, 0);
    return false;
  }

  return false;

}

/* #region    || — — — — — — — — — — ||            UNSORTED... FUNCTIONS            || — — — — — — — — — — — || */

MIDIAddress attach_midi_channel_to_note(uint8_t note) { 

  if (mode_drum == true) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_10 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 1) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_1 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 2) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_2 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 3) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_3 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 4) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_4 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 5) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_5 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 6) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_6 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 7) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_7 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 8) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_8 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 9) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_9 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 10) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_10 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 11) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_11 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 12) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_12 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 13) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_13 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 14) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_14 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 15) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_15 };
    return midiaddress_to_din;
  }

  if (midi_channel_selector_int == 16) {
    MIDIAddress midiaddress_to_din{ note, CHANNEL_16 };
    return midiaddress_to_din;
  }

};

Channel get_current_midi_channel() { 
  if (midi_channel_selector_int == 1)  { return CHANNEL_1;  }
  if (midi_channel_selector_int == 2)  { return CHANNEL_2;  }
  if (midi_channel_selector_int == 3)  { return CHANNEL_3;  }
  if (midi_channel_selector_int == 4)  { return CHANNEL_4;  }
  if (midi_channel_selector_int == 5)  { return CHANNEL_5;  }
  if (midi_channel_selector_int == 6)  { return CHANNEL_6;  }
  if (midi_channel_selector_int == 7)  { return CHANNEL_7;  }
  if (midi_channel_selector_int == 8)  { return CHANNEL_8;  }
  if (midi_channel_selector_int == 9)  { return CHANNEL_9;  }
  if (midi_channel_selector_int == 10) { return CHANNEL_10; }
  if (midi_channel_selector_int == 11) { return CHANNEL_11; }
  if (midi_channel_selector_int == 12) { return CHANNEL_12; }
  if (midi_channel_selector_int == 13) { return CHANNEL_13; }
  if (midi_channel_selector_int == 14) { return CHANNEL_14; }
  if (midi_channel_selector_int == 15) { return CHANNEL_15; }
  if (midi_channel_selector_int == 16) { return CHANNEL_16; }
};

void lpx_sysex_cc_button_default(uint8_t pad) {

  uint8_t lpx_sysex[] = { 240, 0, 32, 41, 2, 12, 3, 3, pad, 4, 4, 4, 247 };
  hstmidi.sendSysEx(lpx_sysex);

}

void lpx_sysex_cc_button_white(uint8_t pad){

  uint8_t lpx_sysex[] = { 240, 0, 32, 41, 2, 12, 3, 3, pad, 127, 127, 127, 247 };
  hstmidi.sendSysEx(lpx_sysex);
  
}

void lpx_sysex_light_cc_buttons(){
  // Light the control change buttons, accordingly to their default state:
  for (uint8_t i = 0; i < (sizeof(cc_buttons) / sizeof(cc_buttons[0])); i++) {
    uint8_t lpx_led_cc_buttons[] = { 240, 0, 32, 41, 2, 12, 3, 3, cc_buttons[i], 4, 4, 4, 247 };
    hstmidi.sendSysEx(lpx_led_cc_buttons);
  };
}


void refresh_a_pad(uint8_t pad){

  if (mode_keyboard == true) {

    // Is the pad in the pool? If so, give it an on color:
    bool pad_in_pool = false;
    for (uint8_t i = 0; i < 16; i++) {
      if (pad == pad_pool[i]) {
        pad_in_pool = true;
        uint8_t on[] = { 240, 0, 32, 41, 2, 12, 3, 3, pad, lpx_r(velocity_pool[i]), lpx_g(velocity_pool[i]), lpx_b(velocity_pool[i]), 247 };
        hstmidi.sendSysEx(on);
      }
    }

    // If the pad is not in the pool, it's black or white:
    if (pad_in_pool == false) {

      // Check if it is a white or black key, if it is one of the 40 white keys
      bool pad_is_white = false;
      for (uint8_t i = 0; i < 40; i++) {
        if (pad == white_keys[i]) {
          pad_is_white = true;
          break; // No need to check for more then one match:
        }
      }

      if (pad_is_white == true) {
        uint8_t white[] = { 240, 0, 32, 41, 2, 12, 3, 3, pad, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2], 247 };
        hstmidi.sendSysEx(white);
      } else {
        uint8_t black[] = { 240, 0, 32, 41, 2, 12, 3, 3, pad, 0, 0, 0, 247 };
        hstmidi.sendSysEx(black);
      }
    }
  }

  if (mode_drum == true) {
  
    led_drum_note_on(pad);
  
  }

}

void send_led_sysex_to_launchpad(uint8_t pad, uint8_t r, uint8_t g, uint8_t b) {
  
  // Send sysex to launchpad:
  uint8_t lpx_pad_sysex[] = { 240, 0, 32, 41, 2, 12, 3, 3, pad, r, g, b, 247 };
  hstmidi.sendSysEx(lpx_pad_sysex);

  // Check if the pad has a pad_pal, if there is a pad_pal, send the sysex for the pal aswell!
  uint8_t pal = pad_pals(pad);
  if (pal != 0) {
    uint8_t lpx_pal_sysex[] = { 240, 0, 32, 41, 2, 12, 3, 3, pal, r, g, b, 247 };
    hstmidi.sendSysEx(lpx_pal_sysex);
  }

}



uint8_t led_drum_note_on(uint8_t pad) {

  // Don't "color over" any screens...
  if (key_transpose_button_pressed == false || pad < 59) {

    // Crash, Ride & Hihat
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_3_y) / sizeof(drum_pad_color_3_y[0])); j++) {
      if (pad == drum_pad_color_3_y[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_3_y[0], drum_color_3_y[1], drum_color_3_y[2]);
        return 0;
        break; // No need to check for more then one match:
        }
      }

    // Kick
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_8_r) / sizeof(drum_pad_color_8_r[0])); j++) {
      if (pad == drum_pad_color_8_r[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_8_r[0], drum_color_8_r[1], drum_color_8_r[2]);
        return 0;
        break; // No need to check for more then one match:
        }
      }

    // Snare
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_1_w) / sizeof(drum_pad_color_1_w[0])); j++) {
      if (pad == drum_pad_color_1_w[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_1_w[0], drum_color_1_w[1], drum_color_1_w[2]);
        return 0;
        break; // No need to check for more then one match:
        }
      }

    // Open Hihat
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_4_y) / sizeof(drum_pad_color_4_y[0])); j++) {
      if (pad == drum_pad_color_4_y[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_4_y[0], drum_color_4_y[1], drum_color_4_y[2]);
        return 0;
        break; // No need to check for more then one match:
        }
      }

    // Clap
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_2_w) / sizeof(drum_pad_color_2_w[0])); j++) {
      if (pad == drum_pad_color_2_w[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_2_w[0], drum_color_2_w[1], drum_color_2_w[2]);
        return 0;
        break; // No need to check for more then one match:
        }
      }

    // Low Tom
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_7_o) / sizeof(drum_pad_color_7_o[0])); j++) {
      if (pad == drum_pad_color_7_o[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_7_o[0], drum_color_7_o[1], drum_color_7_o[2]);
        return 0;
        break; // No need to check for more then one match:
        }
      }

    // Medium Tom
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_6_o) / sizeof(drum_pad_color_6_o[0])); j++) {
      if (pad == drum_pad_color_6_o[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_6_o[0], drum_color_6_o[1], drum_color_6_o[2]);
        return 0;
        break; // No need to check for more then one match:
        }
      }

    // High Tom
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_5_o) / sizeof(drum_pad_color_5_o[0])); j++) {
      if (pad == drum_pad_color_5_o[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_5_o[0], drum_color_5_o[1], drum_color_5_o[2]);
        return 0;
        break; // No need to check for more then one match:
        }
      }
    return 0;
    }
}

uint8_t led_drum_note_off(uint8_t pad) {
  
  // Don't "color over" any screens...
  if (key_transpose_button_pressed == false || pad < 59) {

    // Crash, Ride & Hihat
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_3_y) / sizeof(drum_pad_color_3_y[0])); j++) {
      if (pad == drum_pad_color_3_y[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_3_y[0] * 0.46 , drum_color_3_y[1] * 0.46 , drum_color_3_y[2] * 0.46 );
        return 0;
        break; // No need to check for more then one match:
      }
    }

    // Kick
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_8_r) / sizeof(drum_pad_color_8_r[0])); j++) {
      if (pad == drum_pad_color_8_r[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_8_r[0] * 0.46 , drum_color_8_r[1] * 0.46 , drum_color_8_r[2] * 0.46 );
        return 0;
        break; // No need to check for more then one match:
      }
    }

    // Snare
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_1_w) / sizeof(drum_pad_color_1_w[0])); j++) {
      if (pad == drum_pad_color_1_w[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_1_w[0] * 0.46 , drum_color_1_w[1] * 0.46 , drum_color_1_w[2] * 0.46 );
        return 0;
        break; // No need to check for more then one match:
      }
    }

    // Open Hihat
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_4_y) / sizeof(drum_pad_color_4_y[0])); j++) {
      if (pad == drum_pad_color_4_y[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_4_y[0] * 0.46 , drum_color_4_y[1] * 0.46 , drum_color_4_y[2] * 0.46 );
        return 0;
        break; // No need to check for more then one match:
      }
    }

    // Clap
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_2_w) / sizeof(drum_pad_color_2_w[0])); j++) {
      if (pad == drum_pad_color_2_w[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_2_w[0] * 0.46 , drum_color_2_w[1] * 0.46 , drum_color_2_w[2] * 0.46 );
        return 0;
        break; // No need to check for more then one match:
      }
    }

    // Low Tom
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_7_o) / sizeof(drum_pad_color_7_o[0])); j++) {
      if (pad == drum_pad_color_7_o[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_7_o[0] * 0.46 , drum_color_7_o[1] * 0.46 , drum_color_7_o[2] * 0.46 );
        return 0;
        break; // No need to check for more then one match:
      }
    }

    // Medium Tom
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_6_o) / sizeof(drum_pad_color_6_o[0])); j++) {
      if (pad == drum_pad_color_6_o[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_6_o[0] * 0.46 , drum_color_6_o[1] * 0.46 , drum_color_6_o[2] * 0.46 );
        return 0;
        break; // No need to check for more then one match:
      }
    }

    // High Tom
    for (uint8_t j = 0; j < (sizeof(drum_pad_color_5_o) / sizeof(drum_pad_color_5_o[0])); j++) {
      if (pad == drum_pad_color_5_o[j]) {
        send_led_sysex_to_one_pad(pad, drum_color_5_o[0] * 0.46 , drum_color_5_o[1] * 0.46 , drum_color_5_o[2] * 0.46 );
        return 0;
        break; // No need to check for more then one match:
      }
    }
    return 0;
  }


  // - - - -- - - - - - - -


  // for (uint8_t i = 0; i < (sizeof(drum_pad_color_3_y) / sizeof(drum_pad_color_3_y[0])); i++) {
  //   if (pad == drum_pad_color_3_y[i]) {
  //     send_led_sysex_to_one_pad(pad, drum_color_3_y[0], drum_color_3_y[1], drum_color_3_y[2]); 
  //     return 0;
  //     break;
  //   }
  // }
  // 
  // for (uint8_t i = 0; i < (sizeof(drum_pad_color_8_r) / sizeof(drum_pad_color_8_r[0])); i++) {
  //   if (pad == drum_pad_color_8_r[i]) {
  //     send_led_sysex_to_one_pad(pad, drum_color_8_r[0], drum_color_8_r[1], drum_color_8_r[2]); 
  //     return 0;
  //     break;
  //   }
  // }
  // 
  // for (uint8_t i = 0; i < (sizeof(drum_pad_color_1_w) / sizeof(drum_pad_color_1_w[0])); i++) {
  //   if (pad == drum_pad_color_1_w[i]) {
  //     send_led_sysex_to_one_pad(pad, drum_color_1_w[0], drum_color_1_w[1], drum_color_1_w[2]); 
  //     return 0;
  //     break;
  //   }
  // }
  // 
  // for (uint8_t i = 0; i < (sizeof(drum_pad_color_5_o) / sizeof(drum_pad_color_5_o[0])); i++) {
  //   if (pad == drum_pad_color_5_o[i]) {
  //     send_led_sysex_to_one_pad(pad, drum_color_5_o[0], drum_color_5_o[1], drum_color_5_o[2]); 
  //     return 0;
  //     break;
  //   }
  // }

  return 0;

}

void send_led_sysex_to_one_pad(uint8_t pad, uint8_t r, uint8_t g, uint8_t b) {

  // Send sysex to launchpad:
  uint8_t lpx_pad_sysex[] = { 240, 0, 32, 41, 2, 12, 3, 3, pad, r, g, b, 247 };
  hstmidi.sendSysEx(lpx_pad_sysex);

  }

void send_midi_note_on ( uint8_t note, uint8_t velocity ) {
  usbmidi.sendNoteOn(attach_midi_channel_to_note(note), velocity);
  dinmidi.sendNoteOn(attach_midi_channel_to_note(note), velocity);
}

void replace_old_midi_note_in_pool_with_new(uint8_t pad, uint8_t note, uint8_t velocity) {

  // ...send midi off first...
  usbmidi.sendNoteOff(attach_midi_channel_to_note(note), 0);

  // ...DIN...
  // const MIDIAddress noteAddress{ MIDI_Notes::C(4), CHANNEL_1 };
  // MIDIAddress note_to_din1{ note, lpx_midi_channel };
  dinmidi.sendNoteOff(attach_midi_channel_to_note(note), 0);
  //dinmidi.sendNoteOff(note, 0);

  // ...then midi on, to get the new velocity sent...
  send_midi_note_on(note, velocity);

  // ...set the new color for the LED to match the velocity...
  send_led_sysex_to_launchpad(pad, lpx_r(velocity), lpx_g(velocity), lpx_b(velocity));

}

void send_drum_midi_note_on(uint8_t pad, uint8_t velocity){

  // usbmidi.sendNoteOn(pad_to_midi_processing_table_drum_edition(pad), CHANNEL_10, velocity);
  // dinmidi.sendNoteOn(pad_to_midi_processing_table_drum_edition(pad), CHANNEL_10, velocity);

  usbmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity);
  dinmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity);

  // if tom_1, send both tr-8 and mc-101 notes
  if (pad_to_midi_processing_table_drum_edition(pad) == 50) { 
    usbmidi.sendNoteOn(attach_midi_channel_to_note(48), velocity);
    dinmidi.sendNoteOn(attach_midi_channel_to_note(48), velocity);
  }

  // if tom_2, send both tr-8 and mc-101 notes
  if (pad_to_midi_processing_table_drum_edition(pad) == 47) {
    usbmidi.sendNoteOn(attach_midi_channel_to_note(45), velocity);
    dinmidi.sendNoteOn(attach_midi_channel_to_note(45), velocity);
  }

  // if tom_3, send both tr-8 and mc-101 notes
  if (pad_to_midi_processing_table_drum_edition(pad) == 43) {
    usbmidi.sendNoteOn(attach_midi_channel_to_note(41), velocity);
    dinmidi.sendNoteOn(attach_midi_channel_to_note(41), velocity);
  }
  
  // send_led_sysex_to_one_pad(pad, lpx_r(velocity), lpx_g(velocity), lpx_b(velocity));

  led_drum_note_on(pad);
  


  // // SEND DRUMPAD MIDI NOTE_ON:
  // big_drum_pad_table(pad);
  // usbmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(drum_cell_1)), velocity);
  // dinmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(drum_cell_1)), velocity);
  // send_led_sysex_to_one_pad(drum_cell_1, lpx_r(velocity), lpx_g(velocity), lpx_b(velocity));
  // send_led_sysex_to_one_pad(drum_cell_2, lpx_r(velocity), lpx_g(velocity), lpx_b(velocity));
  // send_led_sysex_to_one_pad(drum_cell_3, lpx_r(velocity), lpx_g(velocity), lpx_b(velocity));
  // send_led_sysex_to_one_pad(drum_cell_4, lpx_r(velocity), lpx_g(velocity), lpx_b(velocity));
}

void send_drum_midi_note_off(uint8_t pad, uint8_t velocity) {

  usbmidi.sendNoteOff(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), 0);
  dinmidi.sendNoteOff(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), 0);

  // if tom_1, send both tr-8 and mc-101 notes
  if (pad_to_midi_processing_table_drum_edition(pad) == 50) {
    usbmidi.sendNoteOff(attach_midi_channel_to_note(48), 0);
    dinmidi.sendNoteOff(attach_midi_channel_to_note(48), 0);
  }

  // if tom_2, send both tr-8 and mc-101 notes
  if (pad_to_midi_processing_table_drum_edition(pad) == 47) {
    usbmidi.sendNoteOff(attach_midi_channel_to_note(45), 0);
    dinmidi.sendNoteOff(attach_midi_channel_to_note(45), 0);
  }

  // if tom_3, send both tr-8 and mc-101 notes
  if (pad_to_midi_processing_table_drum_edition(pad) == 43) {
    usbmidi.sendNoteOff(attach_midi_channel_to_note(41), 0);
    dinmidi.sendNoteOff(attach_midi_channel_to_note(41), 0);
  }

  led_drum_note_off(pad);

}

uint8_t lpx_r(uint8_t velocity) { // Calculate red-   color-value from velocity: 
  uint8_t R = 127;
  if (color_on_selector_value == 0) { R = 127 * 0.75 - velocity; }
  if (color_on_selector_value == 1) { R = 127 - velocity * 0.5; }
  if (color_on_selector_value == 2) { R = 127 - velocity * 0.5; }
  if (mode_drum == true)            { R = velocity; }
  return R;
};

uint8_t lpx_g(uint8_t velocity) { // Calculate green- color-value from velocity: 
  uint8_t G = 127;
  if (color_on_selector_value == 0) { G = 127 - velocity * 0.5; }
  if (color_on_selector_value == 1) { G = 127 * 0.75 - velocity; }
  if (color_on_selector_value == 2) { G = 127 - velocity * 0.5; }
  if (mode_drum == true)            { G = velocity; }
  return G;
};

uint8_t lpx_b(uint8_t velocity) { // Calculate blue-  color-value from velocity: 
  uint8_t B = 127;
  if (color_on_selector_value == 0) { B = 127 - velocity * 0.5; }
  if (color_on_selector_value == 1) { B = 127 * 0.75 - velocity; }
  if (color_on_selector_value == 2) { B = 127 * 0.75 - velocity; }
  if (mode_drum == true)            { B = velocity; }
  return B;
};

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||             VARIUOS SCREENS...            || — — — — — — — — — — — || */

void transport_screen() {

  // Light up the transport buttons:
  uint8_t lpx_sysex_cue_prev[] =  { 240, 0, 32, 41, 2, 12, 3, 3, 89, 127, 127, 127, 247 };
  hstmidi.sendSysEx(lpx_sysex_cue_prev);
  uint8_t lpx_sysex_cue_next[] =  { 240, 0, 32, 41, 2, 12, 3, 3, 79, 127, 127, 127, 247 };
  hstmidi.sendSysEx(lpx_sysex_cue_next);
  uint8_t lpx_sysex_bar_prev[] =  { 240, 0, 32, 41, 2, 12, 3, 3, 69, 127, 127,   0, 247 };
  hstmidi.sendSysEx(lpx_sysex_bar_prev);
  uint8_t lpx_sysex_bar_next[] =  { 240, 0, 32, 41, 2, 12, 3, 3, 59, 127, 127,   0, 247 };
  hstmidi.sendSysEx(lpx_sysex_bar_next);
  uint8_t lpx_sysex_beat_prev[] = { 240, 0, 32, 41, 2, 12, 3, 3, 49, 127,  32,  32, 247 };
  hstmidi.sendSysEx(lpx_sysex_beat_prev);
  uint8_t lpx_sysex_beat_next[] = { 240, 0, 32, 41, 2, 12, 3, 3, 39, 127,  32,  32, 247 };
  hstmidi.sendSysEx(lpx_sysex_beat_next);

}

void transport_screen_off() {

  // Light up the transport buttons:
  uint8_t lpx_sysex_cue_prev[] = { 240, 0, 32, 41, 2, 12, 3, 3, 89, 0, 0, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_cue_prev);
  uint8_t lpx_sysex_cue_next[] = { 240, 0, 32, 41, 2, 12, 3, 3, 79, 0, 0, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_cue_next);
  uint8_t lpx_sysex_bar_prev[] = { 240, 0, 32, 41, 2, 12, 3, 3, 69, 0, 0, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_bar_prev);
  uint8_t lpx_sysex_bar_next[] = { 240, 0, 32, 41, 2, 12, 3, 3, 59, 0, 0, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_bar_next);
  uint8_t lpx_sysex_beat_prev[] = { 240, 0, 32, 41, 2, 12, 3, 3, 49, 0, 0, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_beat_prev);
  uint8_t lpx_sysex_beat_next[] = { 240, 0, 32, 41, 2, 12, 3, 3, 39, 0, 0, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_beat_next);

  }

void transport_screen_animations(uint8_t controller) {

  if (controller == 89) { animation_1_in_progress = true; }
  if (controller == 79) { animation_2_in_progress = true; }
  if (controller == 69) { animation_3_in_progress = true; }
  if (controller == 59) { animation_4_in_progress = true; }
  if (controller == 49) { animation_5_in_progress = true; }
  if (controller == 39) { animation_6_in_progress = true; }

}


void color_test_screen_or_all_notes_off_screen() {

  // Turn all leds off to begin with:
  // for (uint8_t i = 0; i < 64; i++) {
  //   uint8_t lpx_pad_sysex[] = { 240, 0, 32, 41, 2, 12, 3, 0, every_pad[i], 0, 247 };
  //   hstmidi.sendSysEx(lpx_pad_sysex);
  // }

  // Turn all leds off to begin with:
  for (uint8_t i = 0; i < 43; i++) {
    send_led_sysex_to_launchpad(every_pad_without_pals[i], 0, 0, 0);
  }

  uint8_t lpx_pad_sysex[] = { 240, 0, 32, 41, 2, 12, 3, 3, 11, 100, 127, 127, 247 };
  hstmidi.sendSysEx(lpx_pad_sysex);

}


void key_transpose_screen() {

  // Light up the drum mode button:
  if (mode_drum == true){
    send_led_sysex_to_one_pad(29, 127, 64, 0);
  } else {
    send_led_sysex_to_one_pad(29, 8, 2, 0);
  }

  // Light up the color palette button:
  uint8_t lpx_sysex_color_palette_screen_button_on[] = { 240, 0, 32, 41, 2, 12, 3, 3, 89, lpx_r(127), lpx_g(127), lpx_b(127), 247 };
  hstmidi.sendSysEx(lpx_sysex_color_palette_screen_button_on);

  // Light up the midi channel selection button:
  uint8_t lpx_sysex_midi_channel_selection_button_on[] = { 240, 0, 32, 41, 2, 12, 3, 3, 79, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2], 247 };
  hstmidi.sendSysEx(lpx_sysex_midi_channel_selection_button_on);

  // Light up the layout shift buttons:
  uint8_t lpx_sysex_layout_shift_button_l[] = { 240, 0, 32, 41, 2, 12, 3, 3, 93, 4, 4, 4, 247 };
  hstmidi.sendSysEx(lpx_sysex_layout_shift_button_l);
  uint8_t lpx_sysex_layout_shift_button_r[] = { 240, 0, 32, 41, 2, 12, 3, 3, 94, 4, 4, 4, 247 };
  hstmidi.sendSysEx(lpx_sysex_layout_shift_button_r);

  // // Light up the color test button:
  uint8_t lpx_sysex_color_test_screen_button_on[] = { 240, 0, 32, 41, 2, 12, 3, 3, 19, 127, 0, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_color_test_screen_button_on);

  uint8_t key_transpose_screen_black_keys[5] = {82,83,85,86,87};
  for (int i = 0; i < 5; i++) {
    uint8_t lpx_sysex_key_transpose_screen_black_keys[] = { 240, 0, 32, 41, 2, 12, 3, 3, key_transpose_screen_black_keys[i], 6, 7, 8, 247 };
    hstmidi.sendSysEx(lpx_sysex_key_transpose_screen_black_keys);
  }

  uint8_t key_transpose_screen_white_keys[8] = { 71,72,73,74,75,76,77,78 };
  for (int i = 0; i < 8; i++) {
    uint8_t lpx_sysex_key_transpose_screen_white_keys[] = { 240, 0, 32, 41, 2, 12, 3, 3, key_transpose_screen_white_keys[i], 18, 20, 24, 247 };
    hstmidi.sendSysEx(lpx_sysex_key_transpose_screen_white_keys);
  }

  if (key_transpose == 0)  { key_transpose_selector_pad = 71; }
  if (key_transpose == 1)  { key_transpose_selector_pad = 82; }
  if (key_transpose == 2)  { key_transpose_selector_pad = 72; }
  if (key_transpose == 3)  { key_transpose_selector_pad = 83; }
  if (key_transpose == 4)  { key_transpose_selector_pad = 73; }
  if (key_transpose == 5)  { key_transpose_selector_pad = 74; }
  if (key_transpose == 6)  { key_transpose_selector_pad = 85; }
  if (key_transpose == 7)  { key_transpose_selector_pad = 75; }
  if (key_transpose == 8)  { key_transpose_selector_pad = 86; }
  if (key_transpose == 9)  { key_transpose_selector_pad = 76; }
  if (key_transpose == 10) { key_transpose_selector_pad = 87; }
  if (key_transpose == 11) { key_transpose_selector_pad = 77; }
  if (key_transpose == 12) { key_transpose_selector_pad = 78; }

  uint8_t lpx_sysex_key_transpose_screen_selected_key[] = { 240, 0, 32, 41, 2, 12, 3, 3, key_transpose_selector_pad, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2], 247 };
  hstmidi.sendSysEx(lpx_sysex_key_transpose_screen_selected_key);

  uint8_t key_transpose_screen_off_pads[14] = { 61, 64, 68, 61, 62, 63, 64, 65, 66, 67, 68, 81, 84, 88 };
  for (int i = 0; i < 14; i++) {
    uint8_t lpx_sysex_key_transpose_screen_off_pads[] = { 240, 0, 32, 41, 2, 12, 3, 0, key_transpose_screen_off_pads[i], 0, 247 };
    hstmidi.sendSysEx(lpx_sysex_key_transpose_screen_off_pads);
  }
}


void color_palette_screen() {

  uint8_t color_palette_screen_off_pads[24] = { 81,82,83,84,85,86,87,88,71,72,73,74,75,76,77,78,61,62,63,64,65,66,67,68 };
  for (int i = 0; i < 24; i++) {
    uint8_t lpx_sysex_color_palette_screen_off_pads[] = { 240, 0, 32, 41, 2, 12, 3, 0, color_palette_screen_off_pads[i], 0, 247 };
    hstmidi.sendSysEx(lpx_sysex_color_palette_screen_off_pads);
    }

  uint8_t lpx_sysex_color_palette_screen_button_1[] = { 240, 0, 32, 41, 2, 12, 3, 3, 72, 0, 16, 16, 247 };
  hstmidi.sendSysEx(lpx_sysex_color_palette_screen_button_1);

  uint8_t lpx_sysex_color_palette_screen_button_2[] = { 240, 0, 32, 41, 2, 12, 3, 3, 73, 16, 0, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_color_palette_screen_button_2);

  uint8_t lpx_sysex_color_palette_screen_button_3[] = { 240, 0, 32, 41, 2, 12, 3, 3, 74, 16, 16, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_color_palette_screen_button_3);

  if (color_on_selector_value == 0) { color_on_selector_pad = 72; }
  if (color_on_selector_value == 1) { color_on_selector_pad = 73; }
  if (color_on_selector_value == 2) { color_on_selector_pad = 74; }

  uint8_t lpx_sysex_color_palette_screen_selector_pad[] = { 240, 0, 32, 41, 2, 12, 3, 3, color_on_selector_pad, lpx_r(127), lpx_g(127), lpx_b(127), 247 };
  hstmidi.sendSysEx(lpx_sysex_color_palette_screen_selector_pad);

  uint8_t lpx_sysex_color_palette_screen_button_pad[] = { 240, 0, 32, 41, 2, 12, 3, 3, 89, lpx_r(127), lpx_g(127), lpx_b(127), 247 };
  hstmidi.sendSysEx(lpx_sysex_color_palette_screen_button_pad);

};

void midi_channel_selection_screen() {

  uint8_t clear_half_the_screen_pads[24] = { 81,82,83,84,85,86,87,88,71,72,73,74,75,76,77,78,61,62,63,64,65,66,67,68 };
  for (int i = 0; i < 24; i++) {
    uint8_t lpx_sysex_clear_half_the_screen[] = { 240, 0, 32, 41, 2, 12, 3, 0, clear_half_the_screen_pads[i], 0, 247 };
    hstmidi.sendSysEx(lpx_sysex_clear_half_the_screen);
  }

  uint8_t midi_channel_pads[16] = { 81,82,83,84,85,86,87,88,71,72,73,74,75,76,77,78 };
  for (int i = 0; i < 16; i++) {
    uint8_t lpx_sysex_midi_channel_pads[] = { 240, 0, 32, 41, 2, 12, 3, 3, midi_channel_pads[i], 8, 8, 8, 247 };
    hstmidi.sendSysEx(lpx_sysex_midi_channel_pads);
  }

  if (midi_channel_selector_int == 1)  { send_led_sysex_to_one_pad(81, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 2)  { send_led_sysex_to_one_pad(82, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 3)  { send_led_sysex_to_one_pad(83, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 4)  { send_led_sysex_to_one_pad(84, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 5)  { send_led_sysex_to_one_pad(85, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 6)  { send_led_sysex_to_one_pad(86, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 7)  { send_led_sysex_to_one_pad(87, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 8)  { send_led_sysex_to_one_pad(88, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 9)  { send_led_sysex_to_one_pad(71, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 10) { send_led_sysex_to_one_pad(72, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 11) { send_led_sysex_to_one_pad(73, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 12) { send_led_sysex_to_one_pad(74, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 13) { send_led_sysex_to_one_pad(75, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 14) { send_led_sysex_to_one_pad(76, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 15) { send_led_sysex_to_one_pad(77, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
  if (midi_channel_selector_int == 16) { send_led_sysex_to_one_pad(78, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
    
};

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||             PRINT POOL            || — — — — — — — — — — — || */

void print_pool() {

  if (mode_keyboard == true) {
    Serial << endl;
    Serial << "Mode: Keyboard";
    Serial << endl;
    }

  if (mode_drum == true){
    Serial << endl;
    Serial << "Mode: Drum";
    Serial << endl;
  }


  Serial << endl;
  Serial << "POOL:   " << "Index:   " << "Pad:   " << "Note:   " << "Velocity:   ";
  Serial << endl;

  for (uint8_t i = 0; i < 16; i++) {

    //INDEX
    Serial << "          " << i + 1;

    //PAD
    Serial << "          " << pad_pool[i];

    //NOTE
    Serial << "     " << note_pool[i];

    //VELOCITY
    if (note_pool[i] > 99) {                         Serial << "        " << velocity_pool[i]; }
    if (note_pool[i] < 99) { if (note_pool[i] > 0) { Serial << "         "    << velocity_pool[i]; } }
    if (note_pool[i] == 0) {                         Serial << "          "   << velocity_pool[i]; }
    
    Serial << endl;
    
    };
  
  Serial << endl;
  
  };

void print_left_or_right_arrow_released() {
  Serial << " * * *    " << "Transpose (";
  for (uint8_t i = 0; i < 7; i++) Serial << lowest_note_char[i];
  Serial << ") - 1 = " << endl;
  
  Serial << " * * * " << endl;
  Serial << " * * *    Arrow released..." << endl;
  Serial << " * * * " << endl;
  Serial << " * * *    " << "New layout is set to: " << pad_layout_shift << " with ";
  for (uint8_t i = 0; i < 7; i++) Serial << lowest_note_char[i];
  Serial << " as the lowest note." << endl;
  Serial << " * * * " << endl;
  Serial << " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " << endl;
  print_pool();
};

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||             PAD-PALS            || — — — — — — — — — — — || */

uint8_t pad_pals(uint8_t pad) {
  if (pad == 16) { return 21; }
  if (pad == 17) { return 22; }
  if (pad == 18) { return 23; }
  if (pad == 21) { return 16; }
  if (pad == 22) { return 17; }
  if (pad == 23) { return 18; }

  if (pad == 26) { return 31; }
  if (pad == 27) { return 32; }
  if (pad == 28) { return 33; }
  if (pad == 31) { return 26; }
  if (pad == 32) { return 27; }
  if (pad == 33) { return 28; }

  if (pad == 36) { return 41; }
  if (pad == 37) { return 42; }
  if (pad == 38) { return 43; }
  if (pad == 41) { return 36; }
  if (pad == 42) { return 37; }
  if (pad == 43) { return 38; }

  if (pad == 36) { return 41; }
  if (pad == 37) { return 42; }
  if (pad == 38) { return 43; }
  if (pad == 41) { return 36; }
  if (pad == 42) { return 37; }
  if (pad == 43) { return 38; }

  if (pad == 46) { return 51; }
  if (pad == 47) { return 52; }
  if (pad == 48) { return 53; }
  if (pad == 51) { return 46; }
  if (pad == 52) { return 47; }
  if (pad == 53) { return 48; }

  if (pad == 56) { return 61; }
  if (pad == 57) { return 62; }
  if (pad == 58) { return 63; }
  if (pad == 61) { return 56; }
  if (pad == 62) { return 57; }
  if (pad == 63) { return 58; }

  if (pad == 66) { return 71; }
  if (pad == 67) { return 72; }
  if (pad == 68) { return 73; }
  if (pad == 71) { return 66; }
  if (pad == 72) { return 67; }
  if (pad == 73) { return 68; }

  if (pad == 76) { return 81; }
  if (pad == 77) { return 82; }
  if (pad == 78) { return 83; }
  if (pad == 81) { return 76; }
  if (pad == 82) { return 77; }
  if (pad == 83) { return 78; }

  return 0;

};



/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||         PAD-TO-MIDI TABLE       || — — — — — — — — — — — || */

uint8_t pad_to_midi_processing_table(uint8_t pad) {

  if (pad == 11) { return 48; }
  if (pad == 12) { return 49; }
  if (pad == 13) { return 50; }
  if (pad == 14) { return 51; }
  if (pad == 15) { return 52; }
  if (pad == 16) { return 53; }
  if (pad == 17) { return 54; }
  if (pad == 18) { return 55; }

  if (pad == 21) { return 53; }
  if (pad == 22) { return 54; }
  if (pad == 23) { return 55; }
  if (pad == 24) { return 56; }
  if (pad == 25) { return 57; }
  if (pad == 26) { return 58; }
  if (pad == 27) { return 59; }
  if (pad == 28) { return 60; }
  
  if (pad == 31) { return 58; }
  if (pad == 32) { return 59; }
  if (pad == 33) { return 60; }
  if (pad == 34) { return 61; }
  if (pad == 35) { return 62; }
  if (pad == 36) { return 63; }
  if (pad == 37) { return 64; }
  if (pad == 38) { return 65; }
  
  if (pad == 41) { return 63; }
  if (pad == 42) { return 64; }
  if (pad == 43) { return 65; }
  if (pad == 44) { return 66; }
  if (pad == 45) { return 67; }
  if (pad == 46) { return 68; }
  if (pad == 47) { return 69; }
  if (pad == 48) { return 70; }
  
  if (pad == 51) { return 68; }
  if (pad == 52) { return 69; }
  if (pad == 53) { return 70; }
  if (pad == 54) { return 71; }
  if (pad == 55) { return 72; }
  if (pad == 56) { return 73; }
  if (pad == 57) { return 74; }
  if (pad == 58) { return 75; }
  
  if (pad == 61) { return 73; }
  if (pad == 62) { return 74; }
  if (pad == 63) { return 75; }
  if (pad == 64) { return 76; }
  if (pad == 65) { return 77; }
  if (pad == 66) { return 78; }
  if (pad == 67) { return 79; }
  if (pad == 68) { return 80; }
  
  if (pad == 71) { return 78; }
  if (pad == 72) { return 79; }
  if (pad == 73) { return 80; }
  if (pad == 74) { return 81; }
  if (pad == 75) { return 82; }
  if (pad == 76) { return 83; }
  if (pad == 77) { return 84; }
  if (pad == 78) { return 85; }
  
  if (pad == 81) { return 83; }
  if (pad == 82) { return 84; }
  if (pad == 83) { return 85; }
  if (pad == 84) { return 86; }
  if (pad == 85) { return 87; }
  if (pad == 86) { return 88; }
  if (pad == 87) { return 89; }
  if (pad == 88) { return 90; }
  
  return 0;

};

uint8_t pad_to_midi_processing_table_drum_edition(uint8_t pad) {


  // MC-101
  // uint8_t note_kick = 36;
  // uint8_t note_snre = 37;
  // uint8_t note_clap = 39;
  // uint8_t note_clhh = 40;
  // uint8_t note_ophh = 41;
  // uint8_t note_tom1 = 42;
  // uint8_t note_tom2 = 43;
  // uint8_t note_tom3 = 44;
  // uint8_t note_ride = 45;
  // uint8_t note_crsh = 46;

  // TR-8
  uint8_t note_kick = 36;
  uint8_t note_snre = 38;
  uint8_t note_clap = 39;
  uint8_t note_clhh = 42;
  uint8_t note_ophh = 46;
  uint8_t note_tom1 = 50;
  uint8_t note_tom1_101 = 48;
  uint8_t note_tom2 = 47;
  uint8_t note_tom2_101 = 45;
  uint8_t note_tom3 = 43;
  uint8_t note_tom3_101 = 41;
  uint8_t note_ride = 51;
  uint8_t note_crsh = 49;

  if (pad == 11) { return note_crsh; }
  if (pad == 12) { return note_crsh; }
  if (pad == 13) { return note_kick; }
  if (pad == 14) { return note_kick; }
  if (pad == 15) { return note_kick; }
  if (pad == 16) { return note_kick; }
  if (pad == 17) { return note_crsh; }
  if (pad == 18) { return note_crsh; }

  if (pad == 21) { return note_crsh; }
  if (pad == 22) { return note_crsh; }
  if (pad == 23) { return note_kick; }
  if (pad == 24) { return note_kick; }
  if (pad == 25) { return note_kick; }
  if (pad == 26) { return note_kick; }
  if (pad == 27) { return note_crsh; }
  if (pad == 28) { return note_crsh; }
  
  if (pad == 31) { return note_ophh; }
  if (pad == 32) { return note_ophh; }
  if (pad == 33) { return note_clhh; }
  if (pad == 34) { return note_clhh; }
  if (pad == 35) { return note_clhh; }
  if (pad == 36) { return note_clhh; }
  if (pad == 37) { return note_ophh; }
  if (pad == 38) { return note_ophh; }
  
  if (pad == 41) { return note_ophh; }
  if (pad == 42) { return note_ophh; }
  if (pad == 43) { return note_clhh; }
  if (pad == 44) { return note_clhh; }
  if (pad == 45) { return note_clhh; }
  if (pad == 46) { return note_clhh; }
  if (pad == 47) { return note_ophh; }
  if (pad == 48) { return note_ophh; }
  
  if (pad == 51) { return note_clap; }
  if (pad == 52) { return note_clap; }
  if (pad == 53) { return note_snre; }
  if (pad == 54) { return note_snre; }
  if (pad == 55) { return note_snre; }
  if (pad == 56) { return note_snre; }
  if (pad == 57) { return note_clap; }
  if (pad == 58) { return note_clap; }
  
  if (pad == 61) { return note_clap; }
  if (pad == 62) { return note_clap; }
  if (pad == 63) { return note_snre; }
  if (pad == 64) { return note_snre; }
  if (pad == 65) { return note_snre; }
  if (pad == 66) { return note_snre; }
  if (pad == 67) { return note_clap; }
  if (pad == 68) { return note_clap; }
  
  if (pad == 71) { return note_tom3; }
  if (pad == 72) { return note_tom3; }
  if (pad == 73) { return note_tom2; }
  if (pad == 74) { return note_tom2; }
  if (pad == 75) { return note_tom1; }
  if (pad == 76) { return note_tom1; }
  if (pad == 77) { return note_ride; }
  if (pad == 78) { return note_ride; }
  
  if (pad == 81) { return note_tom3; }
  if (pad == 82) { return note_tom3; }
  if (pad == 83) { return note_tom2; }
  if (pad == 84) { return note_tom2; }
  if (pad == 85) { return note_tom1; }
  if (pad == 86) { return note_tom1; }
  if (pad == 87) { return note_ride; }
  if (pad == 88) { return note_ride; }
  
  return 0;

};

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||        PAD LAYOUTS ARRAYS       || — — — — — — — — — — — || */

void white_key_layouts() {

  if (pad_layout_shift == 0 || pad_layout_shift == 12 || pad_layout_shift == 24 || pad_layout_shift == -12 || pad_layout_shift == -24 ) {
    char new_lowest_note_char[7] = { 'C', ' ', ' ', ' ', ' ', ' ', ' ' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 11, 13, 15, 16, 18, 21, 23, 25, 27, 28, 32, 33, 35, 37, 38, 42, 43, 45, 47, 52, 54, 55, 57, 62, 64, 65, 67, 72, 74, 76, 77, 81, 82, 84, 86, 87,255,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -1 || pad_layout_shift == 11 || pad_layout_shift == 23 || pad_layout_shift == -13 || pad_layout_shift == -25 ) {
    char new_lowest_note_char[7] = { 'B', ' ', ' ', ' ', ' ', ' ', ' ' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 11, 12, 14, 16, 17, 21, 22, 24, 26, 28, 31, 33, 34, 36, 38, 41, 43, 44, 46, 48, 51, 53, 55, 56, 58, 61, 63, 65, 66, 68, 71, 73, 75, 77, 78, 82, 83, 85, 87, 88 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -2 || pad_layout_shift == 10 || pad_layout_shift == 22 || pad_layout_shift == -14 || pad_layout_shift == -26 ) {
    char new_lowest_note_char[7] = { 'B', 'b', ' ', '/', ' ', 'A', '#' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 12, 13, 15, 17, 18, 22, 23, 25, 27, 32, 34, 35, 37, 42, 44, 45, 47, 52, 54, 56, 57, 61, 62, 64, 66, 67, 71, 72, 74, 76, 78, 81, 83, 84, 86, 88,255,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -3 || pad_layout_shift == 9 || pad_layout_shift == 21 || pad_layout_shift == -15 || pad_layout_shift == -27 ) {
    char new_lowest_note_char[7] = { 'A', ' ', ' ', ' ', ' ', ' ', ' ' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 11, 13, 14, 16, 18, 21, 23, 24, 26, 28, 31, 33, 35, 36, 38, 41, 43, 45, 46, 48, 51, 53, 55, 57, 58, 62, 63, 65, 67, 68, 72, 73, 75, 77, 82, 84, 85, 87,255,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -4 || pad_layout_shift == 8 || pad_layout_shift == 20 || pad_layout_shift == -16 || pad_layout_shift == -28 ) {
    char new_lowest_note_char[7] = { 'A', 'b', ' ', '/', ' ', 'G', '#' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 12, 14, 15, 17, 22, 24, 25, 27, 32, 34, 36, 37, 41, 42, 44, 46, 47, 51, 52, 54, 56, 58, 61, 63, 64, 66, 68, 71, 73, 74, 76, 78, 81, 83, 85, 86, 88,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -5 || pad_layout_shift == 7 || pad_layout_shift == 19 || pad_layout_shift == -17 || pad_layout_shift == -29 ) {
    char new_lowest_note_char[7] = { 'G', ' ', ' ', ' ', ' ', ' ', ' ' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 11, 13, 15, 16, 18, 21, 23, 25, 26, 28, 31, 33, 35, 37, 38, 42, 43, 45, 47, 48, 52, 53, 55, 57, 62, 64, 65, 67, 72, 74, 75, 77, 82, 84, 86, 87,255,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -6 || pad_layout_shift == 6 || pad_layout_shift == 18 || pad_layout_shift == -18 || pad_layout_shift == -30 ) {
    char new_lowest_note_char[7] = { 'G', 'b', ' ', '/', ' ', 'F', '#' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 12, 14, 16, 17, 21, 22, 24, 26, 27, 31, 32, 34, 36, 38, 41, 43, 44, 46, 48, 51, 53, 54, 56, 58, 61, 63, 65, 66, 68, 71, 73, 75, 76, 78, 81, 83, 85, 87, 88,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -7 || pad_layout_shift == 5 || pad_layout_shift == 17 || pad_layout_shift == -19 || pad_layout_shift == -31 ) {
    char new_lowest_note_char[7] = { 'F', ' ', ' ', ' ', ' ', ' ', ' ' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 11, 13, 15, 17, 18, 22, 23, 25, 27, 28, 32, 33, 35, 37, 42, 44, 45, 47, 52, 54, 55, 57, 62, 64, 66, 67, 71, 72, 74, 76, 77, 81, 82, 84, 86, 88,255,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -8 || pad_layout_shift == 4 || pad_layout_shift == 16 || pad_layout_shift == -20 || pad_layout_shift == -32 ) {
    char new_lowest_note_char[7] = { 'E', ' ', ' ', ' ', ' ', ' ', ' ' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 11, 12, 14, 16, 18, 21, 23, 24, 26, 28, 31, 33, 34, 36, 38, 41, 43, 45, 46, 48, 51, 53, 55, 56, 58, 61, 63, 65, 67, 68, 72, 73, 75, 77, 78, 82, 83, 85, 87,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -9 || pad_layout_shift == 3 || pad_layout_shift == 15 || pad_layout_shift == -21 || pad_layout_shift == -33 ) {
    char new_lowest_note_char[7] = { 'E', 'b', ' ', '/', ' ', 'D', '#' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 12, 13, 15, 17, 22, 24, 25, 27, 32, 34, 35, 37, 42, 44, 46, 47, 51, 52, 54, 56, 57, 61, 62, 64, 66, 68, 71, 73, 74, 76, 78, 81, 83, 84, 86, 88,255,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -10 || pad_layout_shift == 2 || pad_layout_shift == 14 || pad_layout_shift == -22 || pad_layout_shift == -34 ) {
    char new_lowest_note_char[7] = { 'D', ' ', ' ', ' ', ' ', ' ', ' ' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 11, 13, 14, 16, 18, 21, 23, 25, 26, 28, 31, 33, 35, 36, 38, 41, 43, 45, 47, 48, 52, 53, 55, 57, 58, 62, 63, 65, 67, 72, 74, 75, 77, 82, 84, 85, 87,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };

  if (pad_layout_shift == -11 || pad_layout_shift == 1 || pad_layout_shift == 13 || pad_layout_shift == -23 || pad_layout_shift == -35 ) {
    char new_lowest_note_char[7] = { 'D', 'b', ' ', '/', ' ', 'C', '#' };
    for (int i = 0; i < 7; i++) { lowest_note_char[i] = new_lowest_note_char[i]; }
    uint8_t new_white_keys[] = { 12, 14, 15, 17, 22, 24, 26, 27, 31, 32, 34, 36, 37, 41, 42, 44, 46, 48, 51, 53, 54, 56, 58, 61, 63, 64, 66, 68, 71, 73, 75, 76, 78, 81, 83, 85, 86, 88,255,255 };
    for (uint8_t i = 0; i < 40; i++) { white_keys[i] = new_white_keys[i]; }
  };
};

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||        MIDI NOTE HANDLING       || — — — — — — — — — — — || */

void midi_note_processing(uint8_t pad, uint8_t velocity) {

  Serial << " • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • " << endl << endl;

  /* #region    || — — — — — — — — — — ||        "MODIFIER"-SCREENS "SELECTORS"      || — — — — — — — — — — — || */

  if (key_transpose_button_pressed == true && color_palette_button_pressed == true && midi_channel_selector_button_pressed == false) {
    if (pad == 72 && velocity != 0) { color_on_selector_value = 0; }
    if (pad == 73 && velocity != 0) { color_on_selector_value = 1; }
    if (pad == 74 && velocity != 0) { color_on_selector_value = 2; }
    color_palette_screen();

    // Light up the color palette button:
    uint8_t lpx_sysex_color_palette_screen_button_on[] = { 240, 0, 32, 41, 2, 12, 3, 3, 89, lpx_r(127), lpx_g(127), lpx_b(127), 247 };
    hstmidi.sendSysEx(lpx_sysex_color_palette_screen_button_on);
  }

  if (key_transpose_button_pressed == true && midi_channel_selector_button_pressed == true && color_palette_button_pressed == false) {
    if (pad == 81 && velocity != 0) { midi_channel_selector_int = 1;  }
    if (pad == 82 && velocity != 0) { midi_channel_selector_int = 2;  }
    if (pad == 83 && velocity != 0) { midi_channel_selector_int = 3;  }
    if (pad == 84 && velocity != 0) { midi_channel_selector_int = 4;  }
    if (pad == 85 && velocity != 0) { midi_channel_selector_int = 5;  }
    if (pad == 86 && velocity != 0) { midi_channel_selector_int = 6;  }
    if (pad == 87 && velocity != 0) { midi_channel_selector_int = 7;  }
    if (pad == 88 && velocity != 0) { midi_channel_selector_int = 8;  }
    if (pad == 71 && velocity != 0) { midi_channel_selector_int = 9;  }
    if (pad == 72 && velocity != 0) { midi_channel_selector_int = 10; }
    if (pad == 73 && velocity != 0) { midi_channel_selector_int = 11; }
    if (pad == 74 && velocity != 0) { midi_channel_selector_int = 12; }
    if (pad == 75 && velocity != 0) { midi_channel_selector_int = 13; }
    if (pad == 76 && velocity != 0) { midi_channel_selector_int = 14; }
    if (pad == 77 && velocity != 0) { midi_channel_selector_int = 15; }
    if (pad == 78 && velocity != 0) { midi_channel_selector_int = 16; }
    midi_channel_selection_screen();
  }

  if (key_transpose_button_pressed == true && color_palette_button_pressed == false && midi_channel_selector_button_pressed == false) {
    if (pad == 71 && velocity != 0) { key_transpose = 0;  }
    if (pad == 82 && velocity != 0) { key_transpose = 1;  }
    if (pad == 72 && velocity != 0) { key_transpose = 2;  }
    if (pad == 83 && velocity != 0) { key_transpose = 3;  }
    if (pad == 73 && velocity != 0) { key_transpose = 4;  }
    if (pad == 74 && velocity != 0) { key_transpose = 5;  }
    if (pad == 85 && velocity != 0) { key_transpose = 6;  }
    if (pad == 75 && velocity != 0) { key_transpose = 7;  }
    if (pad == 86 && velocity != 0) { key_transpose = 8;  }
    if (pad == 76 && velocity != 0) { key_transpose = 9;  }
    if (pad == 87 && velocity != 0) { key_transpose = 10; }
    if (pad == 77 && velocity != 0) { key_transpose = 11; }
    if (pad == 78 && velocity != 0) { key_transpose = 12; }
    key_transpose_screen();
  }

  /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

  // Check what note to process when pad is pressed:
  // HÄR? Should we split drum mode and keyboard mode here?!

  uint8_t note = 0;

  if (mode_keyboard == true){
    note = pad_to_midi_processing_table(pad) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose;

  


    
    // Check if the note is still "valid" after going through transpose and layoutshift modifications...
    bool note_in_midi_range = true;
    if (pad_to_midi_processing_table(pad) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose < 0 || pad_to_midi_processing_table(pad) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose > 127) {
      note = 0;
      note_in_midi_range = false;
    }
  
    Serial << "NOTE EVENT: " << note << endl;
    Serial << "LAYOUT POSITION: ";
    for (uint8_t i = 0; i < 7; i++) Serial << lowest_note_char[i];
    Serial << "   PAD: " << pad << "   VELOCITY: " << velocity << "   OCTAVE: " << octave_shift[octave_shift_amount_selector] << endl;
  
    // Keep track of pad pals old positions...
    uint8_t pool_position_where_the_old_note_is = 0;
  
    // If the new note is a note in the range 0-127 and it has velocity:
    if (note_in_midi_range == true && velocity != 0) {
  
      // Only add notes to pool if they are visable when pressed:
      if (key_transpose_button_pressed == false || pad < 59) {
  
        // Check first if there is room for a new note (if one of the 16 slots are empty):
        for (uint8_t i = 0; i < 16; i++) {
          if (pad_pool[i] == 0) {
  
            // Check if the new note already is in the pool:
            bool pad_pal_in_pool = false;
            for (uint8_t j = 0; j < 16; j++) {
  
              if (note == note_pool[j]) {
                pool_position_where_the_old_note_is = j;
                pad_pal_in_pool = true;
  
                break; // There should only be one to find, so might aswell break
                }
              }
  
            // If there are pad_pals in the pool...
            if (pad_pal_in_pool == true) {
  
              Serial << "   PAD PAL IN POOL: ";
              Serial << "True || i = " << pool_position_where_the_old_note_is + 1 << ". ";
  
              // ...only replace the midi note if the new velocity is greater then the old...
              if (velocity > velocity_pool[pool_position_where_the_old_note_is]) {
  
                Serial << "This notes velocity (" << velocity << ") is higher then the previous note in the pool, (" << velocity_pool[pool_position_where_the_old_note_is] << "). Replace the midi notes.)" << endl;
                replace_old_midi_note_in_pool_with_new(pad, note, velocity);
  
              } else {
  
                // Serial << "   (Velocity for note " << note << " is too low, (" << velocity << ") don't send any new midi note)" << endl;
                Serial << "   (Velocity for new note ()" << velocity << ") is lower then the previous, (" << velocity_pool[pool_position_where_the_old_note_is] << ") sp don't... on secon thought, it feels wierd not sending a new note, so send the new midi note regardless of the velocity...)" << endl;
                replace_old_midi_note_in_pool_with_new(pad, note, velocity);
              }
              
            } else {
  
              Serial << "   PAD PAL IN POOL: " << "False";
              Serial << " || Send midi on." << endl;
  
              // If the new note isn't already in the pool, send midi_on
              send_midi_note_on(note, velocity);
              // usbmidi.sendNoteOn(attach_midi_channel_to_note(note), velocity);
              // dinmidi.sendNoteOn(attach_midi_channel_to_note(note), velocity);
              // dinmidi.sendNoteOn(note, velocity);
              send_led_sysex_to_launchpad(pad, lpx_r(velocity), lpx_g(velocity), lpx_b(velocity));
  
            }
  
            // Add the note to the array (Even if its already there... We need both!)
            pad_pool[i] = pad;
            note_pool[i] = note;
            velocity_pool[i] = velocity;
            // channel_pool[i] = lpx_midi_channel;
  
            break; // We don't need the note to fill more then one pool slot...
          }
        }
      }
    }
    print_pool();
    Serial << " • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • " << endl;
  
  
    /* #region    || — — — — — — — — — — ||            VELOCITY 0           || — — — — — — — — — — — || */

  // If the velocity recieved is 0
    if (velocity == 0) {

      // If the note is in the pool...
      for (uint8_t i = 0; i < 16; i++) {
        if (pad == pad_pool[i]) {

          uint8_t midi_off_note = note_pool[i];

          // If the pad has been shifted, correct it accordingly:
          if (pad_transposed[i] != 0) {
            pad = pad_transposed[i];
            }

          // Remove the note out of the pool. Empty the pool-slots:
          pad_pool[i] = 0;
          note_pool[i] = 0;
          velocity_pool[i] = 0;
          pad_transposed[i] = 0;
          // channel_pool[i]   = 0;

          // If there´s still a matching note in the pool (even tough we already removed one above...), it's the pad pal! There are pad_pals in the pool!
          bool pad_pal_in_the_pool = false;
          for (uint8_t j = 0; j < 16; j++) {
            if (note == note_pool[j]) {
              pool_position_where_the_old_note_is = j;
              pad_pal_in_the_pool = true;

              Serial << "   PAD PAL IN POOL: " << "True";
              Serial << " || Note still in pool (" << midi_off_note << ") don't send any midi note off message." << endl;

              break; // Break after we've found the first match...
              }
            }

          // If there are no pad_pals in the pool, send midi_off
          if (pad_pal_in_the_pool == false) {

            usbmidi.sendNoteOff(attach_midi_channel_to_note(midi_off_note), velocity);

            // MIDIAddress note_to_din4{ midi_off_note, lpx_midi_channel };
            dinmidi.sendNoteOff(attach_midi_channel_to_note(midi_off_note), velocity);
            // dinmidi.sendNoteOff(midi_off_note, velocity);

            Serial << "   PAD PAL IN POOL: " << "False";
            Serial << " || Send midi note off (" << midi_off_note << ")" << endl;


            // Don't "color over" any screens...
            if (key_transpose_button_pressed == false || pad < 59) {

              // Send Sysex to Launchpad to light the pad
              // Check if the pad is white:
              bool pad_is_white = false;
              for (uint8_t i = 0; i < 40; i++) {

                if (pad == white_keys[i]) {
                  send_led_sysex_to_launchpad(pad, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]);
                  pad_is_white = true;
                  break; // There shouldn't be more then one match...
                  } else send_led_sysex_to_launchpad(pad, 0, 0, 0); // all 40 white pads are checked, the pad is black.

                }
              }
            }
          break; // If we've already found one matching note, their no need to find another
          }
        }

      print_pool();
      Serial << " • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • " << endl;

      }
  
  
  }

  if (mode_drum == true){

    note = pad_to_midi_processing_table_drum_edition(pad);

    Serial << "NOTE EVENT: " << note << endl;
    Serial << "LAYOUT POSITION: ";
    for (uint8_t i = 0; i < 7; i++) Serial << lowest_note_char[i];
    Serial << "   PAD: " << pad << "   VELOCITY: " << velocity << "   OCTAVE: " << octave_shift[octave_shift_amount_selector] << endl;

    // Keep track of pad pals old positions...
    uint8_t pool_position_where_the_old_note_is = 0;

    if (velocity != 0) {



      // Only add notes to pool if they are visable when pressed:
      if (key_transpose_button_pressed == false || pad < 59) {

        send_drum_midi_note_on(pad, velocity);

//         // Check first if there is room for a new note (if one of the 16 slots are empty):
//         for (uint8_t i = 0; i < 16; i++) {
//           if (drum_pool[i] == 0) {
// 
//             // Check if the new note already is in the pool:
//             bool drum_pal_in_pool = false;
//             for (uint8_t j = 0; j < 16; j++) {
// 
//               if (note == note_pool[j]) {
//                 pool_position_where_the_old_note_is = j;
//                 drum_pal_in_pool = true;
// 
//                 break; // There should only be one to find, so might aswell break
//                 }
//               }
// 
//             // If there are pad_pals in the pool...
//             if (drum_pal_in_pool == true) {
// 
//               Serial << "   PAD PAL IN POOL: ";
//               Serial << "True || i = " << pool_position_where_the_old_note_is + 1 << ". ";
// 
//               // ...only replace the midi note if the new velocity is greater then the old...
//               if (velocity > velocity_pool[pool_position_where_the_old_note_is]) {
// 
//                 Serial << "This notes velocity (" << velocity << ") is higher then the previous note in the pool, (" << velocity_pool[pool_position_where_the_old_note_is] << "). Replace the midi notes.)" << endl;
//                 replace_old_drum_note_in_pool_with_new(pad, note, velocity);
// 
//                 } else {
// 
//                 // Serial << "   (Velocity for note " << note << " is too low, (" << velocity << ") don't send any new midi note)" << endl;
//                 Serial << "   (Velocity for new note ()" << velocity << ") is lower then the previous, (" << velocity_pool[pool_position_where_the_old_note_is] << ") sp don't... on secon thought, it feels wierd not sending a new note, so send the new midi note regardless of the velocity...)" << endl;
//                 replace_old_drum_note_in_pool_with_new(pad, note, velocity);
//                 }
// 
//               } else {
// 
//               Serial << "   PAD PAL IN POOL: " << "False";
//               Serial << " || Send midi on." << endl;
// 
//               send_drum_midi_note_on(pad, velocity);
// 
//               }
// 
//             // Add the note to the array (Even if its already there... We need both!)
//             pad_pool[i] = pad;
//             note_pool[i] = note;
//             velocity_pool[i] = velocity;
//             // channel_pool[i] = lpx_midi_channel;
// 
//             break; // We don't need the note to fill more then one pool slot...
//             }
//           }
        }

      
    }

    if (velocity == 0) {

      send_drum_midi_note_off(pad, velocity);

//       // If the note is in the pool...
//       for (uint8_t i = 0; i < 16; i++) {
//         if (pad == pad_pool[i]) {
// 
//           uint8_t midi_off_note = note_pool[i];
// 
//           // Remove the note out of the pool. Empty the pool-slots:
//           pad_pool[i] = 0;
//           note_pool[i] = 0;
//           velocity_pool[i] = 0;
//           pad_transposed[i] = 0;
//           // channel_pool[i]   = 0;
// 
//           // If there´s still a matching note in the pool (even tough we already removed one above...), it's the pad pal! There are pad_pals in the pool!
//           bool pad_pal_in_the_pool = false;
//           for (uint8_t j = 0; j < 16; j++) {
//             if (note == note_pool[j]) {
//               pool_position_where_the_old_note_is = j;
//               pad_pal_in_the_pool = true;
// 
//               Serial << "   PAD PAL IN POOL: " << "True";
//               Serial << " || Note still in pool (" << midi_off_note << ") don't send any midi note off message." << endl;
// 
//               break; // Break after we've found the first match...
//               }
//             }
// 
//           // If there are no pad_pals in the pool, send midi_off
//           if (pad_pal_in_the_pool == false) {
// 
//             send_drum_midi_note_off(pad, velocity);
// 
//             Serial << "   PAD PAL IN POOL: " << "False";
//             Serial << " || Send midi note off (" << midi_off_note << ")" << endl;
// 
//             }
//           break; // If we've already found one matching note, their no need to find another
//           }
//         }


    }

    print_pool();
  }
  



  /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */
};

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||         MIDI CC HANDLING        || — — — — — — — — — — — || */

void control_change_processing(uint8_t controller, uint8_t value) {

  /* #region    || — — — — — — — — — — ||         TRANSPORT SCREEN        || — — — — — — — — — — — || */
  // — — — — — — — — — — // TRANSPORT SCREEN:
  if (controller == 97) {
    if (value == 127) {
      custom_button_pressed = true;
      transport_screen();
      } else {
      custom_button_pressed = false;
      transport_screen_off();
      // refresh_pads = true;
    }
  }

  // — — — — — — — — — — // TRANSPORT SCREEN // TRANSPORT BUTTONS:
  if (custom_button_pressed == true) {
    for (uint8_t i = 0; i < 8; i++) {
      if (controller == transport_buttons[i]){
        if (value != 0){
          if ( animation_in_progress == false ) {

            animation_in_progress = true;
            transport_screen_animations(controller);

          } else {

            // an animation is already running... Stop it!
            animation_in_progress = false;

            // "clear" the pads before starting a new animation:
            refresh_pads = true;
            refresh_all_pads_function();

            // start a new animation:
            animation_in_progress = true;
            transport_screen_animations(controller);
          
          }
        }
      }
    }
  }

  /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

  /* #region    || — — — — — — — — — — ||       KEY TRANSPOSE SCREEN      || — — — — — — — — — — — || */
  // — — — — — — — — — — // KEY TRANSPOSE SCREEN:
  if (controller == 96) {
    if (value == 127) {
      key_transpose_screen();
      key_transpose_button_pressed = true;
      } else {
      key_transpose_button_pressed = false;
      refresh_pads = true;

      // Turn off the left and right arrow buttons:
      uint8_t lpx_sysex_layout_shift_button_l[] = { 240, 0, 32, 41, 2, 12, 3, 3, 93, 0, 0, 0, 247 };
      hstmidi.sendSysEx(lpx_sysex_layout_shift_button_l);
      uint8_t lpx_sysex_layout_shift_button_r[] = { 240, 0, 32, 41, 2, 12, 3, 3, 94, 0, 0, 0, 247 };
      hstmidi.sendSysEx(lpx_sysex_layout_shift_button_r);

      // uint8_t bass_mode_off_off[] = { 240, 0, 32, 41, 2, 12, 3, 3, 39, 0, 0, 0, 247 };
      // hstmidi.sendSysEx(bass_mode_off_off);
    }
  }
  

 // — — — — — — — — — — // COLOR PALETTE SCREEN
  if (key_transpose_button_pressed == true) {
    if (controller == 89){
      if (value == 127){
        color_palette_button_pressed = true;
        color_palette_screen();
      } else {
        color_palette_button_pressed = false;
        if (key_transpose_button_pressed == true) { key_transpose_screen(); }
      }
    }
  }

  // — — — — — — — — — — // MIDI CHANNEL SELECTION SCREEN
  if (key_transpose_button_pressed == true) {
    if (controller == 79) {
      if (value == 127) {
        midi_channel_selector_button_pressed = true;
        midi_channel_selection_screen();
        } else {
        midi_channel_selector_button_pressed = false;
        if (key_transpose_button_pressed == true) { key_transpose_screen(); }
        }
      }
    }

  // — — — — — — — — — — // DRUM MODE ON/OFF(?)
  if (key_transpose_button_pressed == true) {
    if (controller == 29) {
      if (value == 127) {
        mode_toggle();
        refresh_pads = true;
      }
    }
  }
  

  // — — — — — — — — — — // COLOR TEST SCREEN / ALL NOTES OFF BUTTON:
   if (key_transpose_button_pressed == true) {
    if (controller == 19){
      if (value == 127){
        color_test_screen_or_all_notes_off_screen();
        color_test_button_pressed = true;
        
        // All notes off:
        usbmidi.sendControlChange(123, 0);
        // usbmidi.sendControlChange({ 123, CHANNEL_4 }, 127);
        // dinmidi.sendControlChange(123, 0);

        // Does the channel matter?
        dinmidi.sendControlChange({123, get_current_midi_channel()}, 0 );
        
        // // All sounds off:
        // dinmidi.sendControlChange(120, 0);
        

        for (uint8_t i = 0; i < 16; i++) {
          dinmidi.sendControlChange({123, all_midi_channels[i]}, 127);
        }

        clear_pool();

      } else {
        refresh_pads = true;
        if (key_transpose_button_pressed == true) { key_transpose_screen(); }
      }
    }
  }

   /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

  /* #region    || — — — — — — — — — — ||           OCTAVE SHIFT          || — — — — — — — — — — — || */
  // — — — — — — — — — — // OCTAVE SHIFT BUTTONS:

  if (controller == 91) {
    if (value == 127) {
      up_arrow_pressed = true;
      if (down_arrow_pressed != true) {
        octave_shift_amount_selector += 1;
        if (octave_shift_amount_selector > 6) octave_shift_amount_selector = 6;
      } else octave_shift_amount_selector = 3;
      refresh_pads = true;
    } else up_arrow_pressed = false;
    
  }

  if (controller == 92) {
    if (value == 127) {
      down_arrow_pressed = true;
      if (up_arrow_pressed != true){
        octave_shift_amount_selector -= 1;
        if (octave_shift_amount_selector < 0) octave_shift_amount_selector = 0;
      } else octave_shift_amount_selector = 3;
      refresh_pads = true;
    } else down_arrow_pressed = false;
  }
  /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

  /* #region    || — — — — — — — — — — ||              LAYOUT             || — — — — — — — — — — — || */


  int old_layout_shift = pad_layout_shift;

  if (key_transpose_button_pressed == true) {

    // — — — — — — — — — — // LAYOUT SHIFT BUTTONS:
    if (controller == 93) {
      if (value == 127) {
        left_arrow_pressed = true;
        lpx_sysex_cc_button_white(93);
        if (right_arrow_pressed == true) {
          pad_layout_shift = 4;
        } else pad_layout_shift += 1;
        refresh_pads = true;
        Serial << " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " << endl << " * * * " << endl;
      }
      if (value == 0) {
        left_arrow_pressed = false;
        lpx_sysex_cc_button_default(93);
        print_left_or_right_arrow_released();
      }
    }

    if (controller == 94) {
      if (value == 127) {
        right_arrow_pressed = true;
        lpx_sysex_cc_button_white(94);
        if (left_arrow_pressed == true) {
          pad_layout_shift = 4;
        } else pad_layout_shift -= 1;
        refresh_pads = true;
        Serial << " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " << endl << " * * * " << endl;
      }
      if (value == 0) {
        right_arrow_pressed = false;
        lpx_sysex_cc_button_default(94);
        print_left_or_right_arrow_released();
      }
    }
    // Layout shift wrap-around:
    if (pad_layout_shift > 24) pad_layout_shift = -35;
    if (pad_layout_shift < -35) pad_layout_shift = 24;
  }

  // Change the layout_difference::
  layout_difference = pad_layout_shift - old_layout_shift;

  /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

  refresh_all_pads_function();



  /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */
  
};

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||         MIDI CALLBACKS          || — — — — — — — — — — — || */  

struct MyMIDI_Callbacks : FineGrainedMIDI_Callbacks<MyMIDI_Callbacks> {
  // Note how this ^ name is identical to the argument used here ^
  
  /* #region    || — — — — — — — — — — ||        NOTE ON         || — — — — — — — — — — — || */ 

  void onNoteOn(Channel channel, uint8_t pad, uint8_t velocity, Cable cable) {

    /// Start a timer:
    myTime_1 = micros();

    // Call the midi note-in-handler:
    midi_note_processing(pad, velocity);
    
    /// Clock and print the timer:
    myTime_2 = micros() - myTime_1;
    Serial << "Time: " << myTime_2 / 1000 << " ms" << endl;

  };
  
  /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

  /* #region    || — — — — — — — — — — ||     CONTROL CHANGE     || — — — — — — — — — — — || */
  
  void onControlChange(Channel channel, uint8_t controller, uint8_t value, Cable cable) {
    /// Start a timer:
    myTime_1 = micros();

    // Light the control change buttons when pushed:
    for (uint8_t i = 0; i < (sizeof(cc_buttons) / sizeof(cc_buttons[0])); i++) {
      if (controller == cc_buttons[i]) {
        if (value != 0) {
          /// Light up the pushed pad:
          lpx_sysex_cc_button_white(controller);
        } else {
          // If value is 0:
          lpx_sysex_cc_button_default(controller);
        } 
      }
    }

    control_change_processing(controller, value);

    Serial << endl;
    Serial << "CC Knobs: " << controller << "   Value: " << value << "   Channel: " << channel << "   Cable: " << cable;
    Serial << endl;

    /// Clock and print the timer:
    myTime_2 = micros() - myTime_1;
    Serial << "Time: " << myTime_2 / 1000 << " ms   " << myTime_2 << " micro" << endl;
  };

  /// Pitchbend:
  //   void onPitchBend(Channel channel, uint16_t bend, Cable cable) {
  //   }

  /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

  void onChannelPressure(Channel channel, uint8_t pressure, Cable cable) {
  
    // usbmidi.sendChannelPressure(channel, pressure);
    // dinmidi.sendChannelPressure(channel, keypressure_filter.Current());

  };

  void onKeyPressure(Channel channel, uint8_t pad, uint8_t pressure, Cable cable) {
    
    uint8_t note = pad_to_midi_processing_table(pad) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose;
   
    // keypressure_filter.Filter(pressure);

    usbmidi.sendKeyPressure(attach_midi_channel_to_note(note), pressure);
    dinmidi.sendKeyPressure(attach_midi_channel_to_note(note), pressure);
    // dinmidi.sendKeyPressure(attach_midi_channel_to_note(note), keypressure_filter.Current());
    
    
  };

  void onSysExMessage(MIDI_Interface &, SysExMessage sysex) override {
    // Print the message
    Serial << "Received SysEx message: "
           << AH::HexDump(sysex.data, sysex.length) //
           << endl;

          //  << "sysex.data: " << sysex.data
          //  << endl
          //  << "sysex.start: " << sysex.SYSEX_START
          //  << endl
          //  << "sysex.end: " << sysex.SYSEX_END
          //  << endl
          //  << endl
          //  << endl;

//     hstmidi.sendSysEx(lpx_device_inquiry);
// 
//     Serial << "Send: "
//            << lpx_device_inquiry
//            << endl
//           //  << endl
//           //  << F(" on cable ") << sysex.cable.getOneBased()
//            << endl;

// Got this back...
// F0 7E 00 06 02 00 20 29 03 01 00 00 00 04 02 02 F7
    
    
//     if (AH::HexDump(sysex.SYSEX_START) == respons_if_lpx_is_connected_test) 
// 
//     // if (AH::HexDump(sysex.data, sysex.length) == respons_if_lpx_is_connected_test){
//       // uint8_t lpx_programmer_mode[9] = { 240, 0, 32, 41, 2, 12, 14, 1, 247 };
//       // hstmidi.sendSysEx(lpx_programmer_mode);
//       // for (uint8_t i = 0; i < 40; i++) { send_led_sysex_to_launchpad(white_keys[i], lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }
//       // lpx_sysex_light_cc_buttons();
//       Serial << "Launchpad connected!" << endl;
    // }
  };

} callback; 

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||          POTENTIOMETERS         || — — — — — — — — — — — || */

/// Potentiometers:

// What interface are we outputting to?!

FilteredAnalog<10, 6, uint32_t> analog16 {A16}; 
// FilteredAnalog<> analog16 {A16};
FilteredAnalog<10, 6, uint32_t> analog17{ A17 };

FilteredAnalog<10, 6, uint32_t> analog15{ A15 };

FilteredAnalog<10, 6, uint32_t> analog14{ A14 };

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||            ANIMATION            || — — — — — — — — — — — || */

void print_info() {
  //
  }


uint8_t animation_cue_prev() {

  uint8_t anim01[] = {                             18 };
  uint8_t anim02[] = {                         17, 28 };
  uint8_t anim03[] = {                     16, 27, 38 };
  uint8_t anim04[] = {                 15, 26, 37, 48 };
  uint8_t anim05[] = {             14, 25, 36, 47, 58 };
  uint8_t anim06[] = {         13, 24, 35, 46, 57, 68 };
  uint8_t anim07[] = {     12, 23, 34, 45, 56, 67, 78 };
  uint8_t anim08[] = { 11, 22, 33, 44, 55, 66, 77, 88 };
  uint8_t anim09[] = {     21, 32, 43, 54, 65, 76, 87 };
  uint8_t anim10[] = {         31, 42, 53, 64, 75, 86 };
  uint8_t anim11[] = {             41, 52, 63, 74, 85 };
  uint8_t anim12[] = {                 51, 62, 73, 84 };
  uint8_t anim13[] = {                     61, 72, 83 };
  uint8_t anim14[] = {                         71, 82 };
  uint8_t anim15[] = {                             81 };

  
  static unsigned long noiasca_millis = 0;
  static uint8_t state = 33;

  uint8_t color_lines[] = {127, 127, 127};
  uint8_t c0[] = {  0,  0,  0 };
  uint8_t c1[] = {  8,  8,  8 };
  uint8_t c2[] = { 64, 64, 64 };
  uint8_t c3[] = { 16, 16, 16 };
  uint8_t c4[] = {127,127,127 };


  uint8_t c7[] = {  43, 16,  0 };
  uint8_t c8[] = {  52, 24,  0 };
  uint8_t c9[] = {  96, 32,  0 };
  
  if (millis() - noiasca_millis >= a_time) {
    state++;
    state = state % 33;
    Serial << endl << "——————————————————————————— State: " << state << endl;

    switch (state) {
      case 0:  for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };     break;
      
      case 1:  for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                                                                                                                                   send_led_sysex_to_one_pad(18, c0[0],c0[1],c0[2]); break;
      case 2:  for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                                                                                 send_led_sysex_to_one_pad(28, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(17, c0[0],c0[1],c0[2]); break;
      case 3:  for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                               send_led_sysex_to_one_pad(38, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(27, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(16, c0[0],c0[1],c0[2]); break;
      case 4:  for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                             send_led_sysex_to_one_pad(48, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(37, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(26, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(15, c0[0],c0[1],c0[2]); break;
      case 5:  for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                           send_led_sysex_to_one_pad(58, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(47, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(36, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(25, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(14, c0[0],c0[1],c0[2]); break;
      case 6:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                         send_led_sysex_to_one_pad(68, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(57, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(46, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(35, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(24, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(13, c0[0],c0[1],c0[2]); break;
      case 7:  for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                       send_led_sysex_to_one_pad(78, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(67, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(56, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(45, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(34, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(23, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(12, c0[0],c0[1],c0[2]); break;
      case 9:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(88, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(77, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(66, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(55, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(44, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(33, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(22, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(11, c0[0],c0[1],c0[2]); break;
      case 10: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(87, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(76, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(65, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(54, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(43, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(32, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(21, c7[0],c7[1],c7[2]);                                                   break;
      case 11: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(86, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(75, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(64, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(53, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(42, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(31, c7[0],c7[1],c7[2]);                                                                                                     break;
      case 12: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(85, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(74, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(63, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(52, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(41, c8[0],c8[1],c8[2]);                                                                                                                                                       break;
      case 13: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(84, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(73, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(62, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(51, c8[0],c8[1],c8[2]);                                                                                                                                                                                                         break;
      case 14: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(83, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(72, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(61, c8[0],c8[1],c8[2]);                                                                                                                                                                                                                                                           break;
      case 15: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(82, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(71, c9[0],c9[1],c9[2]);                                                                                                                                                                                                                                                                                                             break;
      case 16: /* — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — */      send_led_sysex_to_one_pad(81, c0[0], c0[1], c0[2]);                                                                                                                                                                                                                                                                                                                                                             break;
      
      case 17: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                            break;
      case 18: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim01[pad]); }; break;
      case 19: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim02[pad]); }; break;
      case 20: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim03[pad]); }; break;
      case 21: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim04[pad]); }; break;
      case 22: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim05[pad]); }; break;
      case 23: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim06[pad]); }; break;
      case 24: for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim07[pad]); }; break;
      case 25: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 8; pad++) { refresh_a_pad(anim08[pad]); }; break;
      case 26: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim09[pad]); }; break;
      case 27: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim10[pad]); }; break;
      case 28: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim11[pad]); }; break;
      case 29: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim12[pad]); }; break;
      case 30: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim13[pad]); }; break;
      case 31: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim14[pad]); }; break;
      case 32:                                                                                                                                        for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim15[pad]); }; animation_in_progress = false; animation_1_in_progress = false; refresh_pads = true; refresh_all_pads_function(); return 0;
    }
    noiasca_millis = millis();
  }
  return 0;
}












uint8_t animation_bar_prev() {

  uint8_t anim01[] = {                             18 };
  uint8_t anim02[] = {                         17, 28 };
  uint8_t anim03[] = {                     16, 27, 38 };
  uint8_t anim04[] = {                 15, 26, 37, 48 };
  uint8_t anim05[] = {             14, 25, 36, 47, 58 };
  uint8_t anim06[] = {         13, 24, 35, 46, 57, 68 };
  uint8_t anim07[] = {     12, 23, 34, 45, 56, 67, 78 };
  uint8_t anim08[] = { 11, 22, 33, 44, 55, 66, 77, 88 };
  uint8_t anim09[] = {     21, 32, 43, 54, 65, 76, 87 };
  uint8_t anim10[] = {         31, 42, 53, 64, 75, 86 };
  uint8_t anim11[] = {             41, 52, 63, 74, 85 };
  uint8_t anim12[] = {                 51, 62, 73, 84 };
  uint8_t anim13[] = {                     61, 72, 83 };
  uint8_t anim14[] = {                         71, 82 };
  uint8_t anim15[] = {                             81 };

  
  static unsigned long noiasca_millis = 0;
  static uint8_t state = 33;

  uint8_t color_lines[] = {127, 127, 127};
  uint8_t c0[] = {  0,  0,  0 };
  uint8_t c1[] = { 96,112,112 };
  uint8_t c2[] = {112,119,127 }; 
  uint8_t c3[] = {127, 64,  0 };
  uint8_t c4[] = {127, 80, 32 };
  uint8_t c5[] = {127,111, 96 };
  uint8_t c6[] = {127,100, 64 };
  uint8_t c7[] = {127,108, 80 };
  
  if (millis() - noiasca_millis >= a_time) {
    state++;
    state = state % 33;
    Serial << endl << "——————————————————————————— State: " << state << endl;

    switch (state) {
      case 0:  for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };     break;
      
      case 1:  for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                                                                                                                                   send_led_sysex_to_one_pad(18, c0[0],c0[1],c0[2]); break;
      case 2:  for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                                                                                 send_led_sysex_to_one_pad(28, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(17, c0[0],c0[1],c0[2]); break;
      case 3:  for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                               send_led_sysex_to_one_pad(38, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(27, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(16, c0[0],c0[1],c0[2]); break;
      case 4:  for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                             send_led_sysex_to_one_pad(48, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(37, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(26, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(15, c0[0],c0[1],c0[2]); break;
      case 5:  for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                           send_led_sysex_to_one_pad(58, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(47, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(36, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(25, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(14, c0[0],c0[1],c0[2]); break;
      case 6:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                         send_led_sysex_to_one_pad(68, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(57, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(46, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(35, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(24, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(13, c0[0],c0[1],c0[2]); break;
      case 7:  for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                       send_led_sysex_to_one_pad(78, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(67, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(56, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(45, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(34, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(23, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(12, c0[0],c0[1],c0[2]); break;
      case 9:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(88, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(77, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(66, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(55, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(44, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(33, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(22, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(11, c0[0],c0[1],c0[2]); break;
      case 10: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(87, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(76, c7[0],c7[1],c7[2]); send_led_sysex_to_one_pad(65, c5[0],c5[1],c5[2]); send_led_sysex_to_one_pad(54, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(43, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(32, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(21, c0[0],c0[1],c0[2]);                                                   break;
      case 11: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(86, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(75, c7[0],c7[1],c7[2]); send_led_sysex_to_one_pad(64, c6[0],c6[1],c6[2]); send_led_sysex_to_one_pad(53, c5[0],c5[1],c5[2]); send_led_sysex_to_one_pad(42, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(31, c0[0],c0[1],c0[2]);                                                                                                     break;
      case 12: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(85, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(74, c7[0],c7[1],c7[2]); send_led_sysex_to_one_pad(63, c6[0],c6[1],c6[2]); send_led_sysex_to_one_pad(52, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(41, c0[0],c0[1],c0[2]);                                                                                                                                                       break;
      case 13: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(84, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(73, c7[0],c7[1],c7[2]); send_led_sysex_to_one_pad(62, c7[0],c7[1],c7[2]); send_led_sysex_to_one_pad(51, c0[0],c0[1],c0[2]);                                                                                                                                                                                                         break;
      case 14: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(83, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(72, c7[0],c7[1],c7[2]); send_led_sysex_to_one_pad(61, c2[0],c2[1],c2[2]);                                                                                                                                                                                                                                                           break;
      case 15: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(82, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(71, c2[0],c2[1],c2[2]);                                                                                                                                                                                                                                                                                                             break;
      case 16: /* — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — */      send_led_sysex_to_one_pad(81, c0[0],c0[1],c0[2]);                                                                                                                                                                                                                                                                                                                                                             break;
      
      case 17: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                            break;
      case 18: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim01[pad]); }; break;
      case 19: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim02[pad]); }; break;
      case 20: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim03[pad]); }; break;
      case 21: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim04[pad]); }; break;
      case 22: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim05[pad]); }; break;
      case 23: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim06[pad]); }; break;
      case 24: for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim07[pad]); }; break;
      case 25: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 8; pad++) { refresh_a_pad(anim08[pad]); }; break;
      case 26: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim09[pad]); }; break;
      case 27: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim10[pad]); }; break;
      case 28: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim11[pad]); }; break;
      case 29: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim12[pad]); }; break;
      case 30: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim13[pad]); }; break;
      case 31: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim14[pad]); }; break;
      case 32:                                                                                                                                        for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim15[pad]); }; animation_in_progress = false; animation_3_in_progress = false; refresh_pads = true; refresh_all_pads_function(); return 0;
    }
    noiasca_millis = millis();
  }
  return 0;
}












uint8_t animation_beat_prev() {

  uint8_t anim01[] = {                             18 };
  uint8_t anim02[] = {                         17, 28 };
  uint8_t anim03[] = {                     16, 27, 38 };
  uint8_t anim04[] = {                 15, 26, 37, 48 };
  uint8_t anim05[] = {             14, 25, 36, 47, 58 };
  uint8_t anim06[] = {         13, 24, 35, 46, 57, 68 };
  uint8_t anim07[] = {     12, 23, 34, 45, 56, 67, 78 };
  uint8_t anim08[] = { 11, 22, 33, 44, 55, 66, 77, 88 };
  uint8_t anim09[] = {     21, 32, 43, 54, 65, 76, 87 };
  uint8_t anim10[] = {         31, 42, 53, 64, 75, 86 };
  uint8_t anim11[] = {             41, 52, 63, 74, 85 };
  uint8_t anim12[] = {                 51, 62, 73, 84 };
  uint8_t anim13[] = {                     61, 72, 83 };
  uint8_t anim14[] = {                         71, 82 };
  uint8_t anim15[] = {                             81 };

  
  static unsigned long noiasca_millis = 0;
  static uint8_t state = 33;

  uint8_t color_lines[] = {127, 127, 127};
  uint8_t c0[] = {  0,  0,  0 };
  uint8_t c1[] = {  8,  8,  8 };
  uint8_t c2[] = { 64, 64, 64 };
  uint8_t c3[] = { 16, 16, 16 };
  uint8_t c4[] = {127,127,127 };


  uint8_t c7[] = {  43, 16,  0 };
  uint8_t c8[] = {  52, 24,  0 };
  uint8_t c9[] = {  96, 32,  0 };
  
  if (millis() - noiasca_millis >= a_time) {
    state++;
    state = state % 33;
    Serial << endl << "——————————————————————————— State: " << state << endl;

    switch (state) {
      case 0:  for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };     break;
      
      case 1:  for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                                                                                                                                   send_led_sysex_to_one_pad(18, c0[0],c0[1],c0[2]); break;
      case 2:  for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                                                                                 send_led_sysex_to_one_pad(28, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(17, c0[0],c0[1],c0[2]); break;
      case 3:  for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                               send_led_sysex_to_one_pad(38, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(27, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(16, c0[0],c0[1],c0[2]); break;
      case 4:  for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                             send_led_sysex_to_one_pad(48, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(37, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(26, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(15, c0[0],c0[1],c0[2]); break;
      case 5:  for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                           send_led_sysex_to_one_pad(58, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(47, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(36, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(25, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(14, c0[0],c0[1],c0[2]); break;
      case 6:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                         send_led_sysex_to_one_pad(68, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(57, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(46, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(35, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(24, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(13, c0[0],c0[1],c0[2]); break;
      case 7:  for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                       send_led_sysex_to_one_pad(78, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(67, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(56, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(45, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(34, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(23, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(12, c0[0],c0[1],c0[2]); break;
      case 9:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(88, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(77, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(66, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(55, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(44, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(33, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(22, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(11, c0[0],c0[1],c0[2]); break;
      case 10: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(87, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(76, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(65, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(54, c3[0],c3[1],c3[2]); send_led_sysex_to_one_pad(43, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(32, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(21, c7[0],c7[1],c7[2]);                                                   break;
      case 11: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(86, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(75, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(64, c4[0],c4[1],c4[2]); send_led_sysex_to_one_pad(53, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(42, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(31, c7[0],c7[1],c7[2]);                                                                                                     break;
      case 12: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(85, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(74, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(63, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(52, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(41, c8[0],c8[1],c8[2]);                                                                                                                                                       break;
      case 13: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(84, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(73, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(62, c2[0],c2[1],c2[2]); send_led_sysex_to_one_pad(51, c8[0],c8[1],c8[2]);                                                                                                                                                                                                         break;
      case 14: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(83, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(72, c1[0],c1[1],c1[2]); send_led_sysex_to_one_pad(61, c8[0],c8[1],c8[2]);                                                                                                                                                                                                                                                           break;
      case 15: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(82, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(71, c9[0],c9[1],c9[2]);                                                                                                                                                                                                                                                                                                             break;
      case 16: /* — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — */      send_led_sysex_to_one_pad(81, c0[0], c0[1], c0[2]);                                                                                                                                                                                                                                                                                                                                                             break;
      
      case 17: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                            break;
      case 18: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim01[pad]); }; break;
      case 19: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim02[pad]); }; break;
      case 20: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim03[pad]); }; break;
      case 21: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim04[pad]); }; break;
      case 22: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim05[pad]); }; break;
      case 23: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim06[pad]); }; break;
      case 24: for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim07[pad]); }; break;
      case 25: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 8; pad++) { refresh_a_pad(anim08[pad]); }; break;
      case 26: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim09[pad]); }; break;
      case 27: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim10[pad]); }; break;
      case 28: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim11[pad]); }; break;
      case 29: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim12[pad]); }; break;
      case 30: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim13[pad]); }; break;
      case 31: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim14[pad]); }; break;
      case 32:                                                                                                                                        for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim15[pad]); }; animation_in_progress = false; animation_5_in_progress = false; refresh_pads = true; refresh_all_pads_function(); return 0;
    }
    noiasca_millis = millis();
  }
  return 0;
}












uint8_t animation_cue_next() {

  uint8_t anim01[] = { 81                             };
  uint8_t anim02[] = { 82, 71                         };
  uint8_t anim03[] = { 83, 72, 61                     };
  uint8_t anim04[] = { 84, 73, 62, 51                 };
  uint8_t anim05[] = { 85, 74, 63, 52, 41             };
  uint8_t anim06[] = { 86, 75, 64, 53, 42, 31         };
  uint8_t anim07[] = { 87, 76, 65, 54, 43, 32, 21     };
  uint8_t anim08[] = { 88, 77, 66, 55, 44, 33, 22, 11 };
  uint8_t anim09[] = {     78, 67, 56, 45, 34, 23, 12 };
  uint8_t anim10[] = {         68, 57, 46, 35, 24, 13 };
  uint8_t anim11[] = {             58, 47, 36, 25, 14 };
  uint8_t anim12[] = {                 48, 37, 26, 15 };
  uint8_t anim13[] = {                     38, 27, 16 };
  uint8_t anim14[] = {                         28, 17 };
  uint8_t anim15[] = {                             18 };

  static unsigned long noiasca_millis = 0;
  static uint8_t state = 33;

  uint8_t color_lines[] = { 127, 127, 127 };
  uint8_t c0[] = { 0,  0,  0 };
  uint8_t c1[] = { 8,  8,  8 };
  uint8_t c2[] = { 64, 64, 64 };
  uint8_t c3[] = { 16, 16, 16 };
  uint8_t c4[] = { 127,127,127 };


  uint8_t c7[] = { 43, 16,  0 };
  uint8_t c8[] = { 52, 24,  0 };
  uint8_t c9[] = { 96, 32,  0 };

  if (millis() - noiasca_millis >= a_time) {
    state++;
    state = state % 33;
    Serial << endl << "——————————————————————————— State: " << state << endl;

    switch (state) {
      case 0:  for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };     break;

      case 1:  for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(81, c0[0],c0[1],c0[2]);                                                                                                                                                                                                                                                                                                                                                               break;
      case 2:  for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(82, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(71, c0[0],c0[1],c0[2]);                                                                                                                                                                                                                                                                                                             break;
      case 3:  for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(83, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(72, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(61, c0[0],c0[1],c0[2]);                                                                                                                                                                                                                                                           break;
      case 4:  for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(84, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(73, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(62, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(51, c0[0],c0[1],c0[2]);                                                                                                                                                                                                         break;
      case 5:  for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(85, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(74, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(63, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(52, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(41, c0[0],c0[1],c0[2]);                                                                                                                                                       break;
      case 6:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(86, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(75, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(64, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(53, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(42, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(31, c0[0],c0[1],c0[2]);                                                                                                     break;
      case 7:  for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(87, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(76, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(65, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(54, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(43, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(32, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(21, c0[0],c0[1],c0[2]);                                                   break;
      case 9:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(88, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(77, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(66, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(55, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(44, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(33, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(22, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(11, c0[0],c0[1],c0[2]); break;
      case 10: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                       send_led_sysex_to_one_pad(78, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(67, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(56, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(45, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(34, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(23, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(12, c0[0],c0[1],c0[2]); break;
      case 11: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                         send_led_sysex_to_one_pad(68, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(57, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(46, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(35, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(24, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(13, c0[0],c0[1],c0[2]); break;
      case 12: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                           send_led_sysex_to_one_pad(58, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(47, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(36, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(25, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(14, c0[0],c0[1],c0[2]); break;
      case 13: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                             send_led_sysex_to_one_pad(48, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(37, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(26, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(15, c0[0],c0[1],c0[2]); break;
      case 14: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                               send_led_sysex_to_one_pad(38, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(27, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(16, c0[0],c0[1],c0[2]); break;
      case 15: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                                                                                 send_led_sysex_to_one_pad(28, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(17, c0[0],c0[1],c0[2]); break;
      case 16: /* — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — */                                                                                                                                                                                                                                                                                                                                                                    send_led_sysex_to_one_pad(18, c0[0],c0[1],c0[2]); break;

      case 17: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                            break;
      case 18: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim01[pad]); }; break;
      case 19: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim02[pad]); }; break;
      case 20: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim03[pad]); }; break;
      case 21: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim04[pad]); }; break;
      case 22: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim05[pad]); }; break;
      case 23: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim06[pad]); }; break;
      case 24: for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim07[pad]); }; break;
      case 25: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 8; pad++) { refresh_a_pad(anim08[pad]); }; break;
      case 26: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim09[pad]); }; break;
      case 27: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim10[pad]); }; break;
      case 28: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim11[pad]); }; break;
      case 29: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim12[pad]); }; break;
      case 30: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim13[pad]); }; break;
      case 31: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim14[pad]); }; break;
      case 32:                                                                                                                                        for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim15[pad]); }; animation_in_progress = false; animation_2_in_progress = false; refresh_pads = true; refresh_all_pads_function(); return 0;
      }
    noiasca_millis = millis();
    }
  return 0;
  }












  uint8_t animation_bar_next() {

  uint8_t anim01[] = { 81                             };
  uint8_t anim02[] = { 82, 71                         };
  uint8_t anim03[] = { 83, 72, 61                     };
  uint8_t anim04[] = { 84, 73, 62, 51                 };
  uint8_t anim05[] = { 85, 74, 63, 52, 41             };
  uint8_t anim06[] = { 86, 75, 64, 53, 42, 31         };
  uint8_t anim07[] = { 87, 76, 65, 54, 43, 32, 21     };
  uint8_t anim08[] = { 88, 77, 66, 55, 44, 33, 22, 11 };
  uint8_t anim09[] = {     78, 67, 56, 45, 34, 23, 12 };
  uint8_t anim10[] = {         68, 57, 46, 35, 24, 13 };
  uint8_t anim11[] = {             58, 47, 36, 25, 14 };
  uint8_t anim12[] = {                 48, 37, 26, 15 };
  uint8_t anim13[] = {                     38, 27, 16 };
  uint8_t anim14[] = {                         28, 17 };
  uint8_t anim15[] = {                             18 };

  static unsigned long noiasca_millis = 0;
  static uint8_t state = 33;

  uint8_t color_lines[] = { 127, 127, 127 };
  uint8_t c0[] = { 0,  0,  0 };
  uint8_t c1[] = { 8,  8,  8 };
  uint8_t c2[] = { 64, 64, 64 };
  uint8_t c3[] = { 16, 16, 16 };
  uint8_t c4[] = { 127,127,127 };


  uint8_t c7[] = { 43, 16,  0 };
  uint8_t c8[] = { 52, 24,  0 };
  uint8_t c9[] = { 96, 32,  0 };

  if (millis() - noiasca_millis >= a_time) {
    state++;
    state = state % 33;
    Serial << endl << "——————————————————————————— State: " << state << endl;

    switch (state) {
      case 0:  for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };     break;

      case 1:  for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(81, c0[0],c0[1],c0[2]);                                                                                                                                                                                                                                                                                                                                                               break;
      case 2:  for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(82, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(71, c0[0],c0[1],c0[2]);                                                                                                                                                                                                                                                                                                             break;
      case 3:  for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(83, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(72, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(61, c0[0],c0[1],c0[2]);                                                                                                                                                                                                                                                           break;
      case 4:  for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(84, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(73, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(62, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(51, c0[0],c0[1],c0[2]);                                                                                                                                                                                                         break;
      case 5:  for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(85, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(74, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(63, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(52, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(41, c0[0],c0[1],c0[2]);                                                                                                                                                       break;
      case 6:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(86, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(75, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(64, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(53, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(42, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(31, c0[0],c0[1],c0[2]);                                                                                                     break;
      case 7:  for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(87, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(76, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(65, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(54, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(43, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(32, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(21, c0[0],c0[1],c0[2]);                                                   break;
      case 9:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(88, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(77, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(66, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(55, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(44, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(33, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(22, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(11, c0[0],c0[1],c0[2]); break;
      case 10: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                       send_led_sysex_to_one_pad(78, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(67, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(56, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(45, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(34, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(23, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(12, c0[0],c0[1],c0[2]); break;
      case 11: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                         send_led_sysex_to_one_pad(68, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(57, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(46, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(35, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(24, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(13, c0[0],c0[1],c0[2]); break;
      case 12: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                           send_led_sysex_to_one_pad(58, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(47, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(36, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(25, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(14, c0[0],c0[1],c0[2]); break;
      case 13: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                             send_led_sysex_to_one_pad(48, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(37, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(26, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(15, c0[0],c0[1],c0[2]); break;
      case 14: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                               send_led_sysex_to_one_pad(38, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(27, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(16, c0[0],c0[1],c0[2]); break;
      case 15: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                                                                                 send_led_sysex_to_one_pad(28, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(17, c0[0],c0[1],c0[2]); break;
      case 16: /* — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — */                                                                                                                                                                                                                                                                                                                                                                    send_led_sysex_to_one_pad(18, c0[0],c0[1],c0[2]); break;

      case 17: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                            break;
      case 18: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim01[pad]); }; break;
      case 19: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim02[pad]); }; break;
      case 20: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim03[pad]); }; break;
      case 21: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim04[pad]); }; break;
      case 22: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim05[pad]); }; break;
      case 23: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim06[pad]); }; break;
      case 24: for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim07[pad]); }; break;
      case 25: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 8; pad++) { refresh_a_pad(anim08[pad]); }; break;
      case 26: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim09[pad]); }; break;
      case 27: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim10[pad]); }; break;
      case 28: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim11[pad]); }; break;
      case 29: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim12[pad]); }; break;
      case 30: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim13[pad]); }; break;
      case 31: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim14[pad]); }; break;
      case 32:                                                                                                                                        for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim15[pad]); }; animation_in_progress = false; animation_4_in_progress = false; refresh_pads = true; refresh_all_pads_function(); return 0;
    }
    noiasca_millis = millis();
  }
  return 0;
}












uint8_t animation_beat_next() {

  uint8_t anim01[] = { 81                             };
  uint8_t anim02[] = { 82, 71                         };
  uint8_t anim03[] = { 83, 72, 61                     };
  uint8_t anim04[] = { 84, 73, 62, 51                 };
  uint8_t anim05[] = { 85, 74, 63, 52, 41             };
  uint8_t anim06[] = { 86, 75, 64, 53, 42, 31         };
  uint8_t anim07[] = { 87, 76, 65, 54, 43, 32, 21     };
  uint8_t anim08[] = { 88, 77, 66, 55, 44, 33, 22, 11 };
  uint8_t anim09[] = {     78, 67, 56, 45, 34, 23, 12 };
  uint8_t anim10[] = {         68, 57, 46, 35, 24, 13 };
  uint8_t anim11[] = {             58, 47, 36, 25, 14 };
  uint8_t anim12[] = {                 48, 37, 26, 15 };
  uint8_t anim13[] = {                     38, 27, 16 };
  uint8_t anim14[] = {                         28, 17 };
  uint8_t anim15[] = {                             18 };

  static unsigned long noiasca_millis = 0;
  static uint8_t state = 33;

  uint8_t color_lines[] = { 127, 127, 127 };
  uint8_t c0[] = { 0,  0,  0 };
  uint8_t c1[] = { 8,  8,  8 };
  uint8_t c2[] = { 64, 64, 64 };
  uint8_t c3[] = { 16, 16, 16 };
  uint8_t c4[] = { 127,127,127 };


  uint8_t c7[] = { 43, 16,  0 };
  uint8_t c8[] = { 52, 24,  0 };
  uint8_t c9[] = { 96, 32,  0 };

  if (millis() - noiasca_millis >= a_time) {
    state++;
    state = state % 33;
    Serial << endl << "——————————————————————————— State: " << state << endl;

    switch (state) {
      case 0:  for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };     break;

      case 1:  for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(81, c0[0],c0[1],c0[2]);                                                                                                                                                                                                                                                                                                                                                               break;
      case 2:  for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(82, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(71, c0[0],c0[1],c0[2]);                                                                                                                                                                                                                                                                                                             break;
      case 3:  for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(83, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(72, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(61, c0[0],c0[1],c0[2]);                                                                                                                                                                                                                                                           break;
      case 4:  for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(84, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(73, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(62, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(51, c0[0],c0[1],c0[2]);                                                                                                                                                                                                         break;
      case 5:  for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(85, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(74, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(63, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(52, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(41, c0[0],c0[1],c0[2]);                                                                                                                                                       break;
      case 6:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(86, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(75, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(64, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(53, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(42, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(31, c0[0],c0[1],c0[2]);                                                                                                     break;
      case 7:  for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(87, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(76, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(65, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(54, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(43, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(32, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(21, c0[0],c0[1],c0[2]);                                                   break;
      case 9:  for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     send_led_sysex_to_one_pad(88, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(77, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(66, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(55, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(44, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(33, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(22, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(11, c0[0],c0[1],c0[2]); break;
      case 10: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                       send_led_sysex_to_one_pad(78, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(67, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(56, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(45, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(34, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(23, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(12, c0[0],c0[1],c0[2]); break;
      case 11: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                         send_led_sysex_to_one_pad(68, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(57, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(46, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(35, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(24, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(13, c0[0],c0[1],c0[2]); break;
      case 12: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                           send_led_sysex_to_one_pad(58, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(47, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(36, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(25, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(14, c0[0],c0[1],c0[2]); break;
      case 13: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                             send_led_sysex_to_one_pad(48, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(37, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(26, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(15, c0[0],c0[1],c0[2]); break;
      case 14: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                               send_led_sysex_to_one_pad(38, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(27, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(16, c0[0],c0[1],c0[2]); break;
      case 15: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                                                                                                                                                                                                                                                 send_led_sysex_to_one_pad(28, c0[0],c0[1],c0[2]); send_led_sysex_to_one_pad(17, c0[0],c0[1],c0[2]); break;
      case 16: /* — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — */                                                                                                                                                                                                                                                                                                                                                                    send_led_sysex_to_one_pad(18, c0[0],c0[1],c0[2]); break;

      case 17: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim01[pad], color_lines[0], color_lines[1], color_lines[2]); };                                                                                            break;
      case 18: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim02[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim01[pad]); }; break;
      case 19: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim03[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim02[pad]); }; break;
      case 20: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim04[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim03[pad]); }; break;
      case 21: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim05[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim04[pad]); }; break;
      case 22: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim06[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim05[pad]); }; break;
      case 23: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim07[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim06[pad]); }; break;
      case 24: for (uint8_t pad = 0; pad < 8; pad++) { send_led_sysex_to_one_pad(anim08[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim07[pad]); }; break;
      case 25: for (uint8_t pad = 0; pad < 7; pad++) { send_led_sysex_to_one_pad(anim09[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 8; pad++) { refresh_a_pad(anim08[pad]); }; break;
      case 26: for (uint8_t pad = 0; pad < 6; pad++) { send_led_sysex_to_one_pad(anim10[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 7; pad++) { refresh_a_pad(anim09[pad]); }; break;
      case 27: for (uint8_t pad = 0; pad < 5; pad++) { send_led_sysex_to_one_pad(anim11[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 6; pad++) { refresh_a_pad(anim10[pad]); }; break;
      case 28: for (uint8_t pad = 0; pad < 4; pad++) { send_led_sysex_to_one_pad(anim12[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 5; pad++) { refresh_a_pad(anim11[pad]); }; break;
      case 29: for (uint8_t pad = 0; pad < 3; pad++) { send_led_sysex_to_one_pad(anim13[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 4; pad++) { refresh_a_pad(anim12[pad]); }; break;
      case 30: for (uint8_t pad = 0; pad < 2; pad++) { send_led_sysex_to_one_pad(anim14[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 3; pad++) { refresh_a_pad(anim13[pad]); }; break;
      case 31: for (uint8_t pad = 0; pad < 1; pad++) { send_led_sysex_to_one_pad(anim15[pad], color_lines[0], color_lines[1], color_lines[2]); };     for (uint8_t pad = 0; pad < 2; pad++) { refresh_a_pad(anim14[pad]); }; break;
      case 32:                                                                                                                                        for (uint8_t pad = 0; pad < 1; pad++) { refresh_a_pad(anim15[pad]); }; animation_in_progress = false; animation_6_in_progress = false; refresh_pads = true; refresh_all_pads_function(); return 0;
    }
    noiasca_millis = millis();
  }
  return 0;
}





void update_animation() {
  
  if (animation_1_in_progress == true){
    static uint8_t animation_1_state = 0;
    switch (animation_1_state) {
      case 0:
        animation_1_state = animation_cue_prev();
        break;
    }
  }

  if (animation_2_in_progress == true){ 
      static uint8_t animation_2_state = 0;
      switch (animation_2_state) {
        case 0:
        animation_2_state = animation_cue_next(); 
        break;  
    }
  }

  if (animation_3_in_progress == true) {
    static uint8_t animation_3_state = 0;
    switch (animation_3_state) {
      case 0:
        animation_3_state = animation_bar_prev();
        break;
    }
  }

  if (animation_4_in_progress == true) {
    static uint8_t animation_4_state = 0;
    switch (animation_4_state) {
      case 0:
        animation_4_state = animation_bar_next();
        break;
    }
  }

  if (animation_5_in_progress == true) {
    static uint8_t animation_5_state = 0;
    switch (animation_5_state) {
      case 0:
        animation_5_state = animation_beat_prev();
        break;
    }
  }

  if (animation_6_in_progress == true) {
    static uint8_t animation_6_state = 0;
    switch (animation_6_state) {
      case 0:
        animation_6_state = animation_beat_next();
        break;
    }
  }

  // These do not work...
  // if (animation_2_in_progress == true) { animation_cue_next(); }
  // if (animation_3_in_progress == true){ animation_bar_prev(); }
  // if (animation_4_in_progress == true){ animation_bar_next(); }
  // if (animation_5_in_progress == true){ animation_beat_prev(); }
  // if (animation_6_in_progress == true){ animation_beat_next(); }


};
/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */





/* #region    || — — — — — — — — — — ||            SEQUENCER            || — — — — — — — — — — — || */


// uint8_t sixteenth_note[32] = { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 } ;

// uint8_t quarter_note[8] = { 0, 0, 0, 0,  0, 0, 0, 0, } ;

uint8_t slot[2][4][4] = {
  { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
  { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
};


// // Sequencer data
// typedef struct sequencer_step_data {
//   bool rest;
//   uint8_t note;
//   bool accent;
// };

// sequencer_step_data sequencer[2][4][4] = {
//   { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
//   { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
// };




// void play_step(uint8_t seq_note, uint8_t old_seq_note){

//     usbmidi.sendNoteOff(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
//     dinmidi.sendNoteOff(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;

//     usbmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
//     dinmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
            
// };

// for (uint8_t         bar = 0       ; bar < 1       ; bar++       ) {
//     for (uint8_t     beat = 0      ; beat < 3      ; beat++      ) {
//         for (uint8_t sixteenth = 0 ; sixteenth < 3 ; sixteenth++ ) { 
//             usbmidi.sendNoteOff(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
//             dinmidi.sendNoteOff(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
            
//             uint8_t seq_note = slot[bar][beat][sixteenth] ;
//             usbmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
//             dinmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
            
//             uint8_t prev_seq_note = seq_note ;

//         }
//     }
// };


// void play_sequencer() {

//   static unsigned long noiasca_millis = 0 ;
//   static uint8_t state = 3 ;

//     if (millis() - noiasca_millis >= bpm_in_micro_secnd_format ) {
//         state++ ;
//         state = state % 4 ;

//         switch (state) {
//           case 0:  
//             seq = slot[0][0][0] ; old = 0 ; 
//             play_step(seq, old) ; break ;

//           case 1:  
//             seq = slot[0][0][1]; old = slot[0][0][0];
//             play_step(seq, old); break;

//          case 2:  
//             seq = slot[0][0][2]; old = slot[0][0][1];
//             play_step(seq, old); break;

//           case 3:  
//             seq = slot[0][0][3]; old = slot[0][0][2];
//             play_step(seq, old); break;

//         }
//         noiasca_millis = millis();
//     }

// };



// play AND turn off a note (a drum hit...)
void play_a_sequencer_drum (uint8_t note ) {

  static unsigned long noiasca_millis = 0 ;
  static uint8_t state = 1 ;

    if (millis() - noiasca_millis >= 100 /* 100 ms (?) */ ) {
        state++ ;
        state = state % 2 ;

        switch (state) {
          case 0:  
            send_midi_note_on(note, velocity)
            break ;

          case 1:  
            usbmidi.sendNoteOff(attach_midi_channel_to_note(note), 0);
            break ;

        }
        noiasca_millis = millis();
    }

};

 

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */





/* #region    || — — — — — — — — — — ||              SETUP              || — — — — — — — — — — — || */

void setup() {

  FilteredAnalog<>::setupADC();
  analog16.setupADC();
  analog16.resetToCurrentValue();
  analog16.invert();
  analog17.setupADC();
  analog17.resetToCurrentValue();
  analog17.invert();
  analog15.setupADC();
  analog15.resetToCurrentValue();
  analog15.invert();
  analog14.setupADC();
  analog14.resetToCurrentValue();
  analog14.invert();

  // Initialize the USB Host
  usb.begin();

  // Initialize Control Surface _after_ connecting the interfaces
  Control_Surface.begin();

  // Attach the custom callback
  hstmidi.setCallbacks(callback);

  // Set Launchpad in "programmer mode":
  uint8_t lpx_programmer_mode[9] = { 240, 0, 32, 41, 2, 12, 14, 1, 247 };
  hstmidi.sendSysEx(lpx_programmer_mode);

  // Light the white keys:
  for (uint8_t i = 0; i < 40; i++) { send_led_sysex_to_launchpad(white_keys[i], lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]); }

  // // Light the control change buttons, accordingly to their default state::
  lpx_sysex_light_cc_buttons();

  hstmidi.sendSysEx(lpx_device_inquiry);

}



/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||              LOOP               || — — — — — — — — — — — || */
void loop() {

  // The Control_Surface loop:
  Control_Surface.loop();

  // Check for animations:
  if (animation_in_progress == true) { update_animation(); };

  
  // Potentiometers:
  static Timer<millis> timer = 1; // ms
  if (timer && analog16.update()) { pot_1() };

  if (timer && analog17.update()) { pot_2() };

  if ( timer && analog15.update() || timer && analog14.update() ) { pot_3() };

}
/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */
