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


#ifndef _VANTAGE_SERIAL_H_
#define _VANTAGE_SERIAL_H_

#include <stdio.h>
#include "data_defs.h"

typedef void (VTG_DataReadyIndicateCb_t)(weather_data_t* weather_data);

/*******************************************************************************
 * Public API
 ******************************************************************************/
int  VTG_console_init(char* dev_path, int use_usb_serial, VTG_DataReadyIndicateCb_t DataReadyIndicateCb);
void VTG_console_exit(void);

#endif /* _VANTAGE_SERIAL_H_ */
