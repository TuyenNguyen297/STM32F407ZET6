#ifndef RTC_H
#define RTC_H
#define RTC_CLOCK_BY_LSE 0x00000100
#define RTC_PRESCALER_RESET 0x007F00FF

typedef struct __attribute__((__packed__))
{
	uchar weekday;
	uchar day;
	uchar month;
	uint16_t year;
	uchar hour;
	uchar minute;
	uchar second;
} DateTime;

extern DateTime rtc_dt;

void RTC_Init();
void Update_RTC_Clock(DateTime*);
void Display_RTC_Clock(DateTime*, Bool);

#endif
