#include <stdio.h>
#include <glob.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "files.h"

typedef struct _LoadIntoDatabaseContext {
  char *filename;
  int line_n;
  char *query;
  int fields;
  char **values;
  int value_n;
  sqlite3 *db;
  sqlite3_stmt *statement;
} LoadIntoDatabaseContext;

static void
error (LoadIntoDatabaseContext *context,
       const char *message)
{
  if (context)
    {
      fprintf (stderr,
	       "\n%s:%d: Error: %s\n",
	       context->filename,
	       context->line_n,
	       message);
    }
  else
    {
      fprintf (stderr,
	       "\nError: %s\n",
	       message);
    }

  /*
    maybe we should delete half-finished dbs?
  */

  exit (255);
}

static void
submit (LoadIntoDatabaseContext *context)
{
  char **cursor = context->values;
  int i;

  printf ("Submitting query: %s\n",
	  context->query);

  for (i=0; i<context->fields; i++)
    {
      int is_number = 0;
      char *value = context->values[i];

      printf ("Value %d: %s\n",
	      i+1,
	      value);

      if (value && value[0])
	{
	  char *cursor = value;
	  while (*cursor>='0' &&
		 *cursor<='9')
	    {
	      cursor++;
	    }

	  if (*cursor==0)
	    {
	      is_number = 1;
	    }
	}

      if (is_number)
	{
	  sqlite3_bind_int (context->statement,
			    i+1,
			    atoi (value));
	}
      else
	{
	  sqlite3_bind_text (context->statement,
			     i+1,
			     context->values[i],
			     -1, NULL);
	}
    }

  sqlite3_step (context->statement);

  if (sqlite3_reset (context->statement) != SQLITE_OK)
    {
      error (context,
	     sqlite3_errmsg (context->db));
    }

  for (i=0; i<context->fields; i++)
    {
      free (context->values[i]);
    }

  context->value_n = 0;
}

static void
load_into_database_cb (const char *field,
		       const char *value,
		       void *user_data)
{
  LoadIntoDatabaseContext *context =
    (LoadIntoDatabaseContext*) user_data;
  
  context->line_n ++;

  if (context->db==NULL)
    {
      if (sqlite3_open_v2("db/breviary.db",
			  &(context->db),
			  SQLITE_OPEN_READWRITE |
			  SQLITE_OPEN_CREATE,
			  NULL) != SQLITE_OK)
	{
	  error (context,
		 sqlite3_errmsg (context->db));
	}
    }

  if (field==NULL)
    {

      if (value==NULL)
	{
	  /* we're all done */

	  if (context->value_n==context->fields)
	    {
	      submit (context);
	    }
	  else if (context->value_n!=0)
	    {
	      error (context,
		     "File ended partway through a record");
	    }

	  if (context->values)
	    free (context->values);

	  if (context->query)
	    free (context->query);

	  if (context->filename)
	    free (context->filename);
	}
      else
	{
	  if (context->value_n==context->fields)
	    {
	      if (strcmp (value, "") != 0)
		{
		  error (context,
			 "File is out of sync\n");
		  exit (255);
		}

	      submit (context);
	    }
	  else
	    {
	      context->values[context->value_n] =
		strdup (value);

	      context->value_n ++;
	    }
	}
    }
  else if (strcmp (field, "Setup")==0)
    {
      char *sqlite_error = NULL;

      printf ("Submitting SQL query:\n%s\n",
	      value);

      sqlite3_exec(context->db,
		   value,
		   NULL, NULL, &sqlite_error);

      if (sqlite_error)
	{
	  error (context,
		 sqlite_error);
	  /* no need to free it, since this terminates */
	}
      
    }
  else if (strcmp (field, "Query")==0)
    {
      if (context->query)
	{
	  error (context, 
		 "Query defined multiply");
	}
      else
	{
	  const char *cursor = value;
	  context->query = strdup (value);
	 
	  /* how many spaces do we need to fill in? */
	  while (*cursor) {

	    if (*cursor=='?')
	      {
		context->fields++;
	      }

	    cursor++;
	  }

	  /* reserve space for them */
	  context->values = malloc (sizeof(char*)*context->fields);

	  if (sqlite3_prepare (context->db,
			       value,
			       -1,
			       &(context->statement),
			       NULL) != SQLITE_OK)
	    {
	      error (context,
		     sqlite3_errmsg (context->db));
	    }
	}
    }
}

static void
load_into_database (char *filename)
{
  LoadIntoDatabaseContext context;

  context.query = NULL;
  context.fields = 0;
  context.values = NULL;
  context.value_n = 0;
  context.filename = strdup (filename);
  context.line_n = 0;
  context.db = NULL;
  context.statement = NULL;

  parse_rfc822 (filename,
		load_into_database_cb,
		&context);

  printf ("Closing sqlite.\n");

  if (context.statement)
    {
      if (sqlite3_finalize (context.statement)!=SQLITE_OK)
	{
	  error (&context,
		 sqlite3_errmsg (context.db));
	}
    }

  if (sqlite3_close (context.db)!=SQLITE_OK)
    {
      error (&context,
	     sqlite3_errmsg (context.db));
    }
}

static void
scan_for_files (void)
{
  glob_t globspace;

  switch (glob ("data/*.db.txt",
		0,
		NULL,
		&globspace))
    {
    case 0:
      {
	int i;

	for (i=0; i<globspace.gl_pathc; i++)
	  {
	    printf ("%s ... ",
		    globspace.gl_pathv[i]);

	    load_into_database (globspace.gl_pathv[i]);

	    printf ("done\n");
	  }

	globfree (&globspace);
      }
      break;

    case GLOB_NOSPACE:
      fprintf (stderr,
	       "Out of space\n");
      break;

    case GLOB_ABORTED:
      fprintf (stderr,
	       "Aborted\n");
      break;

    default:
    case GLOB_NOMATCH:
      fprintf (stderr,
	       "Could not find files to build database\n");
      break;
    }
}

static void
check_up_to_date (void)
{
  struct stat details;

  if (stat ("db", &details)==0)
    {
      if (!(details.st_mode & S_IFDIR))
	{
	  error (NULL,
		 "db exists but is not a directory; "
		 "please remove it");
	}

      /* otherwise we're all good */
    }
  else
    {
      if (mkdir ("db", 0777)!=0)
	{
	  error (NULL,
		 "Failed to create 'db' directory");
	}
    }

  if (stat ("db/breviary.db", &details)!=0)
    {
      switch (errno)
	{
	case ENOENT:
	  /* file not found, which is all good */
	  return;

	default:
	  error (NULL,
		 strerror (errno));
	}
    }

  error (NULL,
	 "breviary.db already exists (and we "
	 "haven't implemented up-to-date checks "
	 "yet)");
}

int
main (int argc, char **argv)
{
  go_to_root ();

  check_up_to_date ();

  scan_for_files ();
}

/* eof breviary-build-db.c */
