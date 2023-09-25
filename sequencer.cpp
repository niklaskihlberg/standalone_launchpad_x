


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

 