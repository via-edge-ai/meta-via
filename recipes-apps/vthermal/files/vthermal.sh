#!/bin/sh
 
if [ -f "/data/vthermal" ]; then
    echo "Run /data/vthermal"
    /data/vthermal $1
else
    /usr/bin/vthermal $1
fi
