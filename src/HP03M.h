// HP03M.h

#ifndef _HP03M_H_
#define _HP03M_H_

#define MAX_SENS_COUNT	4
extern u32 SensCount;
extern u32 php; // указатель на текущий сенсор

#define AVG_BUF_SIZE	8

typedef struct
{
		float AvgBuf[AVG_BUF_SIZE];
		int AvgPos;
		float AvgSum;
		float Avg;
} AvgBufDef;


typedef struct HP03M_Sensor_t
{
		float H_C1;
		float H_C2;
		float H_C3;
		float H_C4;
		float H_C5;
		float H_C6;
		float H_C7;
		float H_D1; // measured pressure
		float H_D2; // measured temperature

		u8 H_A, H_B, H_C, H_D;

		float H_Press;
		float H_Temp;

		AvgBufDef AvgPress;
//		float AvgPressBuf[AVG_BUF_SIZE];
//		int AvgPressPos;
//		float AvgPress;

		AvgBufDef AvgTemp;
//		float AvgTempBuf[AVG_BUF_SIZE];
//		int AvgTempPos;
//		float AvgTemp;

		float H_Press_Offset;
		float H_Temp_Offset;

} HP03M_Sensor;

typedef HP03M_Sensor* PHP03M_Sensor;

extern HP03M_Sensor Sens[MAX_SENS_COUNT];

extern volatile u16 CCR1_Val;
extern volatile u16 CCR2_Val;

//extern float H_Press, H_Temp;

void HP03M_Init(void);
void HP03M_4th_Init(void);
void hp03m_process(void);

#endif
