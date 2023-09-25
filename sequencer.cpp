


uint8_t sixteenth_note[32] = { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }

uint8_t quarter_note[8] = { 0, 0, 0, 0,  0, 0, 0, 0, }

uint8_t slot[2][4][4] = {
    { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
    { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
};


for (uint8_t bar = 0      ; bar < 1      ; bar++      ) {
    for (uint8_t beat = 0     ; beat < 3     ; beat++     ) {
        for (uint8_t sixteenth = 0; sixteenth < 3; sixteenth++) { 
            
            usbmidi.sendNoteOff(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
            dinmidi.sendNoteOff(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
            
            uint8_t seq_note = slot[bar][beat][sixteenth] ;
            usbmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
            dinmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
            
            uint8_t prev_seq_note = seq_note ;

        }
    }
};

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
      

 