 #!/bin/bash

killall ser

valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./ser 
