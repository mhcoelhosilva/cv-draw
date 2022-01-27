#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/matheussilva/dev/other/projs/fd/build
  make -f /Users/matheussilva/dev/other/projs/fd/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/matheussilva/dev/other/projs/fd/build
  make -f /Users/matheussilva/dev/other/projs/fd/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/matheussilva/dev/other/projs/fd/build
  make -f /Users/matheussilva/dev/other/projs/fd/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/matheussilva/dev/other/projs/fd/build
  make -f /Users/matheussilva/dev/other/projs/fd/build/CMakeScripts/ReRunCMake.make
fi

