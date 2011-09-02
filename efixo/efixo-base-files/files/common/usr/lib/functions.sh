#! /bin/sh
#
# Anthony Viallard - oct. 2009
#
# mutex functions is based on gch idea. Thanks to him :)
#

mutex_lock()
{
    mutex="$1"

    while ! (set -C; echo $$ > "$mutex") > /dev/null 2>&1; do
        if test -n `cat "$mutex"` \
	    && ! kill -0 `cat "$mutex"` > /dev/null 2>&1; then
	    rm -f "$mutex"
	elif test `cat "$mutex"` = $$; then
	    logger "mutex_lock: we (pid $$) already own mutex $mutex"
	fi
        usleep 100000
    done
}

mutex_unlock()
{
    mutex="$1"

    if test -e "$mutex" && test $$ = `cat "$mutex"`; then
	rm -f "$mutex"
	return 0
    fi

    if ! test -e "$mutex"; then
	logger "mutex_unlock: mutex $mutex does not exist"
	return 1
    fi

    logger "mutex_unlock: we (pid $$) do not own mutex $mutex (owned by `cat "$mutex"`)"
    return 1
}
