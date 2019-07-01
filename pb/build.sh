#!/bin/bash

protoc --cpp_out=. comm_basic.proto

cp *.h *.cc ../src