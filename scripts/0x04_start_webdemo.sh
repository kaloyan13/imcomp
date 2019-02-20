#!/bin/sh

if ! screen -list | grep -q "imcomp_webdemo"; then
  echo "Starting imcomp_webdemo ..."
  screen -dmS imcomp_webdemo ./0x04_start_imcomp.sh
else
  echo "imcomp_webdemo already started!"
fi

