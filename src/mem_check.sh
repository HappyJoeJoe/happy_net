#!/bin/bash

killall ser

valgrind --tool=memcheck ./ser