#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "calendar.h"

struct _Breviary {

  time_t time;

  /**
   * Whether this is Year 1 or 2.
   */
  unsigned int year1or2: 2;

  unsigned int hour:6;

  /* Flags */
  unsigned int debug:1;
  unsigned int bvmblue:1;
  unsigned int adventblue:1;
  unsigned int rose:1;

  unsigned int facets[5];

};

Breviary *
breviary_new (time_t when)
{
  Breviary *result = malloc (sizeof (Breviary));

  result->time = when;
  result->facets[0] = 0;

  result->debug = 0;
  result->bvmblue = 0;
  result->adventblue = 0;
  result->rose = 0;

  return result;
}

void
breviary_set_flag (Breviary *breviary,
		   const char *flag,
		   int value)
{
  value = !!value;

  if (strcmp("debug", flag)==0) {
    breviary->debug = value;
  } else if (strcmp("bvmblue", flag)==0) {
    breviary->bvmblue = value;
  } else if (strcmp("adventblue", flag)==0) {
    breviary->adventblue = value;
  } else if (strcmp("rose", flag)==0) {
    breviary->rose = value;
  }
}

long
ymd_to_serial (int y, int m, int d)
{
  struct tm time;

  time.tm_sec = 0;
  time.tm_min = 0;
  time.tm_hour = 0;
  time.tm_mday = d;
  time.tm_mon = m-1; /* January is 0 */
  time.tm_year = y;
  time.tm_wday = 0;
  time.tm_yday = 0;
  time.tm_isdst = 0;

  return mktime(&time) / (60*60*24);
}

/**
 * Returns the serial number of the Sunday immediately
 * before the parameter, unless the parameter represents
 * a Sunday, in which case it is returned unchanged.
 *
 * \param serial  The serial number of a day.
 * \result        A Sunday within a week before "serial".
 */
long
rewind_to_sunday (long serial)
{
  serial += 4;
  return (serial - (serial%7)) - 4;
}

/**
 * Returns the date of Easter for the given calendar year.
 *
 * \param y  The number of years after 1900 (for
 *           example, 2010 is 110).
 * \result   The date of Easter for the given year,
 *           expressed as a number of days after
 *           1 January 1970.
 */
unsigned long
year_to_easter (int y)
{
  int c, g, k, l, i, j, m, d;

  y = y + 1900;
  c = y/100;
  g = y%19;
  k = (c-17) / 25;
  i = (c - c/4 - (c-k)/3 + 19*g + 15) % 30;
  i = i - (i/28)*(1- (i/28)*(29/(i+1))*((21-g)/11));
  j = (y + y/4 + i + 2 - c + c/4) % 7;
  l = i - j;
  m = 3 + (l+40)/44;
  d = l + 28 - 31*(m/4);

  return ymd_to_serial (y-1900, m, d);
}

/**
 * Returns the date of Advent Sunday, the first
 * day of the liturgical year, for the given calendar year.
 *
 * \param y  The number of years after 1900 (for
 *           example, 2010 is 110).
 * \result   The date of Advent Sunday for the given
 *           year, expressed as a number of days
 *           after 1 January 1970.
 */
unsigned long
year_to_advent_sunday (int y)
{
  /* Firstly, discover which day Christmas falls on */
  struct tm time;
  long result = ymd_to_serial (y, 12, 25);
  time_t christmas = result * (60*60*24);

  gmtime_r (&christmas,
	    &time);

  if (time.tm_wday==0)
    {
      /* Christmas falls on a Sunday */
      result -= 7;
    }
  else
    {
      /*
       * It was that many days after the
       * Sunday we want.
       */
      result -= time.tm_wday;
    }

  /* Now skip backwards three more weeks */
  result -= (3*7);

  return result;

}

