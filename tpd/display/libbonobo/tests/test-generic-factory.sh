#! /bin/sh

if test "z$ORBIT_TMPDIR" = "z"; then
	ORBIT_TMPDIR="/tmp/orbit-$USER/tst"
	rm -Rf $ORBIT_TMPDIR
	mkdir -p $ORBIT_TMPDIR
fi
TMPDIR=$ORBIT_TMPDIR;
export TMPDIR;

BONOBO_ACTIVATION_SERVER="../activation-server/bonobo-activation-server";
PATH=".:$PATH";
LD_LIBRARY_PATH="./.libs:$LD_LIBRARY_PATH";
unset BONOBO_ACTIVATION_DEBUG_OUTPUT
PATH="./.libs:$PATH";

export BONOBO_ACTIVATION_SERVER PATH LD_LIBRARY_PATH

# job control must be active
set -m

echo "Starting factory"
./generic-factory | tr -d '\015' > generic-factory.output &
sleep 1

echo "Starting client"
./test-generic-factory | tr -d '\015' > test-generic-factory.output

echo "Waiting for factory to terminate; Please hold on a second, otherwise hit Ctrl-C."
wait %1 2> /dev/null

echo "Comparing factory output with model..."
if diff -u $MODELS_DIR/generic-factory.output generic-factory.output; then
    echo "...OK"
    rm -f generic-factory.output
else
    echo "...DIFFERENT!"
    rm -f generic-factory.output
    rm -f test-generic-factory.output
    exit 1;
fi

echo "Comparing client output with model..."
if diff -u $MODELS_DIR/test-generic-factory.output test-generic-factory.output; then
    echo "...OK"
    rm -f test-generic-factory.output
else
    echo "...DIFFERENT!"
    rm -f test-generic-factory.output
    exit 1;
fi

exit 0

