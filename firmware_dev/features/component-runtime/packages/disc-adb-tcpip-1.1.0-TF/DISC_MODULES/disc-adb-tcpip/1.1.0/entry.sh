#!/bin/sh
# Legacy r16 starts this once as a service; upgraded runtime integrates in-process.
unset LD_PRELOAD
exec sh ./bootstrap.sh