static void
dump_date (const char* message,
	   long serial)
{
  struct tm tm;
  time_t time = serial * (24*60*60);

  gmtime_r (&time,
	    &tm);

  printf ("%s - %ld - %d-%d-%d\n",
	  message,
	  serial,
	  tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
}

void
breviary_ensure_initialised (Breviary *breviary)
{
  struct tm tm;
  long target_serial,
    easter_serial,
    advent_sunday_serial,
    week_count_start_serial,
    season_start_serial,
    sunday_before_season_start_serial;
  int season = SEASON_NONE;
  int days_after_easter;
  int days_after_week_count_start;

  if (breviary->facets[0]!=0)
    return;

#if 0
  printf ("Breviary is not initialised; doing it now.\n");
#endif

  breviary->facets[0] = 1;
  breviary->facets[1] = 1;
  breviary->facets[2] = 1;
  breviary->facets[3] = 1;
  breviary->facets[4] = 0;

  gmtime_r (&(breviary->time),
	    &tm);

  breviary->hour = tm.tm_hour;

  breviary->facets[0] =
    10100 +
    tm.tm_mon*100 +
    tm.tm_mday;

  target_serial =
    ymd_to_serial (tm.tm_year,
		   tm.tm_mon+1,
		   tm.tm_mday);

#if 0
  dump_date ("Target date",
	     target_serial);
#endif

  easter_serial = year_to_easter (tm.tm_year);

#if 0
  dump_date ("Easter",
	     easter_serial);
#endif

  days_after_easter = target_serial - easter_serial;

  breviary->facets[3] =
    40500 +
    days_after_easter;

  week_count_start_serial =
    rewind_to_sunday (ymd_to_serial (tm.tm_year,
				     1, 1));

#if 0
  dump_date ("Measuring weeks from",
	     week_count_start_serial);
#endif

  days_after_week_count_start =
    target_serial - week_count_start_serial;

  breviary->facets[1] =
    20000 +
    10 * (days_after_week_count_start/7) +
    days_after_week_count_start%7;

  advent_sunday_serial =
    year_to_advent_sunday (tm.tm_year);

#if 0
  dump_date ("Advent Sunday",
	     advent_sunday_serial);
#endif

  /* Now to calculate the season */

  if (days_after_easter > -47 &&
      days_after_easter < 0)
    {
      breviary->facets[2] =
	SEASON_LENT;
      season_start_serial =
	easter_serial - 47;
    }
  else if (days_after_easter >= 0 &&
	   days_after_easter <= 49)
    {
      /*
       * yes, this is correct: Pentecost
       * itself is in Easter season, not
       * Pentecost season
       */

      breviary->facets[2] =
	SEASON_EASTER;
      season_start_serial =
	easter_serial;
    }
  else if (target_serial >= advent_sunday_serial &&
	   breviary->facets[0] < 11225)
    {
      breviary->facets[2] =
	SEASON_ADVENT;
      season_start_serial =
	advent_sunday_serial;
    }
  else if (breviary->facets[0] >= 11225 ||
	   breviary->facets[0] <= 10105)
    {
      int year = tm.tm_year;

      if (breviary->facets[0] <= 10105)
	{
	  year--;
	}

      breviary->facets[2] =
	SEASON_CHRISTMAS;
      season_start_serial =
	ymd_to_serial (year, 12, 25);
    }
  else if (days_after_easter <= -47)
    {
      breviary->facets[2] =
	SEASON_EPIPHANY;
      season_start_serial =
	ymd_to_serial (tm.tm_year,
		       1, 6);
    }
  else
    {
      breviary->facets[2] =
	SEASON_PENTECOST;
      season_start_serial =
	easter_serial + 50;
    }

#if 0
  dump_date ("Season starts on",
	     season_start_serial);
#endif

  sunday_before_season_start_serial =
    rewind_to_sunday (season_start_serial);

  if (sunday_before_season_start_serial ==
      season_start_serial)
    {
      /*
       * If the season begins on a Sunday,
       * the first week is 1, not 0.
       */
      sunday_before_season_start_serial -= 7;
    }

  if (target_serial >= advent_sunday_serial-7 &&
      target_serial < advent_sunday_serial)
    {
      /*
       * The feast of Christ the King gets a
       * special case: it's coded as week
       * 99 of Pentecost season.  (See the
       * docs for a discussion of why.)
       */

      breviary->facets[2] += 990;
    }
  else
    {
      breviary->facets[2] += 10 *
	((target_serial - sunday_before_season_start_serial)/7);
    }

  /* And lastly, add the day of the week. */
  breviary->facets[2] +=
    days_after_week_count_start%7;

#if 0
  
  /* Year 1 or 2? */

  breviary->year1or2 = 1 + (tm.tm_year%2);
  if (target_serial >= advent_sunday_serial)
    {
      /*
       * If we're on or after Advent Sunday, we're
       * into the following year, so flip the
       * year number.
       */
      breviary->year1or2 = 3 - breviary->year1or2;
    }

  printf ("Year 1 or 2? %d\n", breviary->year1or2);

  /* Calculate the season. */
#endif
  
}

const int*
breviary_get_facets (Breviary *breviary)
{
  breviary_ensure_initialised (breviary);

  return &(breviary->facets[0]);
}

void
breviary_free (Breviary *breviary)
{
  free (breviary);
}

static void
dump_sundays (void)
{
  long current = ymd_to_serial (75, 1, 1);
  int day;

  printf ("# Dump Sundays\n");

  for (day=1; day<32; day++) {
    struct tm tm;
    char *is_sunday = "";
    time_t time;

    time = current * (60*60*24);

    gmtime_r (&time,
	      &tm);

    if (tm.tm_wday==0)
      {
	is_sunday = " (Sunday)";
      }

    printf ("%d-%02d-%02d %10ld %10ld%s\n",
	    tm.tm_year+1900,
	    tm.tm_mon+1,
	    tm.tm_mday,
	    current,
	    rewind_to_sunday (current),
	    is_sunday);

    current++;
  }
}

char *
breviary_explain_facet (int facet)
{
  const size_t size = 15;
  char *result = malloc (size);

  result[0] = 0;

  switch (facet / 10000)
    {
    case 1: /* Gregorian */
      {
	int month = (facet%10000)/100;
	int day = facet%100;
	char *month_name = NULL;
	
	switch (month)
	  {
	  case  1: month_name = "Jan"; break;
	  case  2: month_name = "Feb"; break;
	  case  3: month_name = "Mar"; break;
	  case  4: month_name = "Apr"; break;
	  case  5: month_name = "May"; break;
	  case  6: month_name = "Jun"; break;
	  case  7: month_name = "Jul"; break;
	  case  8: month_name = "Aug"; break;
	  case  9: month_name = "Sep"; break;
	  case 10: month_name = "Oct"; break;
	  case 11: month_name = "Nov"; break;
	  case 12: month_name = "Dec"; break;
	  }

	if (month_name)
	  {
	    snprintf (result, size,
		      "%s %2d",
		      month_name, day);
	  }
      }
      break;

    case 2: /* Week number */
      {
	int week = (facet-20000) / 10;
	int day = facet % 10;
	char* day_name = NULL;

	switch (day)
	  {
	  case  0: day_name = "Sun"; break;
	  case  1: day_name = "Mon"; break;
	  case  2: day_name = "Tue"; break;
	  case  3: day_name = "Wed"; break;
	  case  4: day_name = "Thu"; break;
	  case  5: day_name = "Fri"; break;
	  case  6: day_name = "Sat"; break;
	  }

	if (day_name)
	  {
	    snprintf (result, size,
		      "%s, week %d",
		      day_name,
		      week);
	  }
      }
      break;

    case 3: /* Season */
      {
	int season = (facet / 1000) * 1000;
	int week = (facet/10) % 100;
	char *season_name = NULL;

	switch (season)
	  {
	  case SEASON_ADVENT:
	    season_name = "Adve";
	    break;

	  case SEASON_CHRISTMAS:
	    season_name = "Chri";
	    break;

	  case SEASON_EPIPHANY:
	    season_name = "Ephi";
	    break;

	  case SEASON_LENT:
	    season_name = "Lent";
	    break;

	  case SEASON_EASTER:
	    season_name = "East";
	    break;

	  case SEASON_PENTECOST:
	    season_name = "Pent";
	    break;
	  }

	if (season_name)
	  {
	    snprintf (result, size,
		      "%s %d",
		      season_name, week);
	  }
      }
      break;

    case 4: /* Easter */
      {
	int value = facet - 40500;
	char sign = '+';

	if (value<0)
	  {
	    value = -value;
	    sign = '-';
	  }

	snprintf (result, size,
		  "Easter%c%03d",
		  sign, value);
      }
      break;
    }

  if (result[0]==0)
    {
      snprintf (result, size,
		"Unknown");
    }

  return result;
}

void
breviary_free_string (char *string)
{
  if (string==NULL)
    {
      return;
    }

  free (string);
}

static void
dump_facets (void)
{
  long current = ymd_to_serial (75, 1, 1);
  int i;
  const int *cursor;
  Breviary *breviary;

  printf ("# Dump facets\n");

  for (i=0; i<365; i++) {
    struct tm tm;
    time_t time;

    time = current * (60*60*24);

    gmtime_r (&time,
	      &tm);

    printf ("%d-%02d-%02d",
	    tm.tm_year+1900,
	    tm.tm_mon+1,
	    tm.tm_mday);

    breviary = breviary_new (time);

    cursor = breviary_get_facets (breviary);

    while (*cursor)
      {
	char *explanation =
	  breviary_explain_facet (*cursor);
	printf (" %d %-15s", *cursor,
		explanation);
	breviary_free_string (explanation);
	cursor++;
      }

    breviary_free (breviary);

    printf ("\n");

    current++;
  }

}

int
breviary_test_dump_feast (unsigned int feast)
{
  int year;
  struct tm tm;

  switch (feast)
    {
    case 1:
      dump_sundays ();
      return;

    case 2:
      dump_facets ();
      return;

    case 3:
    case 4:
      break;

    default:
      printf ("# Unknown request %d\n", feast);
      return;
    }

  for (year=70; year<=120; year++)
    {
      int date;
      time_t time;

      if (feast==3)
	date = year_to_easter (year);
      else
	date = year_to_advent_sunday (year);

      time = (time_t) date * (24*60*60);

      gmtime_r (&time,
		&tm);

      printf ("%d-%d-%d\n",
	      tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
    }
}

/* EOF calendar.c */
