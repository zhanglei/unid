dnl $Id$
dnl config.m4 for extension unid

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(unid, for unid support,
dnl Make sure that the comment is aligned:
dnl [  --with-unid             Include unid support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(unid, whether to enable unid support,
dnl Make sure that the comment is aligned:
[  --enable-unid           Enable unid support])

if test "$PHP_UNID" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-unid -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/unid.h"  # you most likely want to change this
  dnl if test -r $PHP_UNID/$SEARCH_FOR; then # path given as parameter
  dnl   UNID_DIR=$PHP_UNID
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for unid files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       UNID_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$UNID_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the unid distribution])
  dnl fi

  dnl # --with-unid -> add include path
  dnl PHP_ADD_INCLUDE($UNID_DIR/include)

  dnl # --with-unid -> check for lib and symbol presence
  dnl LIBNAME=unid # you may want to change this
  dnl LIBSYMBOL=unid # you most likely want to change this

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $UNID_DIR/$PHP_LIBDIR, UNID_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_UNIDLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong unid lib version or lib not found])
  dnl ],[
  dnl   -L$UNID_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(UNID_SHARED_LIBADD)

  PHP_NEW_EXTENSION(unid, unid.c shm.c, $ext_shared)
fi
