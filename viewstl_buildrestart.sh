#!/bin/bash

trap "pkill viewstl; exit 0" INT

while true;
do
    ./viewstl "${@:1}" &
    inotifywait -e delete_self -e attrib ./viewstl
    kill $(pidof ./viewstl "${@:1}")
    echo "Rebuild detected on $(date "+%H:%M:%S"). Restarting..."
    sleep 0.5
done
