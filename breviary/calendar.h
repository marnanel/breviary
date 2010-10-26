#ifndef CALENDAR_H
#define CALENDAR_H 1

#include <time.h>

typedef struct _Breviary Breviary;

/**
 * The seasons of the church year.  The quirky
 * numbering is in order to fit in with the
 * facets system.
 */
typedef enum _Season {
  SEASON_NONE = 0,
  SEASON_ADVENT = 31000,
  SEASON_CHRISTMAS = 32000,
  SEASON_EPIPHANY = 33000,
  SEASON_LENT = 34000,
  SEASON_EASTER = 35000,
  SEASON_PENTECOST = 36000,
} Season;

struct _Breviary* breviary_new (time_t when);

void breviary_set_flag (Breviary *breviary,
			const char *flag,
			int value);

const int* breviary_get_facets (Breviary *breviary);

char *breviary_explain_facet (int facet);

void breviary_free_string (char *string);

void breviary_free (Breviary *breviary);

/**
 * Dumps the date of a given movable feast to
 * standard output.  Used for testing.  This
 * function has no access to the feasts
 * table, and only knows about two feasts.
 *
 * \param feast  The feast we want.  (Document
 *               this further!)
 */
int breviary_test_dump_feast (unsigned int feast);

#endif /* !CALENDAR_H */
