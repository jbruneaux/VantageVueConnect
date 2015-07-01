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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "data_defs.h"

#define WUNDERGROUND_HOST_NAME   "weatherstation.wunderground.com"
#define WUNDERGROUND_GET_PAGE    "/weatherstation/updateweatherstation.php"

typedef struct weather_data_entry_s
{
  /* Actual content (might be shared between different TAILQ) */
  weather_data_t*             weather_data;

  /* Chaining entry */
  TAILQ_ENTRY(weather_data_entry_s) ListEntry;
} weather_data_entry_t;

#define TENM_WEATHER_DATA_COUNT  ((10*60) / VANTAGE_PERIODIC_WEATHER_DATA_QUERY_IN_S)
unsigned int tenm_weather_data_counter = 0;
TAILQ_HEAD(WeatherDataHead_s, weather_data_entry_s) WeatherDataHead_LastTenM, WeatherDataHead_LastHour, WeatherDataHead_LastDay;

#define HOUR_WEATHER_DATA_GRANULARITY_IN_S    60
#define HOUR_WEATHER_DATA_COUNT  ((60*60) / HOUR_WEATHER_DATA_GRANULARITY_IN_S)
#define HOUR_WEATHER_DATA_SKIP_COUNT       (HOUR_WEATHER_DATA_GRANULARITY_IN_S / VANTAGE_PERIODIC_WEATHER_DATA_QUERY_IN_S)
unsigned int hour_weather_data_counter = 0;
unsigned int hour_weather_data_skip_counter = HOUR_WEATHER_DATA_SKIP_COUNT;

#define DAY_WEATHER_DATA_GRANULARITY_IN_S    3600
#define DAY_WEATHER_DATA_COUNT  ((24*60*60) / DAY_WEATHER_DATA_GRANULARITY_IN_S)
#define DAY_WEATHER_DATA_SKIP_COUNT       (DAY_WEATHER_DATA_GRANULARITY_IN_S / VANTAGE_PERIODIC_WEATHER_DATA_QUERY_IN_S)
unsigned int day_weather_data_counter = 0;
unsigned int day_weather_data_skip_counter = DAY_WEATHER_DATA_SKIP_COUNT;


#define DATE_CONTENT_BUFFER_SIZE  (TENM_WEATHER_DATA_COUNT * 25) /* 23 is the maximum characters count used by
                                                                  * a date : '"YYYY-MM-DD HH:MM:SS",' */
#define DATA_CONTENT_BUFFER_SIZE  (TENM_WEATHER_DATA_COUNT * 10) /* 9 is the maximum characters count used by
                                                                  * a float value : '"XXXX.X",' */

extern int loglevel;

