//GasMeters.h
/*
 * GasMeters.h
 *
 *  Created on: 26.03.2013
 *      Author: mrhru
 */

#ifndef GASMETERS_H_
#define GASMETERS_H_

#include "defines.h"
#ifdef GAZ_STATION

#include "types.h"

extern volatile u32 OneKHzCounter;

typedef struct MeterStruct
{
	// ���������
	bool started;				// ���� ���������
	bool done;					// ���������
	bool error;					// ������
	// ������ � ��������� �������
	bool todo_done;				// ���������
	bool todo_error;			// ������

	u32 time_min;				// �����, ����� �������� ��������� ��������� �������� ������� � ������������ ���������
	u32 time_max;				// �������, ����� �������� ��������� �������������
	u32 time_counter;			// ����� ����� ������ � ��������� �������� ���������
	u32 timeout_counter;		// ����� ����� ���������

	s32 liter_counter;			// ������� �������� ���������, �� ������ ����������

	u32 rtc_prev;				// ��� ����������� ���������� ������� 1���, ������� �������
	u32 oneKHzCounter_prev;		// ��� ����������� ���������� ������� 1���, ������� �������
	bool liter_prev_state;		// ������� ��������� �����, ������ ��� �������� ������
	bool liter_state;			// ��������� �����, ������ ��� �������� ������

} TMeterStruct;

typedef TMeterStruct* PMeter;

extern TMeterStruct Meters[4];

typedef struct
{
	u8 LiterPort;
	u8 LiterPin;
	u8 CheckPort;
	u8 CheckPin;
} TMeterPort;

extern TMeterPort MeterPorts[METER_COUNT];

void Meters_Init(void);
void StartMeter(int n);

extern const struct TType TMeter;

#endif

#endif /* GASMETERS_H_ */
