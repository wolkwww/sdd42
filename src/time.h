//time.h

#ifndef __TIME_H
#define __TIME_H

/*
 * Copyright (C) 2001-2003 by egnite Software GmbH. All rights reserved.
...
 *
 * For additional information see http://www.ethernut.de/
 */

/*!
 * \addtogroup xgCrtTime
 * @{
 */

//#include <stdio.h>
#include <sys/types.h>

#include "lpc_types.h"
#include "types.h"
//#include "common.h"

/*!
 * \brief Type definition for struct _tm
 */
typedef struct _tm tm;

/*! 
 * \brief structure to store a date/time value. 
 *
 * The structure tm stores a complete date and time combination. The granularity 
 * is one second.
 *
 * \note The range of \b tm_mon is from 0 (January) to 11 (December).\n
 *       \b tm_year holds the year since 1900, for example value 103 means year 2003.
 */
struct _tm {
	u32	tm_sec;                 /*!< \brief seconds after the minute - [0,59] */
	u32	tm_min;                 /*!< \brief minutes after the hour - [0,59] */
	u32	tm_hour;                /*!< \brief hours since midnight - [0,23] */
	u32	tm_mday;                /*!< \brief day of the month - [1,31] */
	u32	tm_mon;                 /*!< \brief months since January - [0,11] */
	u32	tm_year;                /*!< \brief years since 1900 */
	u32	tm_wday;                /*!< \brief days since Sunday - [0,6] */
	u32	tm_yday;                /*!< \brief days since January 1 - [0,365] */
	u32	tm_isdst;               /*!< \brief daylight savings time flag */
};

/*! 
 * \typedef long time_t
 * \brief Serial date/time. Holds number of seconds after January 1st, 1970.
 */
//typedef u32 time_t;

//time_t time(time_t * timer);
s32 gmtime_x(const time_t* timer, tm* theTime);

//tm* gmtime(const time_t* timer);
//u32 localtime_r(const time_t * timer, tm * theTime);
//tm *localtime(const time_t * timer);

//u32 stime(time_t * timer);
//time_t mktime(tm * timeptr);
time_t _mkgmtime(tm * timeptr);

/*! 
 * \brief Used to control daylight conversions.
 *
 * Assign a nonzero value to enable daylight conversions. If enabled
 * the hour part of time values is adjusted if we are in daylight saving time.
 * Zero value to disable conversion.
 * Default is enabled.
 */
extern uint8_t _daylight;

/*!
 * \brief Defines your local timezone.
 *
 * Difference in seconds between universal coordinated time and local time. 
 * This value is subtracted from the universal coordinated time to 
 * calculate your local time. Default value is 5 * 60 * 60 = 18000, 
 * which defines the time zone EST (GMT-5).
 *
 * \note Before using the time functions, set \b _timezone to your 
 * local value.
 * 
 */
extern long _timezone;

/*!
 * \brief Difference between standard and daylight savings time in seconds.
 *
 * This value is used to calculate the daylight savings time by subtracting this
 * value from the standard time. Unit is seconds.
 * Usually daylight savings time is one hour in advance to standard time, thus the default
 * value is -1 * 60 * 60 = -3600.
 */
extern long _dstbias;

/*@}*/
#endif
