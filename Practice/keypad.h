#ifndef KEYPAD_H
#define KEYPAD_H

extern volatile Bool readkey_done, all_row_output_low, key_unread;
extern volatile uint16_t RowPressed, ColPressed, blink_interval_ms;
extern char pad;

char Keypad_Scan();
void React_On_KeyPressed(char *);

#endif
