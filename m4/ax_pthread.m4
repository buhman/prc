# http://www.gnu.org/software/autoconf-archive/ax_pthread.html

#serial 21

AU_ALIAS([ACX_PTHREAD], [AX_PTHREAD])
AC_DEFUN([AX_PTHREAD], [
AC_REQUIRE([AC_CANONICAL_HOST])
AC_LANG_PUSH([C])
ax_pthread_ok=no

if test x"$PTHREAD_LIBS$PTHREAD_CFLAGS" != x; then
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        AC_MSG_CHECKING([for pthread_join in LIBS=$PTHREAD_LIBS with CFLAGS=$PTHREAD_CFLAGS])
        AC_TRY_LINK_FUNC([pthread_join], [ax_pthread_ok=yes])
        AC_MSG_RESULT([$ax_pthread_ok])
        if test x"$ax_pthread_ok" = xno; then
                PTHREAD_LIBS=""
                PTHREAD_CFLAGS=""
        fi
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
fi

ax_pthread_flags="pthreads none -Kthread -kthread lthread -pthread -pthreads -mthreads pthread --thread-safe -mt pthread-config"

case ${host_os} in
        solaris*)

        ax_pthread_flags="-pthreads pthread -mt -pthread $ax_pthread_flags"
        ;;

        darwin*)
        ax_pthread_flags="-pthread $ax_pthread_flags"
        ;;
esac

AC_MSG_CHECKING([if compiler needs -Werror to reject unknown flags])
save_CFLAGS="$CFLAGS"
ax_pthread_extra_flags="-Werror"
CFLAGS="$CFLAGS $ax_pthread_extra_flags -Wunknown-warning-option -Wsizeof-array-argument"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([int foo(void);],[foo()])],
                  [AC_MSG_RESULT([yes])],
                  [ax_pthread_extra_flags=
                   AC_MSG_RESULT([no])])
CFLAGS="$save_CFLAGS"

if test x"$ax_pthread_ok" = xno; then
for flag in $ax_pthread_flags; do

        case $flag in
                none)
                AC_MSG_CHECKING([whether pthreads work without any flags])
                ;;

                -*)
                AC_MSG_CHECKING([whether pthreads work with $flag])
                PTHREAD_CFLAGS="$flag"
                ;;

                pthread-config)
                AC_CHECK_PROG([ax_pthread_config], [pthread-config], [yes], [no])
                if test x"$ax_pthread_config" = xno; then continue; fi
                PTHREAD_CFLAGS="`pthread-config --cflags`"
                PTHREAD_LIBS="`pthread-config --ldflags` `pthread-config --libs`"
                ;;

                *)
                AC_MSG_CHECKING([for the pthreads library -l$flag])
                PTHREAD_LIBS="-l$flag"
                ;;
        esac

        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        LIBS="$PTHREAD_LIBS $LIBS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS $ax_pthread_extra_flags"

        AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <pthread.h>
                        static void routine(void *a) { a = 0; }
                        static void *start_routine(void *a) { return a; }],
                       [pthread_t th; pthread_attr_t attr;
                        pthread_create(&th, 0, start_routine, 0);
                        pthread_join(th, 0);
                        pthread_attr_init(&attr);
                        pthread_cleanup_push(routine, 0);
                        pthread_cleanup_pop(0) /* ; */])],
                [ax_pthread_ok=yes],
                [])

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        AC_MSG_RESULT([$ax_pthread_ok])
        if test "x$ax_pthread_ok" = xyes; then
                break;
        fi

        PTHREAD_LIBS=""
        PTHREAD_CFLAGS=""
done
fi

# Various other checks:
if test "x$ax_pthread_ok" = xyes; then
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Detect AIX lossage: JOINABLE attribute is called UNDETACHED.
        AC_MSG_CHECKING([for joinable pthread attribute])
        attr_name=unknown
        for attr in PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_UNDETACHED; do
            AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <pthread.h>],
                           [int attr = $attr; return attr /* ; */])],
                [attr_name=$attr; break],
                [])
        done
        AC_MSG_RESULT([$attr_name])
        if test "$attr_name" != PTHREAD_CREATE_JOINABLE; then
            AC_DEFINE_UNQUOTED([PTHREAD_CREATE_JOINABLE], [$attr_name],
                               [Define to necessary symbol if this constant
                                uses a non-standard name on your system.])
        fi

        AC_MSG_CHECKING([if more special flags are required for pthreads])
        flag=no
        case ${host_os} in
            aix* | freebsd* | darwin*) flag="-D_THREAD_SAFE";;
            osf* | hpux*) flag="-D_REENTRANT";;
            solaris*)
            if test "$GCC" = "yes"; then
                flag="-D_REENTRANT"
            else
                # TODO: What about Clang on Solaris?
                flag="-mt -D_REENTRANT"
            fi
            ;;
        esac
        AC_MSG_RESULT([$flag])
        if test "x$flag" != xno; then
            PTHREAD_CFLAGS="$flag $PTHREAD_CFLAGS"
        fi

        AC_CACHE_CHECK([for PTHREAD_PRIO_INHERIT],
            [ax_cv_PTHREAD_PRIO_INHERIT], [
                AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <pthread.h>]],
                                                [[int i = PTHREAD_PRIO_INHERIT;]])],
                    [ax_cv_PTHREAD_PRIO_INHERIT=yes],
                    [ax_cv_PTHREAD_PRIO_INHERIT=no])
            ])
        AS_IF([test "x$ax_cv_PTHREAD_PRIO_INHERIT" = "xyes"],
            [AC_DEFINE([HAVE_PTHREAD_PRIO_INHERIT], [1], [Have PTHREAD_PRIO_INHERIT.])])

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        # More AIX lossage: compile with *_r variant
        if test "x$GCC" != xyes; then
            case $host_os in
                aix*)
                AS_CASE(["x/$CC"],
                  [x*/c89|x*/c89_128|x*/c99|x*/c99_128|x*/cc|x*/cc128|x*/xlc|x*/xlc_v6|x*/xlc128|x*/xlc128_v6],
                  [#handle absolute path differently from PATH based program lookup
                   AS_CASE(["x$CC"],
                     [x/*],
                     [AS_IF([AS_EXECUTABLE_P([${CC}_r])],[PTHREAD_CC="${CC}_r"])],
                     [AC_CHECK_PROGS([PTHREAD_CC],[${CC}_r],[$CC])])])
                ;;
            esac
        fi
fi

test -n "$PTHREAD_CC" || PTHREAD_CC="$CC"

AC_SUBST([PTHREAD_LIBS])
AC_SUBST([PTHREAD_CFLAGS])
AC_SUBST([PTHREAD_CC])

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$ax_pthread_ok" = xyes; then
        ifelse([$1],,[AC_DEFINE([HAVE_PTHREAD],[1],[Define if you have POSIX threads libraries and header files.])],[$1])
        :
else
        ax_pthread_ok=no
        $2
fi
AC_LANG_POP
])dnl AX_PTHREAD
