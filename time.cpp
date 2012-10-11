/*
  time.c - low level time and date functions
  Copyright (c) Michael Margolis 2009

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  6  Jan 2010 - initial release 
  12 Feb 2010 - fixed leap year calculation error
  1  Nov 2010 - fixed setTime bug (thanks to Korman for this)
  24 Sep 2012 - mods for Akafugu clock (AVR)
  11 Oct 2012 - modified for VFD Deluxe Firmware by Akafugu Corporation
*/

#include "time.h"

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
 
void breakTime(time_t time, tmElements_t* tm)
{
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  unsigned long days;
  
  tm->Second = time % 60;
  time /= 60; // now it is minutes
  tm->Minute = time % 60;
  time /= 60; // now it is hours
  tm->Hour = time % 24;
  time /= 24; // now it is days
  tm->Wday = ((time + 4) % 7) + 1;  // Sunday is day 1 
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm->Year = year; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm->Month = month + 1;  // jan is month 1  
  tm->Day = time + 1;     // day of month
}

time_t makeTime(tmElements_t* tm){   
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  int i;
  time_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm->Year*(SECS_PER_DAY * 365);
  for (i = 0; i < tm->Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < tm->Month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm->Year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm->Day-1) * SECS_PER_DAY;
  seconds+= tm->Hour * SECS_PER_HOUR;
  seconds+= tm->Minute * SECS_PER_MIN;
  seconds+= tm->Second;
  return seconds; 
}

