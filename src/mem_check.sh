 #!/bin/bash

killall ser

valgrind --tool=memcheck --leak-check=full -v ./ser 