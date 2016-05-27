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

#include <stdarg.h>

#ifdef LOG_USES_SYSLOG
#pragma message "Log uses syslog"
#include <syslog.h>
#include <stdio.h>
#else
#pragma message "Log uses stderr"
#include <stdio.h>
#endif

#ifdef LOG_USES_SYSLOG
/* Syslog priority table */
static int LOG_TabPriority[] =
{
  LOG_DEBUG   ,/*LOG_LVL_DEBUG    */
  LOG_INFO    ,/*LOG_LVL_INFO     */
  LOG_NOTICE  ,/*LOG_LVL_NOTICE   */
  LOG_WARNING ,/*LOG_LVL_WARNING  */
  LOG_ERR     ,/*LOG_LVL_ERROR    */
  LOG_ALERT   ,/*LOG_LVL_ALERT    */
  LOG_EMERG   ,/*LOG_LVL_PANIC    */
};
#endif

void LOG_Init(const char* log_identifier)
{
  openlog(log_identifier, LOG_NDELAY, LOG_USER);
}

void LOG_printf(int priority, const char *format, ...)
{
  va_list va;

  /* Send message */
  va_start(va, format);
#ifdef LOG_USES_SYSLOG
  vsyslog(LOG_TabPriority[priority], format, va);
#else
  vfprintf(stderr, format, va);
#endif
  va_end(va);
}

void LOG_Close(void)
{
  closelog();
}
