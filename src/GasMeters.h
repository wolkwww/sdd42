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
	// состо€ни€
	bool started;				// идет измерение
	bool done;					// завершено
	bool error;					// ошибка
	// помощь в генерации событий
	bool todo_done;				// завершено
	bool todo_error;			// ошибка

	u32 time_min;				// врем€, после которого ожидаетс€ последний литровый импульс и закачиваетс€ измерение
	u32 time_max;				// таймаут, после которого измерени€ заканчиваютс€
	u32 time_counter;			// врем€ между первым и последним литровым импульсом
	u32 timeout_counter;		// общее врем€ измерени€

	s32 liter_counter;			// счетчик литровых импульсов, не счита€ стартового

	u32 rtc_prev;				// дл€ обнаружени€ отсутстви€ сигнала 1к√ц, прежние секунды
	u32 oneKHzCounter_prev;		// дл€ обнаружени€ отсутстви€ сигнала 1к√ц, прежний счетчик
	bool liter_prev_state;		// прошлое состо€ние входа, служит дл€ подсчета литров
	bool liter_state;			// состо€ние входа, служит дл€ подсчета литров

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
