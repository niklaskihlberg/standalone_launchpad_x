/* #region    || — — — — — — — — — — ||            INCLUDES             || — — — — — — — — — — — || */

#include <Control_Surface.h>
#include <MIDI_Interfaces/USBHostMIDI_Interface.hpp>
#include <string>

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||         GLOBAL DEFINES          || — — — — — — — — — — — || */

/// Create ports for USB devices plugged into Teensy's 2nd USB port (via hubs):
USBHost usb;
USBHub hub{usb};

/// Create MIDI interfaces:
USBDebugMIDI_Interface dbgmidi = 115200;   // Serial Debug
USBMIDI_Interface usbmidi;                 // Device MIDI
USBHostMIDI_Interface hstmidi{usb};        // USB Host MIDI

/// Constants:
uint8_t cc_buttons[16]         = { 91, 92, 93, 94, 96, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
uint8_t lpx_color_white[3]     = { 89, 100, 127 };
uint8_t every_pad[64]          = { 11, 12, 13, 14, 15, 16, 17, 18, 21, 22, 23, 24, 25, 26, 27, 28, 31, 32, 33, 34, 35, 36, 37, 38, 41, 42, 43, 44, 45, 46, 47, 48, 51, 52, 53, 54, 55, 56, 57, 58, 61, 62, 63, 64, 65, 66, 67, 68, 71, 72, 73, 74, 75, 76, 77, 78, 81, 82, 83, 84, 85, 86, 87, 88 };

/// Variables:
uint8_t white_keys[40]     = { 11, 12, 14, 16, 18, 21, 23, 24, 26, 28, 31, 33, 34, 36, 38, 41, 43, 45, 46, 48, 51, 53, 55, 56, 58, 61, 63, 65, 67, 68, 72, 73, 75, 77, 78, 82, 83, 85, 87,255 };
uint8_t new_white_keys[40] = { 11, 12, 14, 16, 17, 21, 22, 24, 26, 28, 31, 33, 34, 36, 38, 41, 43, 44, 46, 48, 51, 53, 55, 56, 58, 61, 63, 65, 66, 68, 71, 73, 75, 77, 78, 82, 83, 85, 87,255 };
uint8_t new_black_pads[32] = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };

/// Pool
uint8_t pad_pool[16]               = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t note_pool[16]              = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t velocity_pool[16]          = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t pad_transposed[16]         = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
 
int octave_shift_amount_selector   = 3;
int octave_shift[7]                = { -36, -24, -12, 0, 12, 24, 36 };

uint8_t key_transpose_selector_pad = 0;
int key_transpose                  = 0;

int pad_layout_shift               = 4;
String lowest_note                 = "E";

/// Color button:
bool change_color_on_button_pressed = false;
int color_on_selector_value = 0;
uint8_t color_on_selector_pad = 71;

/// Transpose buttons:
bool left_arrow_pressed           = false;
bool right_arrow_pressed          = false;
bool up_arrow_pressed             = false;
bool down_arrow_pressed           = false;
bool key_transpose_button_pressed = false;

/// Timeing:
unsigned long myTime_1;
unsigned long myTime_2;

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||            FUNCTIONS            || — — — — — — — — — — — || */

