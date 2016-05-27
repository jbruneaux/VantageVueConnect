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


#ifndef _LOG_H_
#define _LOG_H_

/* Log Level */
typedef enum LOG_Level_e
{
  LOG_LVL_DEBUG,    /* For debug purpose */
  LOG_LVL_INFO,     /* For information purpose */
  LOG_LVL_NOTICE,   /* For important notification purpose */
  LOG_LVL_WARNING,  /* For warning purpose */
  LOG_LVL_ERROR,    /* For error purpose */
  LOG_LVL_ALERT,    /* For alert purpose */
  LOG_LVL_PANIC     /* For panic purpose */
}
LOG_Level_t;

void LOG_Init(const char* log_identifier);
void LOG_Close(void);

void LOG_printf(int priority, const char *format, ...);

#endif
