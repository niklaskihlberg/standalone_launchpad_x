


uint8_t sixteenth_note[32] = { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }

uint8_t quarter_note[8] = { 0, 0, 0, 0,  0, 0, 0, 0, }

uint8_t slot[2][4][4] = {
    { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
    { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
};



// Sequencer data
typedef struct sequencer_step_data {
  bool rest;
  uint8_t note;
  bool accent;
};

sequencer_step_data sequencer[2][4][4] = {
    { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
    { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
};




void play_step(uint8_t seq_note, uint8_t old_seq_note){

    usbmidi.sendNoteOff(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
    dinmidi.sendNoteOff(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;

    usbmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
    dinmidi.sendNoteOn(attach_midi_channel_to_note(pad_to_midi_processing_table_drum_edition(pad)), velocity) ;
            
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


void play_sequencer() {

  static unsigned long noiasca_millis = 0 ;
  static uint8_t state = 3 ;

    if (millis() - noiasca_millis >= bpm_in_micro_secnd_format ) {
        state++ ;
        state = state % 4 ;

        switch (state) {
          case 0:  
            seq = slot[0][0][0] ; old = 0 ; 
            play_step(seq, old) ; break ;

          case 1:  
            seq = slot[0][0][1]; old = slot[0][0][0];
            play_step(seq, old); break;

         case 2:  
            seq = slot[0][0][2]; old = slot[0][0][1];
            play_step(seq, old); break;

          case 3:  
            seq = slot[0][0][3]; old = slot[0][0][2];
            play_step(seq, old); break;

        }
        noiasca_millis = millis();
    }

};



// play AND turn off a note (a drum hit...)
void play_a_sequencer_drum() {

  static unsigned long noiasca_millis = 0 ;
  static uint8_t state = 1 ;

    if (millis() - noiasca_millis >= 100 /* 100 ms (?) */ ) {
        state++ ;
        state = state % 2 ;

        switch (state) {
          case 0:  
            send_midi_on( slot[0][0][0] ) ;
            break ;

          case 1:  
            send_midi_off( slot[0][0][0] ) ;
            break ;

        }
        noiasca_millis = millis();
    }

};

 