#!/bin/sh

# This is a generic script for firing up a server, waiting for it to write
# its stringified IOR to a file, then firing up a server

[ -z "$USER" ] && USER=`id -un`

## Disable core dumps because bonobo-activation-empty-server is aborting with the error:
##   "This process has not registered the required OAFIID your source
##   code should register '%s'. If your code is performing delayed
##   registration and this message is trapped in error, see
##   bonobo_activation_i"...
## And the core file is making distcheck fail.
## TODO: Check why is bonobo-activation-empty-server aborting.

ulimit -c 0

eval $(dbus-launch --sh-syntax)

if test "z$ORBIT_TMPDIR" = "z"; then
	ORBIT_TMPDIR="/tmp/orbit-$USER/tst"
	rm -Rf $ORBIT_TMPDIR
	mkdir -p $ORBIT_TMPDIR
fi
TMPDIR=$ORBIT_TMPDIR;
export TMPDIR;

BONOBO_ACTIVATION_SERVER="../../activation-server/bonobo-activation-server";
BONOBO_ACTIVATION_DEBUG_OUTPUT="1";
PATH=".:./.libs:$PATH";
LD_LIBRARY_PATH="./.libs:$LD_LIBRARY_PATH";

export BONOBO_ACTIVATION_SERVER BONOBO_ACTIVATION_DEBUG_OUTPUT BONOBO_ACTIVATION_PATH PATH LD_LIBRARY_PATH

if ./bonobo-activation-test; then
    kill -15 $DBUS_SESSION_BUS_PID
    exit 0;
else
    kill -15 $DBUS_SESSION_BUS_PID
    exit 1;
fi
