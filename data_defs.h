/*
 * VantageVue Connect - A small program to connect a vantage vue console
 * Copyright (C) 2015  BRUNEAUX Jerome <jbruneaux@laposte.net>
 *
 * This file is part of VantageVueConnect.
 *
 * VantageVueConnect is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * any later version.
 *
 * VantageVueConnect is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with VantageVueConnect.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _DATA_DEFS_H_
#define _DATA_DEFS_H_

#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include <endian.h>

#include <sys/queue.h>


/*
  __BIG_ENDIAN__ and __LITTLE_ENDIAN__ are define in some gcc versions
  only, probably depending on the architecture. Try to use endian.h if
  the gcc way fails - endian.h also doesn not seem to be available on all
  platforms.
*/
#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#else /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
#undef WORDS_BIGENDIAN
#else
#ifdef BSD
#include <sys/endian.h>
#else
#include <endian.h>
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#undef WORDS_BIGENDIAN
#else
#error "unable to determine endianess!"
#endif /* __BYTE_ORDER */
#endif /* __LITTLE_ENDIAN__ */
#endif /* __BIG_ENDIAN__ */

#define VANTAGE_PERIODIC_WEATHER_DATA_QUERY_IN_S  5
#define MAX_WEATHER_DATA_FOR_24HR (24*60*60) / VANTAGE_PERIODIC_WEATHER_DATA_QUERY_IN_S

#ifdef WORDS_BIGENDIAN
#define BSWAP16(x) bswap_16(x)
#else
#define BSWAP16(x) (x)
#endif

typedef struct weather_data_s
{
  struct tm tm;

  /* Temperatures */
  float outside_temperature_F;
  float outside_temperature_C;
  float outside_chill_F;
  float outside_chill_C;
  float dew_point_F;
  float dew_point_C;

  float inside_temperature_F;
  float inside_temperature_C;

  /* Rain */
  float rain_rate_I;
  float rain_rate_MM;
  float rain_day_I;
  float rain_day_MM;

  /* Wind */
  float wind_speed_MPH;
  float wind_speed_KPH;
  float wind_speed_avg_2m_MPH;
  float wind_speed_avg_2m_KPH;
  
  int   wind_direction;
  float wind_gust;
  float wind_gust_10m;
  int   wind_direction_gust_10m;

  /* Others */
  int   outside_humidity;
  int   inside_humidity;
  float barometric_pressure_I;
  float barometric_pressure_Hpa;

} weather_data_t;

#endif /* _DATA_DEFS_H_ */