int wunderground_update(weather_data_t *weather_data, char *station_id, char* station_password)
{
  struct sockaddr_in serveraddr;
  struct hostent *server;
  unsigned char get_request_content[1024];
  unsigned char request[1024];
  unsigned char* http_str;
  int j;

  if ( loglevel > 1 )
  {
    fprintf(stderr, "+%s\n", __FUNCTION__);
  }

  int tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
     
  if (tcpSocket < 0)
  {
    fprintf(stderr, "Error opening socket");
    return -1;
  }

  server = gethostbyname(WUNDERGROUND_HOST_NAME);
  if (server == NULL)
  {
    fprintf(stderr, "gethostbyname() failed\n");
    close(tcpSocket);
    return -1;
  }

  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;

  bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
  serveraddr.sin_port = htons(80);
  
  if (connect(tcpSocket, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
  {
    fprintf(stderr, "Error Connecting");
    close(tcpSocket);
    return -1;
  }
 
  bzero(request, sizeof(request));

  j = 0;
  j = snprintf(get_request_content, sizeof(get_request_content)-j, 
                "%s?action=updateraw", 
                WUNDERGROUND_GET_PAGE);
  j += snprintf(get_request_content + j, sizeof(get_request_content)-j, 
                "&ID=%s&PASSWORD=%s&dateutc=%04d-%02d-%02d+%02d:%02d:%02d", 
                station_id, station_password, 
                weather_data->tm.tm_year + 1900, weather_data->tm.tm_mon + 1, weather_data->tm.tm_mday, 
                weather_data->tm.tm_hour, weather_data->tm.tm_min, weather_data->tm.tm_sec);
  j += snprintf(get_request_content + j, sizeof(get_request_content)-j, 
                "&winddir=%d&windspeedmph=%3.1f&windspdmph_avg2m=%3.1f",
                weather_data->wind_direction, weather_data->wind_speed_MPH, weather_data->wind_speed_avg_2m_MPH);
  if ( weather_data->wind_gust_10m != FLT_MIN )
  {
    j += snprintf(get_request_content + j, sizeof(get_request_content)-j, 
                  "&windgustmph=%3.1f&windgustmph_10m=%3.1f",
                  weather_data->wind_gust_10m, weather_data->wind_gust_10m);

  }
  j += snprintf(get_request_content + j, sizeof(get_request_content)-j, 
                "&humidity=%d&tempf=%2.1f&dewptf=%2.1f&indoortempf=%2.1f&indoorhumidity=%2.1f",
                weather_data->outside_humidity, weather_data->outside_temperature_F, weather_data->dew_point_F, 
                weather_data->inside_temperature_F, weather_data->inside_humidity);
  j += snprintf(get_request_content + j, sizeof(get_request_content)-j, 
                "&baromin=%3.1f",
                weather_data->barometric_pressure_I);
  j += snprintf(get_request_content + j, sizeof(get_request_content)-j, 
                "&dailyrainin=",
                weather_data->rain_day_I);

  snprintf(request, sizeof(request), 
           "GET %s HTTP/1.0\r\n"  // POST or GET, both tested and works. Both HTTP 1.0 HTTP 1.1 works, but sometimes 
           "Host: %s\r\n\r\n",     // but sometimes HTTP 1.0 works better in localhost type
           get_request_content, WUNDERGROUND_HOST_NAME);

  if (send(tcpSocket, request, strlen(request), 0) < 0)
  {
    fprintf(stderr, "Error with send()");
    close(tcpSocket);
    return -1;
  }
   
  bzero(request, sizeof(request));
   
  recv(tcpSocket, request, sizeof(request), 0);
  //printf("Received :\n%s", request);

  http_str = strstr(request, "HTTP/");
  if ( http_str != NULL )
  {
    http_str += sizeof("HTTP/") + 3;
    if ( strstr(http_str, "200 OK") == (char*)http_str )
    {
      if ( loglevel > 0 )
      {
        fprintf(stderr, "Success\n");
      }
    }
    else
    {
      fprintf(stderr, "Error updating wunderground\n");
    }
  }

  close(tcpSocket);

  if ( loglevel > 1 )
  {
    fprintf(stderr, "-%s\n", __FUNCTION__);
  }

  return 0;
}

void local_web_init(void)
{
  /* Initialize list */
  TAILQ_INIT(&(WeatherDataHead_LastTenM));
  TAILQ_INIT(&(WeatherDataHead_LastHour));
  TAILQ_INIT(&(WeatherDataHead_LastDay));
}

static int local_web_update_data_section(int fd_out, struct WeatherDataHead_s* WeatherDataHead)
{
  unsigned char *file_data = NULL, *dates_content = NULL, *data_content_1 = NULL, *data_content_2 = NULL;
  int i, ret = 0, n, len, dates_len, data1_len, data2_len, file_data_size;
  weather_data_entry_t *tmp;
  weather_data_t *weather_data;

  if ( loglevel > 1 )
  {
    fprintf(stderr, "+%s\n", __FUNCTION__);
  }

  dates_content = malloc(DATE_CONTENT_BUFFER_SIZE);
  if ( dates_content == NULL )
  {
    perror("malloc");
    ret = -1;
    goto local_web_update_data_section_exit;
  }

  data_content_1 = malloc(DATA_CONTENT_BUFFER_SIZE);
  if ( data_content_1 == NULL )
  {
    perror("malloc");
    ret = -1;
    goto local_web_update_data_section_exit;
  }

  data_content_2 = malloc(DATA_CONTENT_BUFFER_SIZE);
  if ( data_content_2 == NULL )
  {
    perror("malloc");
    ret = -1;
    goto local_web_update_data_section_exit;
  }

  i = 0;
  dates_len = 0;
  TAILQ_FOREACH(tmp, WeatherDataHead, ListEntry) 
  {
    weather_data = tmp->weather_data;
    dates_len += snprintf(dates_content + dates_len, DATE_CONTENT_BUFFER_SIZE - dates_len, "\"%04d-%02d-%02d %02d:%02d:%02d\",", weather_data->tm.tm_year + 1900, weather_data->tm.tm_mon + 1, weather_data->tm.tm_mday, weather_data->tm.tm_hour, weather_data->tm.tm_min, weather_data->tm.tm_sec);
    i++;
  }
  dates_len -= 1;
  dates_content[dates_len] = '\0';

  if ( loglevel > 1 )
  {
    fprintf(stdout, "Updating web using %d records\n", i);
  }

  data1_len = 0;
  data2_len = 0;
  TAILQ_FOREACH(tmp, WeatherDataHead, ListEntry) 
  {
    weather_data = tmp->weather_data;
    data1_len += snprintf(data_content_1 + data1_len, DATA_CONTENT_BUFFER_SIZE - data1_len, "\"%2.1f\",", weather_data->outside_temperature_C);
    data2_len += snprintf(data_content_2 + data2_len, DATA_CONTENT_BUFFER_SIZE - data2_len, "\"%2.1f\",", weather_data->outside_chill_C);
  }
  data1_len -= 1;
  data2_len -= 1;
  data_content_1[data1_len] = '\0';
  data_content_2[data2_len] = '\0';

  file_data_size = dates_len + data1_len + data2_len + 200;  
  file_data = malloc(file_data_size);
  if ( file_data == NULL )
  {
    perror("malloc");
    ret = -1;
    goto local_web_update_data_section_exit;
  }

  memset(file_data, '\0', file_data_size);

  len = snprintf(file_data, file_data_size, "                  var OutsideTempData = [[%s],[[%s],[%s]]];\n", dates_content, data_content_1, data_content_2);

  n = write(fd_out, file_data, len);
  if ( n < len )
  {
    perror("write");
    ret = -1;
    goto local_web_update_data_section_exit;
  }

  data1_len = 0;
  TAILQ_FOREACH(tmp, WeatherDataHead, ListEntry) 
  {
    weather_data = tmp->weather_data;
    data1_len += snprintf(data_content_1 + data1_len, DATA_CONTENT_BUFFER_SIZE - data1_len, "\"%d\",", weather_data->outside_humidity);
  }
  data1_len -= 1;
  data_content_1[data1_len] = '\0';
  len = snprintf(file_data, file_data_size, "                  var OutsideHumidityData = [[%s],[[%s]]];\n", dates_content, data_content_1);

  n = write(fd_out, file_data, len);
  if ( n < len )
  {
    perror("write");
    ret = -1;
    goto local_web_update_data_section_exit;
  }


  data1_len = 0;
  TAILQ_FOREACH(tmp, WeatherDataHead, ListEntry) 
  {
    weather_data = tmp->weather_data;
    data1_len += snprintf(data_content_1 + data1_len, DATA_CONTENT_BUFFER_SIZE - data1_len, "\"%4.1f\",", weather_data->barometric_pressure_Hpa);
  }
  data1_len -= 1;
  data_content_1[data1_len] = '\0';
  len = snprintf(file_data, file_data_size, "                  var BarometerData = [[%s],[[%s]]];\n", dates_content, data_content_1);

  n = write(fd_out, file_data, len);
  if ( n < len )
  {
    perror("write");
    ret = -1;
    goto local_web_update_data_section_exit;
  }

  data1_len = 0;
  TAILQ_FOREACH(tmp, WeatherDataHead, ListEntry) 
  {
    weather_data = tmp->weather_data;
    data1_len += snprintf(data_content_1 + data1_len, DATA_CONTENT_BUFFER_SIZE - data1_len, "\"%4.1f\",", weather_data->wind_speed_KPH);
  }
  data1_len -= 1;
  data_content_1[data1_len] = '\0';
  len = snprintf(file_data, file_data_size, "                  var WindSpeedData = [[%s],[[%s]]];\n", dates_content, data_content_1);

  n = write(fd_out, file_data, len);
  if ( n < len )
  {
    perror("write");
    ret = -1;
    goto local_web_update_data_section_exit;
  }

  data1_len = 0;
  TAILQ_FOREACH(tmp, WeatherDataHead, ListEntry) 
  {
    weather_data = tmp->weather_data;
    data1_len += snprintf(data_content_1 + data1_len, DATA_CONTENT_BUFFER_SIZE - data1_len, "\"%4.1f\",", weather_data->rain_rate_MM);
  }
  data1_len -= 1;
  data_content_1[data1_len] = '\0';
  len = snprintf(file_data, file_data_size, "                  var RainRateData = [[%s],[[%s]]];\n", dates_content, data_content_1);

  n = write(fd_out, file_data, len);
  if ( n < len )
  {
    perror("write");
    ret = -1;
    goto local_web_update_data_section_exit;
  }

  data1_len = 0;
  TAILQ_FOREACH(tmp, WeatherDataHead, ListEntry) 
  {
    weather_data = tmp->weather_data;
    data1_len += snprintf(data_content_1 + data1_len, DATA_CONTENT_BUFFER_SIZE - data1_len, "\"%2.1f\",", weather_data->inside_temperature_C);
  }
  data1_len -= 1;
  data_content_1[data1_len] = '\0';
  len = snprintf(file_data, file_data_size, "                  var InsideTempData = [[%s],[[%s]]];\n", dates_content, data_content_1);

  n = write(fd_out, file_data, len);
  if ( n < len )
  {
    perror("write");
    ret = -1;
    goto local_web_update_data_section_exit;
  }

  data1_len = 0;
  TAILQ_FOREACH(tmp, WeatherDataHead, ListEntry) 
  {
    weather_data = tmp->weather_data;
    data1_len += snprintf(data_content_1 + data1_len, DATA_CONTENT_BUFFER_SIZE - data1_len, "\"%d\",", weather_data->inside_humidity);
  }
  data1_len -= 1;
  data_content_1[data1_len] = '\0';
  len = snprintf(file_data, file_data_size, "                  var InsideHumidityData = [[%s],[[%s]]];\n", dates_content, data_content_1);

  n = write(fd_out, file_data, len);
  if ( n < len )
  {
    perror("write");
    ret = -1;
    goto local_web_update_data_section_exit;
  }

local_web_update_data_section_exit:
  if ( file_data != NULL )
    free(file_data);
  if ( dates_content != NULL )
    free(dates_content);
  if ( data_content_1 != NULL )
    free(data_content_1);
  if ( data_content_2 != NULL )
    free(data_content_2);

  if ( loglevel > 1 )
  {
    fprintf(stderr, "-%s\n", __FUNCTION__);
  }

  return ret;
}

int local_web_update(weather_data_t *weather_data, unsigned char* www_root)
{
  unsigned char html_path[1024];
  unsigned char* file_content = NULL;
  int ret = 0, fd_in_b = -1, fd_in_e = -1, fd_out_tenm = -1, fd_out_hour = -1, fd_out_day = -1, n, len, file1_size, file2_size;
  struct stat file_stat;
  struct tm tm = weather_data->tm;
  weather_data_t *weather_data_copy;
  weather_data_entry_t *tmp, *new_tenm, *new_hour, *new_day;

  if ( loglevel > 1 )
  {
    fprintf(stderr, "+%s\n", __FUNCTION__);
  }

  new_tenm = malloc(sizeof(weather_data_entry_t));
  if ( new_tenm == NULL )
  {
    perror("malloc");
    return -1;
  }

  new_hour = malloc(sizeof(weather_data_entry_t));
  if ( new_hour == NULL )
  {
    perror("malloc");
    free(new_tenm);
    return -1;
  }

  new_day = malloc(sizeof(weather_data_entry_t));
  if ( new_day == NULL )
  {
    perror("malloc");
    free(new_tenm);
    free(new_hour);
    return -1;
  }

  weather_data_copy = malloc(sizeof(weather_data_t));
  if ( weather_data_copy == NULL )
  {
    perror("malloc");
    free(new_tenm);
    free(new_hour);
    free(new_day);
    return -1;
  }
  
  memcpy(weather_data_copy, weather_data, sizeof(weather_data_t));

  new_tenm->weather_data = weather_data_copy;
  new_hour->weather_data = weather_data_copy;
  new_day->weather_data = weather_data_copy;

  /* If the 10 minute weather data list has reached its maximum, then remove
   * the first (oldest) element
   */
  if ( loglevel > 2 )
  {
    fprintf(stdout, "T s%d/%d c%d/%d\n", 0, 0,
             tenm_weather_data_counter, TENM_WEATHER_DATA_COUNT);
  }
  if ( tenm_weather_data_counter == TENM_WEATHER_DATA_COUNT )
  {
    tmp = TAILQ_FIRST(&WeatherDataHead_LastTenM);
    TAILQ_REMOVE(&WeatherDataHead_LastTenM, tmp, ListEntry);
    free(tmp);
    tenm_weather_data_counter--;
  }
  TAILQ_INSERT_TAIL(&WeatherDataHead_LastTenM, new_tenm, ListEntry);
  tenm_weather_data_counter++;

  /* Update the hour data only every X received data TODO : Average over the last minute ?*/
  if ( loglevel > 2 )
  {
    fprintf(stdout, "H s%d/%d c%d/%d\n", hour_weather_data_skip_counter, HOUR_WEATHER_DATA_SKIP_COUNT,
             hour_weather_data_counter, HOUR_WEATHER_DATA_COUNT);
  }
  if ( hour_weather_data_skip_counter == HOUR_WEATHER_DATA_SKIP_COUNT )
  {
    if ( hour_weather_data_counter == HOUR_WEATHER_DATA_COUNT )
    {
      tmp = TAILQ_FIRST(&WeatherDataHead_LastHour);
      TAILQ_REMOVE(&WeatherDataHead_LastHour, tmp, ListEntry);
      free(tmp);
      hour_weather_data_counter--;
    }
    TAILQ_INSERT_TAIL(&WeatherDataHead_LastHour, new_hour, ListEntry);
    hour_weather_data_counter++;
  }

  /* Update the day data only every X received data TODO : Average over the last hour ?*/
  if ( loglevel > 2 )
  {
    fprintf(stdout, "D s%d/%d c%d/%d\n", day_weather_data_skip_counter, DAY_WEATHER_DATA_SKIP_COUNT,
             day_weather_data_counter, DAY_WEATHER_DATA_COUNT);
  }
  if ( day_weather_data_skip_counter == DAY_WEATHER_DATA_SKIP_COUNT )
  {
    if ( day_weather_data_counter == DAY_WEATHER_DATA_COUNT )
    {
      tmp = TAILQ_FIRST(&WeatherDataHead_LastDay);
      TAILQ_REMOVE(&WeatherDataHead_LastDay, tmp, ListEntry);
      free(tmp->weather_data);
      free(tmp);
      day_weather_data_counter--;
    }
    TAILQ_INSERT_TAIL(&WeatherDataHead_LastDay, new_day, ListEntry);
    day_weather_data_counter++;
  }
  

  fd_in_b = open("/usr/share/vantage_connect/html_template/weather_chart_begin.html", O_RDONLY);
  if ( fd_in_b < 0 )
  {
    perror("Unable to open input file");
    ret = -1;
    goto local_web_update_exit;
  }

  fd_in_e = open("/usr/share/vantage_connect/html_template/weather_chart_end.html", O_RDONLY);
  if ( fd_in_e < 0 )
  {
    perror("Unable to open input file");
    ret = -1;
    goto local_web_update_exit;
  }

  snprintf(html_path, sizeof(html_path), "%s/weather_chart_10m.html", www_root);
  fd_out_tenm = open(html_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  if ( fd_out_tenm < 0 )
  {
    perror("Unable to open output file");
    ret = -1;
    goto local_web_update_exit;
  }

  if ( hour_weather_data_skip_counter == HOUR_WEATHER_DATA_SKIP_COUNT )
  {
    snprintf(html_path, sizeof(html_path), "%s/weather_chart_1h.html", www_root);
    fd_out_hour = open(html_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if ( fd_out_hour < 0 )
    {
      perror("Unable to open output file");
      ret = -1;
      goto local_web_update_exit;
    }
  }

  if ( day_weather_data_skip_counter == DAY_WEATHER_DATA_SKIP_COUNT )
  {
    snprintf(html_path, sizeof(html_path), "%s/weather_chart_24h.html", www_root);
    fd_out_day = open(html_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if ( fd_out_day < 0 )
    {
      perror("Unable to open output file");
      ret = -1;
      goto local_web_update_exit;
    }
  }

  /* Write HTML page header */
  if ( fstat(fd_in_b, &file_stat) == -1 )
  {
    perror("stat");
    ret = -1;
    goto local_web_update_exit;
  }

  file1_size = file_stat.st_size;

  if ( fstat(fd_in_e, &file_stat) == -1 )
  {
    perror("stat");
    ret = -1;
    goto local_web_update_exit;
  }
  file2_size = file_stat.st_size;

  file_content = malloc((file1_size > file2_size) ? file1_size : file2_size);
  if ( file_content == NULL )
  {
    perror("malloc");
    ret = -1;
    goto local_web_update_exit;
  }

  n = read(fd_in_b, file_content, file1_size);
  if ( n < file1_size )
  {
    perror("read");
    ret = -1;
    goto local_web_update_exit;
  }

  n = write(fd_out_tenm, file_content, file1_size);
  if ( n < file1_size )
  {
    perror("write");
    ret = -1;
    goto local_web_update_exit;
  }

  if ( hour_weather_data_skip_counter == HOUR_WEATHER_DATA_SKIP_COUNT )
  {
    n = write(fd_out_hour, file_content, file1_size);
    if ( n < file1_size )
    {
      perror("write");
      ret = -1;
      goto local_web_update_exit;
    }
  }

  if ( day_weather_data_skip_counter == DAY_WEATHER_DATA_SKIP_COUNT )
  {
    n = write(fd_out_day, file_content, file1_size);
    if ( n < file1_size )
    {
      perror("write");
      ret = -1;
      goto local_web_update_exit;
    }
  }

  /* Fill the data vars for the 10 minutes page*/
  local_web_update_data_section(fd_out_tenm, &WeatherDataHead_LastTenM);

  /* Fill the data vars for the 1 hour page */
  if ( hour_weather_data_skip_counter == HOUR_WEATHER_DATA_SKIP_COUNT )
  {
    local_web_update_data_section(fd_out_hour, &WeatherDataHead_LastHour);
  }

  /* Fill the data vars for the 24 hour page */
  if ( day_weather_data_skip_counter == DAY_WEATHER_DATA_SKIP_COUNT )
  {
    local_web_update_data_section(fd_out_day, &WeatherDataHead_LastDay);
  }

  /* Write HTML page footer */
  n = read(fd_in_e, file_content, file2_size);
  if ( n < file2_size )
  {
    perror("read");
    ret = -1;
    goto local_web_update_exit;
  }

  n = write(fd_out_tenm, file_content, file2_size);
  if ( n < file2_size )
  {
    perror("write");
    ret = -1;
    goto local_web_update_exit;
  }

  if ( hour_weather_data_skip_counter == HOUR_WEATHER_DATA_SKIP_COUNT )
  {
    n = write(fd_out_hour, file_content, file2_size);
    if ( n < file2_size )
    {
      perror("write");
      ret = -1;
      goto local_web_update_exit;
    }
  }

  if ( day_weather_data_skip_counter == DAY_WEATHER_DATA_SKIP_COUNT )
  {
    n = write(fd_out_day, file_content, file2_size);
    if ( n < file2_size )
    {
      perror("write");
      ret = -1;
      goto local_web_update_exit;
    }
  }

local_web_update_exit:
  if ( fd_in_b > 0 )
    close(fd_in_b);
  if ( fd_in_e > 0 )
    close(fd_in_e);
  if ( fd_out_tenm > 0 )
    close(fd_out_tenm);
  if ( fd_out_hour > 0 )
    close(fd_out_hour);
  if ( fd_out_day > 0 )
    close(fd_out_day);
  if ( file_content != NULL )
    free(file_content);


  if ( hour_weather_data_skip_counter == HOUR_WEATHER_DATA_SKIP_COUNT )
  {
    hour_weather_data_skip_counter = 0;
  }
  hour_weather_data_skip_counter ++;


  if ( day_weather_data_skip_counter == DAY_WEATHER_DATA_SKIP_COUNT )
  {
    day_weather_data_skip_counter = 0;
  }
  day_weather_data_skip_counter ++;

  if ( loglevel > 1 )
  {
    fprintf(stderr, "-%s\n", __FUNCTION__);
  }

  return ret;
}
