dnl Process this file with autoconf to produce a configure script.
AC_INIT(breviary, 0.01, thomas@thurman.org.uk)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(breviary/calendar.c)

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
AM_PROG_CC_C_O

PKG_CHECK_MODULES([SQLITE], [sqlite3])
AC_SUBST([SQLITE_CFLAGS])
AC_SUBST([SQLITE_LIBS])

AC_OUTPUT(Makefile breviary/Makefile)
