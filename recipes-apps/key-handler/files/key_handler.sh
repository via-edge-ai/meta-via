#!/bin/sh
 
if [ -f "/data/key_handler" ]; then
    echo "Run /data/key_handler"
    /data/key_handler $1
else
    /usr/bin/key_handler $1
fi
