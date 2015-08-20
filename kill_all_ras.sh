# ipcs -m | grep -v 0x00000000 | egrep '^0' | awk '{print "-m" $2}' | xargs ipcrm
ps aux | grep "/home/yi/network_prog/NP-Project2-fork/ras.o" | awk '{print $2}' | xargs kill
