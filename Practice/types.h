#ifndef TYPES_H
#define TYPES_H

typedef unsigned char uchar;

#define HSI 0
#define HSE 1
#define PLL 2

#define MAX_PIN_CHAR 4
#define KPAD_ROW 4
#define KPAD_COL 4

//#define CLK_SRC HSI
//#define CLK_SRC HSE
#define CLK_SRC PLL

#if CLK_SRC == HSI
#define SYSCLK 16000000UL
#elif CLK_SRC == HSE
#define SYSCLK 8000000UL
#elif CLK_SRC == PLL
#define SYSCLK 168000000UL
#else
#define SYSCLK 8000000UL
#endif

#ifdef SYSCLK
#define AHBCLK SYSCLK / 1
#define HCLK AHBCLK
#define FCLK AHBCLK
#define SYSTICK_CLK AHBCLK / 8
#define APB1CLK AHBCLK / 4
#define APB2CLK AHBCLK / 2
#define APB1TIMCLK APB1CLK * 2
#define APB2TIMCLK APB2CLK * 2
#define CK_PSC_1 APB1TIMCLK
#define CK_PSC_2 APB2TIMCLK
#endif

#define TIMER_1 1
#define TIMER_2 2
#define TIMER_3 3
#define TIMER_4 4
#define TIMER_5 5
#define TIMER_6 6
#define TIMER_7 7
#define TIMER_8 8
#define TIMER_9 9
#define TIMER_10 10
#define TIMER_11 11
#define TIMER_12 12
#define TIMER_13 13
#define TIMER_14 14

#define TIM_CHN_1 1
#define TIM_CHN_2 2
#define TIM_CHN_3 3
#define TIM_CHN_4 4

/*************Timers 1/2/3/4-6/7--10/11--- are already used ******************/
#define ICM_TEST_TIMER TIMER_1 // Current Input Capture mode testing TIMER = TIMER_1
#define ADC_TRIG_TIMER TIMER_2
#define STEPPER_TIMER TIMER_3  // Microstepping by PWM
#define PWM_TEST_TIMER TIMER_4 // Current PWM mode testing TIMER = TIMER_4
#define KPAD_TIMER TIMER_6
#define SYSTEM_TIMER TIMER_7
#define Ultrasonic_Trig_TIM TIMER_10
#define Ultrasonic_Echo_TIM TIMER_11

#define ADC_CHN_0 0
#define ADC_CHN_1 1
#define ADC_CHN_2 2
#define ADC_CHN_3 3
#define ADC_CHN_4 4
#define ADC_CHN_5 5
#define ADC_CHN_6 6
#define ADC_CHN_7 7
#define ADC_CHN_8 8
#define ADC_CHN_9 9
#define ADC_CHN_10 10
#define ADC_CHN_11 11
#define ADC_CHN_12 12
#define ADC_CHN_13 13
#define ADC_CHN_14 14
#define ADC_CHN_15 15
#define ADC_CHN_16 16
#define ADC_CHN_17 17
#define ADC_CHN_18 18
#define ADC_INTERNAL_TEMP ADC_CHN_16
#define ADC_INTERNAL_VREF ADC_CHN_17
#define ADC_INTERNAL_VBAT ADC_CHN_18

#define BASIC_TIM_PRI 255 // TIM6,7
#define BTN_K0_PRI 10
#define BTN_K1_PRI 10
#define KPAD_C0_PRI 10
#define KPAD_C1_PRI 10
#define KPAD_C2_PRI 10
#define KPAD_C3_PRI 10

#define ROTARY_A_PRI 5
#define ROTARY_SW_PRI 5

typedef enum
{
	US, MS
} DELAY_TYPE;

typedef enum
{
	UP, DOWN, LEFT, RIGHT
} DIRECTION;

typedef enum
{
	INPUT, OUTPUT, AF, ANALOG
} pinMode;

typedef enum
{
	LOW_SPEED, MEDIUM_SPEED, HIGH_SPEED, VERY_HIGH_SPEED
} OUTPUT_SPEED;

typedef enum
{
	ULTRA_FAST = 0x00, FAST = 0x01, NORMAL = 0x02, SLOW = 0x03
} ADC_SPEED;

typedef enum
{
	WEAK, STRONG
} PULL_UP;

typedef enum
{
	AF0,
	AF1,
	AF2,
	AF3,
	AF4,
	AF5,
	AF6,
	AF7,
	AF8,
	AF9,
	AF10,
	AF11,
	AF12,
	AF13,
	AF14,
	AF15
} AF_MODE;

typedef enum
{
	LOW = 0,
	HIGH = 1,
	false = 0,
	true = 1,
	OFF = 0,
	ON = 1,
	COUNTER_CLOCKWISE = 0,
	CLOCKWISE = 1,
	NO_TRIGGER_HANDLER = 0,
	TRIGGER_HANDLER = 1,
	null = 0,
	NO_DP = 0,
	DP = 1,
	REFRESH = 1
} Bool;

typedef enum
{
	RISING, FALLING
} TRIGGER_EDGE;

typedef enum
{
	DELAY, IRQ
} TIMER_ROLE;

typedef enum
{
	INTERNAL, EXTERNAL
} TIMER_SIGNAL;

typedef enum
{
	NO_IRQ = 0x0000, TRG_IRQ = 0x0001
} TRIGGER_IRQ;

typedef enum
{
	COUNTUP, COUNTDOWN, CENTER_ALIGN
} COUNT_MODE;

typedef enum
{
	PWM1, PWM2
} PWMMODE;

typedef enum
{
	NO_ICPSC = 0x0000,
	ICPSC_2 = 0x0004,
	ICPSC_4 = 0x0008,
	ICPSC_8 = 0x000C
} IC_PSC;

typedef enum
{
	NO_FILTER = 0X0000,
	FILTER_2 = 0x0010,
	FILTER_4 = 0x0020,
	FILTER_8 = 0x0030
} N_FILTER;

typedef enum
{
	NONINVERTING = 0X000, INVERTING = 0x0002, BOTH_EDGE = 0x000A
} CAPTURE_ACTIVE_EDGE;

typedef enum
{
	FULLSTEP = 1, HALFSTEP = 2
} STEPPER_MODE;

typedef enum
{
	STEP_1 = 1, STEP_2, STEP_3, STEP_4
} STEPPER_FULLSTEP;

typedef enum
{
	MICROSTEPPING_4 = 920,
	MICROSTEPPING_8 = 3000 /*3.8V*/,
	MICROSTEPPING_16 = 9000,
	MICROSTEPPING_32 = 30000/*4.3V*/,
	MICROSTEPPING_40 = 400000,
	MICROSTEPPING_256 = 100000
} STEPPER_PWM_FREQUENCY;

typedef enum
{
	NON_ZERO_LEADING, ZERO_LEADING
} NUMBER_FORMAT;

typedef enum
{
	ADC_1 = 0, ADC_2 = 1, ADC_3 = 2
} ADC_MODULE;

typedef union
{
	uint16_t debounce_time;
	uint16_t milisecond;
	uint32_t reload;
} DRAFT;

typedef enum
{
	AMT_HUMI, AMT_TEMP, INTERNAL_TEMP, INTERNAL_VREF, INTERNAL_VBATE
} ADC_VAL;

typedef enum
{
	DATE, TIME
} GET_DATETIME;

typedef enum
{
	SOFTWARE_TRIGGER, TIMER_TRIGGER
} ADC_TRIGGER_SRC;


#endif
