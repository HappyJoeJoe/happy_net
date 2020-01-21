 #!/bin/bash

killall ser

valgrind --tool=memcheck --leak-check=full -q ./ser 

# valgrind --tool=massif ./ser