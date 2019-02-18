#!/bin/bash

protoc --cpp_out=. request.proto

cp *.h *.cc ../src