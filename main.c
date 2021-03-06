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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "vantage_serial.h"
#include "data_defs.h"
#include "data_updater.h"
#include "log.h"


static char *dev_filename = "/dev/ttyUSB";
int dev_filename_opt = 0;
static char *www_root = "/tmp";
static char *station_id = NULL;
static char *station_password = NULL;

int loglevel = 0;
int use_loop2 = 1;
int use_usb_serial = 0;

static void print_usage(char* prog_name)
{
    printf("Usage %s:\n", prog_name);
    printf("\t-n --no-loop2            : Don't use LOOP2 packet request (default : use LOOP2)\n");
    printf("\t-w --www=[WWW_ROOT]      : Folder where the web pages are written (default : /tmp) \n");  
    printf("\t-s --station_id=[ID]     : Wunderground station id (default : none)\n");
    printf("\t-p --password=[PASS]     : Wunderground password (default : none)\n");
    printf("\t-u --usb-serial          : Use a usb to serial device. In this case, dev can be set as\n"
                                        "device prefix (i.e : /dev/ttySER). If no prefix is given, the\n"
                                        "default prefix is /dev/ttyUSB\n");
    printf("\t-d --dev=[DEV_FILENAME]  : Device to open (e.g : /dev/ttyUSB0) or prefix is -u is used \n");  
    printf("\n");
    printf("\t-l --loglevel=[LEVEL]    : Log level (default : 0)\n");
    printf("\t-h --help                : Display tool usage\n");

    exit(0);
}

static int parse_dev_file(char *optarg)
{
  dev_filename = optarg;
  dev_filename_opt = 1;

  return(0);
}

static int parse_www_root(char *optarg)
{
  www_root = optarg;

  return(0);
}

static int parse_stationid(char *optarg)
{
  station_id = optarg;

  return(0);
}

static int parse_password(char *optarg)
{
  station_password = optarg;

  return(0);
}

static int parse_loglevel(char *optarg)
{
  loglevel = atoi(optarg);

  return (0);
}

static int parse_cmd_line(int argc, char **argv)
{
	int c;

	typedef int (*PFI)();

	PFI parse_param[] = { parse_dev_file, parse_www_root, parse_stationid, parse_password, parse_loglevel };

  while (1)
  {
    //int this_option_optind = optind ? optind : 1;
    int option_index = 0;

    /* Options with arguments and parse functions must be kept at the top of the struct */
    static struct option long_options[] = 
    {
      {"dev", required_argument, 0, 'd'},
      {"www", required_argument, 0, 'w'},
      {"station_id", required_argument, 0, 's'},
      {"password", required_argument, 0, 'p'},

      {"loglevel", required_argument, 0, 'l'},

      /* No arguments options */
      {"usb-serial", no_argument, 0, 'u'},
      {"no-loop2", no_argument, 0, 'n'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };

    c = getopt_long_only (argc, argv, "d:w:s:p:unh", long_options, &option_index);

    if (c == -1) 
    {
      break;
    }

    switch (c) 
    {
      case 0:
#ifdef OPTION_DEBUG
        fprintf(stderr, "option %s", long_options[option_index].name);

        if (optarg) 
        {
          fprintf(stderr, " with arg %s", optarg);
        }

        fprintf(stderr, "\n");
#endif

        (*parse_param[option_index])(optarg);
      break;

      case 'd':
        parse_dev_file(optarg);
      break;

      case 'w':
        parse_www_root(optarg);
      break;

      case 's':
        parse_stationid(optarg);
      break;

      case 'p':
        parse_password(optarg);
      break;

      case 'l':
        parse_loglevel(optarg);
      break;

      case 'n':
        use_loop2 = 0;
      break;

      case 'u':
        use_usb_serial = 1;
      break;

      case 'h':
      case '?':
      default:
        print_usage(argv[0]);
      break;
    }
	}

	return(0);
}

static void weather_data_ready_indicate_cb(weather_data_t* weather_data)
{
  if (loglevel > 2)
  {
    LOG_printf(LOG_LVL_INFO, "Received weather data\n");
    
    LOG_printf(LOG_LVL_INFO, "\tOutside temperature : %fC / %fF\n", weather_data->outside_temperature_C, weather_data->outside_temperature_F);
    LOG_printf(LOG_LVL_INFO, "\tOutside chill : %fC / %fF\n", weather_data->outside_chill_C, weather_data->outside_chill_F);
    LOG_printf(LOG_LVL_INFO, "\tOutside humidity : %d%%\n", weather_data->outside_humidity);
    LOG_printf(LOG_LVL_INFO, "\tDew point : %fC\n", weather_data->dew_point_C);
    LOG_printf(LOG_LVL_INFO, "\tInside temperature : %fC\n", weather_data->inside_temperature_C);
    LOG_printf(LOG_LVL_INFO, "\tInside humidity : %d%%\n", weather_data->inside_humidity);
    LOG_printf(LOG_LVL_INFO, "\tBarometer : %fhPa / %fI\n", weather_data->barometric_pressure_Hpa, weather_data->barometric_pressure_I);
    LOG_printf(LOG_LVL_INFO, "\tRain rate : %fmm\n", weather_data->rain_rate_MM);
    LOG_printf(LOG_LVL_INFO, "\tRain day : %fmm\n", weather_data->rain_day_MM);
    LOG_printf(LOG_LVL_INFO, "\tWind Speed : %fkm/h / %fmph\n", weather_data->wind_speed_KPH, weather_data->wind_speed_MPH);
    LOG_printf(LOG_LVL_INFO, "\tWind Speed Avg 2m : %fkm/h\n", weather_data->wind_speed_avg_2m_KPH);
    LOG_printf(LOG_LVL_INFO, "\tWind direction : %d degrees\n", weather_data->wind_direction);
  }

  if ((station_id != NULL) && (station_password != NULL))
  {
    wunderground_update(weather_data, station_id, station_password);
  }
  local_web_update(weather_data, www_root);
}

int main(int argc, char**argv)
{
  /* Parse command line options */
  parse_cmd_line(argc, argv);

  if ((dev_filename_opt == 0) &&
      (use_usb_serial == 0))
  {
    print_usage(argv[0]);
    fprintf(stderr, "\n\nError : specify device to use\n");
    return -1;
  }

  LOG_Init("VantageVueConnect");

  local_web_init();

  VTG_console_init(dev_filename, use_usb_serial, weather_data_ready_indicate_cb);

  pause();

  LOG_Close();

  return 0;
}

