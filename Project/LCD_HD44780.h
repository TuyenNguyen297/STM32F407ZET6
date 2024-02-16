#ifndef LCD_HD44780_H
#define LCD_HD44780_H

#define LCD_CLEAR_SCREEN 0x01 // delay_ms 1.53
#define LCD_RETURN_HOME 0x02  // delay_ms 1.53
#define LCD_ENTRY_MODE 0x04 // Increment mode, entire shift off
#define LCD_ENTRY_INCREMENT_MODE 0x02 // Increment mode, entire shift off
#define LCD_ENTRY_SHIFT_OFF 0x00

#define LCD_DISPLAY_MODE 0x08
#define LCD_DISPLAY_OFF 0x00
#define LCD_DISPLAY_ON 0x04
#define LCD_CURSOR_ON 0x02
#define LCD_CURSOR_OFF 0x00
#define LCD_CURSOR_BLINKON 0x01
#define LCD_CURSOR_BLINKOFF 0x00

#define LCD_MOVE_CURSOR_LEFT 0x10
#define LCD_MOVE_CURSOR_RIGHT 0x14
#define LCD_SHIFT_DISPLAY_LEFT 0x18
#define LCD_SHIFT_DISPLAY_RIGHT 0x1C
#define LCD_DDRAM_SELECT 0x80
#define LCD_CGRAM_SELECT 0x40

#define LCD_BUS_4 0x20
#define LCD_BUS_8 0x30
#define LCD_BUS LCD_BUS_4

#if LCD_BUS == LCD_BUS_4 // bus 4
#define  LCD_BUS_LENGTH LCD_BUS_4
#else // bus 8
#define  LCD_BUS_LENGTH LCD_BUS_8
#endif

#define LCD_1_LINE 0x00 // 1 line
#define LCD_SMALL_FONT 0x00
#define LCD_2_LINES 0x08 // 2 lines with small font size
#define LCD_BIG_FONT 0x04

#define MAX_PAGE 5
#define MIN_PAGE 1

extern volatile uchar page;


void LCD_Latch(char*);
void LCD_PutData(uchar);
void LCD_Command(uchar);
void LCD_Print(char*);
void LCD_SetCursor(uchar, uchar);
void LCD_Shift(DIRECTION);
void LCD_Init();
void LCD_Clear();
void LCD_Clear_Row_From_Col(uchar, uchar);
void LCD_SetEntry(uchar);
void LCD_SetDisplay(uchar);
void LCD_SetFunction(uchar, DELAY_TYPE, double);
void LCD_Print_2D_String(char (*)[][18], uint16_t, uchar (*)[][2]);
void LCD_Display(uchar);

#endif
