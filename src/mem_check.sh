 #!/bin/bash

killall ser

valgrind --tool=memcheck --leak-check=full --show-reachable=yes -q ./ser 
