ipcs -m | grep -v 0x00000000 | egrep '^0' | awk '{print "-m" $2}' | xargs ipcrm