void send_midi_led_sysex_to_launchpad(uint8_t pad, uint8_t r, uint8_t g, uint8_t b) {
  
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

void key_transpose_screen() {

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

void replace_old_midi_note_in_pool_with_new(uint8_t pad, uint8_t note, uint8_t velocity) {

  // ...send midi off first...
  usbmidi.sendNoteOff(note, 0);

  // ...then midi on, to get the new velocity sent...
  usbmidi.sendNoteOn(note, velocity);

  // ...set the new color for the LED to match the velocity...
  send_midi_led_sysex_to_launchpad(pad, note_on_color_r(velocity), note_on_color_g(velocity), note_on_color_b(velocity));

}

uint8_t note_on_color_r(uint8_t velocity) { // Calculate red-   color-value from velocity: 
  uint8_t R = 127;
  if (color_on_selector_value == 0) { R = 127 * 0.75 - velocity; }
  if (color_on_selector_value == 1) { R = 127 - velocity * 0.5; }
  if (color_on_selector_value == 2) { R = 127 - velocity * 0.5; }

  return R;
};

uint8_t note_on_color_g(uint8_t velocity) { // Calculate green- color-value from velocity: 
  uint8_t G = 127;
  if (color_on_selector_value == 0) { G = 127 - velocity * 0.5; }
  if (color_on_selector_value == 1) { G = 127 * 0.75 - velocity; }
  if (color_on_selector_value == 2) { G = 127 - velocity * 0.5; }
  return G;
};

uint8_t note_on_color_b(uint8_t velocity) { // Calculate blue-  color-value from velocity: 
  uint8_t B = 127;
  if (color_on_selector_value == 0) { B = 127 - velocity * 0.5; }
  if (color_on_selector_value == 1) { B = 127 * 0.75 - velocity; }
  if (color_on_selector_value == 2) { B = 127 * 0.75 - velocity; }
  return B;
};

void change_color_on_screen() {

  uint8_t change_color_on_screen_off_pads[24] = { 81,82,83,84,85,86,87,88,71,72,73,74,75,76,77,78,61,62,63,64,65,66,67,68 };
  for (int i = 0; i < 24; i++) {
    uint8_t lpx_sysex_key_transpose_screen_off_pads[] = { 240, 0, 32, 41, 2, 12, 3, 0, change_color_on_screen_off_pads[i], 0, 247 };
    hstmidi.sendSysEx(lpx_sysex_key_transpose_screen_off_pads);
    }

  uint8_t lpx_sysex_change_color_on_screen_button_1[] = { 240, 0, 32, 41, 2, 12, 3, 3, 72, 0, 16, 16, 247 };
  hstmidi.sendSysEx(lpx_sysex_change_color_on_screen_button_1);

  uint8_t lpx_sysex_change_color_on_screen_button_2[] = { 240, 0, 32, 41, 2, 12, 3, 3, 73, 16, 0, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_change_color_on_screen_button_2);

  uint8_t lpx_sysex_change_color_on_screen_button_3[] = { 240, 0, 32, 41, 2, 12, 3, 3, 74, 16, 16, 0, 247 };
  hstmidi.sendSysEx(lpx_sysex_change_color_on_screen_button_3);

  if (color_on_selector_value == 0) { color_on_selector_pad = 72; }
  if (color_on_selector_value == 1) { color_on_selector_pad = 73; }
  if (color_on_selector_value == 2) { color_on_selector_pad = 74; }

  uint8_t lpx_sysex_color_on_screen_selector_pad[] = { 240, 0, 32, 41, 2, 12, 3, 3, color_on_selector_pad, note_on_color_r(127), note_on_color_g(127), note_on_color_b(127), 247 };
  hstmidi.sendSysEx(lpx_sysex_color_on_screen_selector_pad);

};

void print_pool() {
  
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
  Serial << " * * *    " << "Transpose (" << lowest_note << ") - 1 = " << endl;
  Serial << " * * * " << endl;
  Serial << " * * *    Arrow released..." << endl;
  Serial << " * * * " << endl;
  Serial << " * * *    " << "New layout is set to: " << pad_layout_shift << " with " << lowest_note << " as the lowest note." << endl;
  Serial << " * * * " << endl;
  Serial << " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " << endl;
  print_pool();
};

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

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||        PAD LAYOUTS ARRAYS       || — — — — — — — — — — — || */

void white_key_layouts() {

  if (pad_layout_shift == 0 || pad_layout_shift == 12 || pad_layout_shift == 24 || pad_layout_shift == -12 || pad_layout_shift == -24 ) {
    lowest_note = "C";
    uint8_t new_white_keys[] = { 11, 13, 15, 16, 18, 21, 23, 25, 27, 28, 32, 33, 35, 37, 38, 42, 43, 45, 47, 52, 54, 55, 57, 62, 64, 65, 67, 72, 74, 76, 77, 81, 82, 84, 86, 87,255,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -1 || pad_layout_shift == 11 || pad_layout_shift == 23 || pad_layout_shift == -13 || pad_layout_shift == -25 ) {
    lowest_note = "B";
    uint8_t new_white_keys[] = { 11, 12, 14, 16, 17, 21, 22, 24, 26, 28, 31, 33, 34, 36, 38, 41, 43, 44, 46, 48, 51, 53, 55, 56, 58, 61, 63, 65, 66, 68, 71, 73, 75, 77, 78, 82, 83, 85, 87, 88 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -2 || pad_layout_shift == 10 || pad_layout_shift == 22 || pad_layout_shift == -14 || pad_layout_shift == -26 ) {
    lowest_note = "Bb / A#";
    uint8_t new_white_keys[] = { 12, 13, 15, 17, 18, 22, 23, 25, 27, 32, 34, 35, 37, 42, 44, 45, 47, 52, 54, 56, 57, 61, 62, 64, 66, 67, 71, 72, 74, 76, 78, 81, 83, 84, 86, 88,255,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -3 || pad_layout_shift == 9 || pad_layout_shift == 21 || pad_layout_shift == -15 || pad_layout_shift == -27 ) {
    lowest_note = "A";
    uint8_t new_white_keys[] = { 11, 13, 14, 16, 18, 21, 23, 24, 26, 28, 31, 33, 35, 36, 38, 41, 43, 45, 46, 48, 51, 53, 55, 57, 58, 62, 63, 65, 67, 68, 72, 73, 75, 77, 82, 84, 85, 87,255,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -4 || pad_layout_shift == 8 || pad_layout_shift == 20 || pad_layout_shift == -16 || pad_layout_shift == -28 ) {
    lowest_note = "Ab / G#";
    uint8_t new_white_keys[] = { 12, 14, 15, 17, 22, 24, 25, 27, 32, 34, 36, 37, 41, 42, 44, 46, 47, 51, 52, 54, 56, 58, 61, 63, 64, 66, 68, 71, 73, 74, 76, 78, 81, 83, 85, 86, 88,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -5 || pad_layout_shift == 7 || pad_layout_shift == 19 || pad_layout_shift == -17 || pad_layout_shift == -29 ) {
    lowest_note = "G";
    uint8_t new_white_keys[] = { 11, 13, 15, 16, 18, 21, 23, 25, 26, 28, 31, 33, 35, 37, 38, 42, 43, 45, 47, 48, 52, 53, 55, 57, 62, 64, 65, 67, 72, 74, 75, 77, 82, 84, 86, 87,255,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -6 || pad_layout_shift == 6 || pad_layout_shift == 18 || pad_layout_shift == -18 || pad_layout_shift == -30 ) {
    lowest_note = "Gb / F#";
    uint8_t new_white_keys[] = { 12, 14, 16, 17, 21, 22, 24, 26, 27, 31, 32, 34, 36, 38, 41, 43, 44, 46, 48, 51, 53, 54, 56, 58, 61, 63, 65, 66, 68, 71, 73, 75, 76, 78, 81, 83, 85, 87, 88,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -7 || pad_layout_shift == 5 || pad_layout_shift == 17 || pad_layout_shift == -19 || pad_layout_shift == -31 ) {
    lowest_note = "F";
    uint8_t new_white_keys[] = { 11, 13, 15, 17, 18, 22, 23, 25, 27, 28, 32, 33, 35, 37, 42, 44, 45, 47, 52, 54, 55, 57, 62, 64, 6, 67, 71, 72, 74, 76, 77, 81, 82, 84, 86, 88,255,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -8 || pad_layout_shift == 4 || pad_layout_shift == 16 || pad_layout_shift == -20 || pad_layout_shift == -32 ) {
    lowest_note = "E";
    uint8_t new_white_keys[] = { 11, 12, 14, 16, 18, 21, 23, 24, 26, 28, 31, 33, 34, 36, 38, 41, 43, 45, 46, 48, 51, 53, 55, 56, 58, 61, 63, 65, 67, 68, 72, 73, 75, 77, 78, 82, 83, 85, 87,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -9 || pad_layout_shift == 3 || pad_layout_shift == 15 || pad_layout_shift == -21 || pad_layout_shift == -33 ) {
    lowest_note = "Eb / D#";
    uint8_t new_white_keys[] = { 12, 13, 15, 17, 22, 24, 25, 27, 32, 34, 35, 37, 42, 44, 46, 47, 51, 52, 54, 56, 57, 61, 62, 64, 66, 68, 71, 73, 74, 76, 78, 81, 83, 84, 86, 88,255,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -10 || pad_layout_shift == 2 || pad_layout_shift == 14 || pad_layout_shift == -22 || pad_layout_shift == -34 ) {
    lowest_note = "D";
    uint8_t new_white_keys[] = { 11, 13, 14, 16, 18, 21, 23, 25, 26, 28, 31, 33, 35, 36, 38, 41, 43, 45, 47, 48, 52, 53, 55, 57, 58, 62, 63, 65, 67, 72, 74, 75, 77, 82, 84, 85, 87,255,255,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };

  if (pad_layout_shift == -11 || pad_layout_shift == 1 || pad_layout_shift == 13 || pad_layout_shift == -23 || pad_layout_shift == -35 ) {
    lowest_note = "Db / C#";
    uint8_t new_white_keys[] = { 12, 14, 15, 17, 22, 24, 26, 27, 31, 32, 34, 36, 37, 41, 42, 44, 46, 48, 51, 53, 54, 56, 58, 61, 63, 64, 66, 68, 71, 73, 75, 76, 78, 81, 83, 85, 86, 88,255,255 };
    for (uint8_t i = 0; i < 40; i++) white_keys[i] = new_white_keys[i];
  };
};

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||        MIDI NOTE HANDLING       || — — — — — — — — — — — || */

void midi_note_processing(uint8_t pad, uint8_t velocity) {

  Serial << " • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • " << endl << endl;

  if (change_color_on_button_pressed == true) {
    if (pad == 72) { color_on_selector_value = 0; }
    if (pad == 73) { color_on_selector_value = 1; }
    if (pad == 74) { color_on_selector_value = 2; }
    change_color_on_screen();
    }

  if (key_transpose_button_pressed == true){
    if (pad == 71) { key_transpose = 0;  }
    if (pad == 82) { key_transpose = 1;  }
    if (pad == 72) { key_transpose = 2;  }
    if (pad == 83) { key_transpose = 3;  }
    if (pad == 73) { key_transpose = 4;  }
    if (pad == 74) { key_transpose = 5;  }
    if (pad == 85) { key_transpose = 6;  }
    if (pad == 75) { key_transpose = 7;  }
    if (pad == 86) { key_transpose = 8;  }
    if (pad == 76) { key_transpose = 9;  }
    if (pad == 87) { key_transpose = 10; }
    if (pad == 77) { key_transpose = 11; }
    if (pad == 78) { key_transpose = 12; }
    key_transpose_screen();
  }

  // only refresh the visible pads, don’t ”color over” any screen:
  if (key_transpose_button_pressed == false || pad < 59) {

    // Check what note to process when pad is pressed:
    uint8_t note = pad_to_midi_processing_table(pad) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose;
    
    bool note_in_midi_range = true;
    
    if (pad_to_midi_processing_table(pad) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose < 0 || pad_to_midi_processing_table(pad) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose > 127) {
      note = 0;
      note_in_midi_range = false;
    }

    Serial << "NOTE EVENT: " << note << endl;
    Serial << "LAYOUT POSITION: " << lowest_note;
    Serial << "   PAD: " << pad << "   VELOCITY: " << velocity << "   OCTAVE: " << octave_shift[octave_shift_amount_selector] << endl;

    // Keep track of pad pals old positions...
    uint8_t pool_position_where_the_old_note_is = 0;

    // If the new note has velocity:
    if (note_in_midi_range == true && velocity != 0) {

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
            usbmidi.sendNoteOn(note, velocity);
            send_midi_led_sysex_to_launchpad(pad, note_on_color_r(velocity), note_on_color_g(velocity), note_on_color_b(velocity));

//            // And send sysex to the launchpad to light the pad
//            Serial << "LED: " << pad << " - ON";
//             uint8_t lpx_led_on_w_velocity[] = { 240, 0, 32, 41, 2, 12, 3, 3, pad, note_on_color_r(velocity), note_on_color_g(velocity), note_on_color_b(velocity), 247 };
//             hstmidi.sendSysEx(lpx_led_on_w_velocity);
// 
//             // Check if the pad has a pad_pal. If there is a pad_pal send the sysex for the pal aswell!
//             uint8_t pal = pad_pals(pad);
//             if (pal != 0) {
//               Serial << " & PAL: " << pal << " - ON" << endl;
//               uint8_t lpx_led_on_w_velocity[] = { 240, 0, 32, 41, 2, 12, 3, 3, pal, note_on_color_r(velocity), note_on_color_g(velocity), note_on_color_b(velocity), 247 };
//               hstmidi.sendSysEx(lpx_led_on_w_velocity);
//             } else Serial << endl;
          }

          // Add the note to the array (Even if its already there... We need both!)
          pad_pool[i] = pad;
          note_pool[i] = note;
          velocity_pool[i] = velocity;

          break; // We don't need the note to fill more then one pool slot...
        }
      }
    }

    /* #region    || — — — — — — — — — — ||            VELOCITY 0           || — — — — — — — — — — — || */

    // If the velocity recieved is 0
    if (velocity == 0) {

      // If the note is in the pool...
      for (uint8_t i = 0; i < 16; i++) {
        if (pad == pad_pool[i]) {

          uint8_t midi_off_note = note_pool[i];

          // If the pad has been shifted, correct it accordingly:
          if (pad_transposed[i] != 0){
            pad = pad_transposed[i];
          }

          // Remove the note out of the pool. Empty the pool-slots:
          pad_pool[i]       = 0;
          note_pool[i]      = 0;
          velocity_pool[i]  = 0;
          pad_transposed[i] = 0;
          
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

            usbmidi.sendNoteOff(midi_off_note, velocity);

            Serial << "   PAD PAL IN POOL: " << "False";
            Serial << " || Send midi note off (" << midi_off_note << ")" << endl;

            // Send Sysex to Launchpad to light the pad
            // Check if the pad is white:
            bool pad_is_white = false;
            for (uint8_t i = 0; i < 40; i++) {

              if (pad == white_keys[i]) {
                send_midi_led_sysex_to_launchpad(pad, lpx_color_white[0], lpx_color_white[1], lpx_color_white[2]);
                pad_is_white = true;
                break; // There shouldn't be more then one match...
              } else send_midi_led_sysex_to_launchpad(pad, 0, 0, 0); // all 40 white pads are checked, the pad is black.
            }
          }
          break; // If we've already found one matching note, their no need to find another
        }
      }
    }

    /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

    print_pool();
    Serial << " • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • • " << endl;
  }
  };

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||         MIDI CC HANDLING        || — — — — — — — — — — — || */

void control_change_processing(uint8_t controller, uint8_t value) {

  bool refresh_pads = false;

  // — — — — — — — — — — // KEY TRANSPOSE SCREEN:
  if (controller == 96) {
    if (value == 127) {
      key_transpose_screen();
      key_transpose_button_pressed = true;
      } else {
      key_transpose_button_pressed = false;
      refresh_pads = true;
      }
    }

 // — — — — — — — — — — // COLOR ON SCREEN
  if (key_transpose_button_pressed == true) {
    if (controller == 89){
      if (value == 127){
        change_color_on_screen();
        change_color_on_button_pressed = true;
      } else {
        change_color_on_button_pressed = false;
        refresh_pads = true;
      }
    }
  }

  // — — — — — — — — — — // COLOR TEST SCREEN:
   if (key_transpose_button_pressed == true) {
    if (controller == 19){
      if (value == 127){
        color_test_screen();
        if (color_test_button_pressed == false) color_test_button_pressed = true;
        if (color_test_button_pressed == true ) color_test_button_pressed = false;
        refresh_pads = true;
      } else {
        // do nothing, except refresh pads on release.
        refresh_pads = true;
      }
    }
  }

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

  /* #region    || — — — — — — — — — — ||              LAYOUT             || — — — — — — — — — — — || */


  int old_layout_shift = pad_layout_shift;

  // — — — — — — — — — — // LAYOUT SHIFT BUTTONS:
  if (controller == 93) {
    if (value == 127) {
      left_arrow_pressed = true;
      if (right_arrow_pressed == true) {
        pad_layout_shift = 4;
      } else pad_layout_shift += 1;
      refresh_pads = true;
      Serial << " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " << endl << " * * * " << endl;
    }
    if (value == 0) {
      left_arrow_pressed = false;
      print_left_or_right_arrow_released();
    }
  }

  if (controller == 94) {
    if (value == 127) {
      right_arrow_pressed = true;
      if (left_arrow_pressed == true) {
        pad_layout_shift = 4;
      } else pad_layout_shift -= 1;
      refresh_pads = true;
      Serial << " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " << endl << " * * * " << endl;
    }
    if (value == 0) {
      right_arrow_pressed = false;
      print_left_or_right_arrow_released();
    }
  }

  // Layout shift wrap-around:
  if (pad_layout_shift > 24) pad_layout_shift = -35;
  if (pad_layout_shift < -35) pad_layout_shift = 24;

  // Get / Set the current layout for the white keys:
  white_key_layouts();

  // Change the layout_difference::
  int layout_difference = pad_layout_shift - old_layout_shift;

  // — — — — — — — — — — // COLOR PADS ACCORDING TO THE CURRENT LAYOUT:
  if (refresh_pads == true) {

    // Are there any pads in the pool? If so, correct them to the new layout:
    for (uint8_t i = 0; i < 16; i++) {
      if (pad_pool[i] != 0) {
        // Change the value of the pals in the pool so they will react to any pads being released
        if (pad_transposed[i] == 0) pad_transposed[i] = pad_pool[i] +- layout_difference;
        else pad_transposed[i] = pad_transposed[i] +- layout_difference;
        
        /* #region    || — — — — — — — — — — ||    PAD_TRANSPOSED ROW-WRAP      || — — — — — — — — — — — || */

        if (pad_transposed[i] == 100) { pad_transposed[i] = 88;  }
        if (pad_transposed[i] == 80)  { pad_transposed[i] = 75;  }
        if (pad_transposed[i] == 70)  { pad_transposed[i] = 65;  }
        if (pad_transposed[i] == 60)  { pad_transposed[i] = 55;  }
        if (pad_transposed[i] == 50)  { pad_transposed[i] = 45;  }
        if (pad_transposed[i] == 40)  { pad_transposed[i] = 35;  }
        if (pad_transposed[i] == 30)  { pad_transposed[i] = 25;  }
        if (pad_transposed[i] == 20)  { pad_transposed[i] = 15;  }

        if (pad_transposed[i] == 89)  { pad_transposed[i] = 101; }
        if (pad_transposed[i] == 79)  { pad_transposed[i] = 84;  }
        if (pad_transposed[i] == 69)  { pad_transposed[i] = 74;  }
        if (pad_transposed[i] == 59)  { pad_transposed[i] = 64;  }
        if (pad_transposed[i] == 49)  { pad_transposed[i] = 54;  }
        if (pad_transposed[i] == 39)  { pad_transposed[i] = 44;  }
        if (pad_transposed[i] == 29)  { pad_transposed[i] = 34;  }
        if (pad_transposed[i] == 19)  { pad_transposed[i] = 24;  }

        /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

        // Color the "new" on-pads:
        send_midi_led_sysex_to_launchpad(pad_transposed[i], note_on_color_r(velocity_pool[i]), note_on_color_g(velocity_pool[i]), note_on_color_b(velocity_pool[i]));
        
      }
    }
    
    // If the "note transpose screen" is active, don't "color over" that the screen...
    int pads_to_check = 64;
    if (key_transpose_button_pressed == true) pads_to_check = 40;

    // Color the pads after the new layout // Go through all of the 64 pads... (or 40 pads if any 3-row-”screen” is open...)
    for (uint8_t i = 0; i < pads_to_check; i++) {

      // Check if the note is "out of bounds" (If the note is not within midi range):
      bool note_in_midi_range = true;
      if (pad_to_midi_processing_table(every_pad[i]) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose < 0
        || pad_to_midi_processing_table(every_pad[i]) + pad_layout_shift + octave_shift[octave_shift_amount_selector] + key_transpose > 127) {
        note_in_midi_range = false;
      }

      // Check if any of them are in one of the 16 note pool slots •
      bool pad_in_pool = false;
  
      // Check if any of them are in one of the 16 note pool slots •
      for (uint8_t j = 0; j < 16; j++) {
        if (every_pad[i] == pad_transposed[j]) {
          pad_in_pool = true;
          break; // There can be only one...
        }
      }

      // Check if any of them is a pad pal...
      for (uint8_t j = 0; j < 16; j++) {
        if (every_pad[i] == pad_pals(pad_transposed[j])) {
          pad_in_pool = true;
          break; // We only need one match...
          }
        }

      // If the pad is not in the pool, and it is in midi range, go ahead and color:
      if (pad_in_pool == false && note_in_midi_range == true) {

        // Check if it is a white or black key, if it is one of the 40 white keys
        bool pad_is_white = false;
        for (uint8_t j = 0; j < 40; j++) {
          if (every_pad[i] == white_keys[j]) {
            pad_is_white = true;
            break; // No need to check for more then one match:
          }
        }

        if (pad_is_white == true) {

          // ? //
          // ? // Maybe we don't need to color EVERY 64 pad, we're already checking for pals in the coloring funcion.
          // ? //

          // Color the white pads white: 
          uint8_t white[] = { 240, 0, 32, 41, 2, 12, 3, 3, every_pad[i], lpx_color_white[0], lpx_color_white[1], lpx_color_white[2], 247 };
          hstmidi.sendSysEx(white);
        } else {
          // Color the black pads black
          uint8_t sysex[] = { 240, 0, 32, 41, 2, 12, 3, 3, every_pad[i], 0, 0, 0, 247 };
          hstmidi.sendSysEx(sysex);
          // Save all the black pads in their array, for printing later...
          for (uint8_t k = 0; k < 32; k++) {
            if (new_black_pads[k] == 0) {
              new_black_pads[k] = every_pad[i];
              break; // Each pad only needs to one slot...
            }
          }
        }
      }

      // If the note is "out of bounds" (not in midi range), color it with a "warning color":
      if (note_in_midi_range == false) {
        send_midi_led_sysex_to_launchpad(every_pad[i], 8, 0, 0);
      }
    }

    /* #region    || — — — — — — — — — — ||         PRINT TRANSPOSE         || — — — — — — — — — — — || */

    // Print the new pad-layout-info:
    Serial << " * * *    New white keys: ";
    for (uint8_t i = 0; i < 40; i++) { if (white_keys[i] < 90) { Serial << white_keys[i] << ", " << endl; } }

    Serial << " * * *    New black keys: ";
    for (uint8_t i = 0; i < 32; i++) { if (new_black_pads[i] != 0) { Serial << new_black_pads[i] << ", " << endl; } }

    Serial << " * * *    Pads on:        ";
    for (uint8_t i = 0; i < 16; i++) { if (pad_transposed[i] != 0) { Serial << pad_transposed[i] << ", " << pad_pals(pad_transposed[i]) << ", " << endl; } }

    /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */
  }
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
    for (uint8_t i = 0; i < 16; i++) {
      if (controller == cc_buttons[i]) {
        if (value != 0) {
          /// Light up the pushed pad:
          uint8_t lpx_cc_led_white[] = {240, 0, 32, 41, 2, 12, 3, 3, controller, 127, 127, 127, 247};
          hstmidi.sendSysEx(lpx_cc_led_white);
        } else {
          // If value is 0:
          uint8_t lpx_led_cc_buttons[] = { 240, 0, 32, 41, 2, 12, 3, 3, controller, 2, 2, 2, 247 };
          hstmidi.sendSysEx(lpx_led_cc_buttons);
        } 
      }
    }

    control_change_processing(controller, value);

    /// Clock and print the timer:
    myTime_2 = micros() - myTime_1;
    Serial << "Time: " << myTime_2 / 1000 << " ms   " << myTime_2 << " micro" << endl;
  };

  /// Pitchbend:
  //   void onPitchBend(Channel channel, uint16_t bend, Cable cable) {
  //   }

  /* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

} callback; 


/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||              SETUP              || — — — — — — — — — — — || */

void setup() {

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
  for (uint8_t i = 0; i < 40; i++) {
    uint8_t lpx_pads_start_setup[] = { 240, 0, 32, 41, 2, 12, 3, 3, white_keys[i], lpx_color_white[0], lpx_color_white[1], lpx_color_white[2], 247 };
    hstmidi.sendSysEx(lpx_pads_start_setup);
  }

  // Light the control change buttons:
  for (uint8_t i = 0; i < 5; i++) {
    uint8_t lpx_led_cc_buttons[] = { 240, 0, 32, 41, 2, 12, 3, 3, cc_buttons[i], 2, 2, 2, 247 };
    hstmidi.sendSysEx(lpx_led_cc_buttons);
  }

}

/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */

/* #region    || — — — — — — — — — — ||              LOOP               || — — — — — — — — — — — || */
void loop() {
  Control_Surface.loop();
}
/* #endregion || — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — — || */
