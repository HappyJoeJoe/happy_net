#!/bin/bash

protoc --cpp_out=. person.proto

cp *.h *.cc ../src