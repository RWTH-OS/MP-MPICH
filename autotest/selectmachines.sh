#!/bin/bash

TEMPLATE=~/.machines
VALID=~/.valid.machines
FLAG=FALSE

rm -f $VALID 2>/dev/null

for NODE in `cat $TEMPLATE` ; do
  OSINFO=`telnet $NODE osinfo 2>/dev/null | grep "Linux"`
  if [ -n "$OSINFO" ] ; then
      echo $NODE >> "$VALID"
      FLAG=TRUE
  fi
done

if [ $FLAG = "FALSE" ] ; then
    echo `hostname` >> "$VALID"
fi
