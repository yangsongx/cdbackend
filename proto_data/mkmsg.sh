#!/bin/bash

# TODO  below script not working yet!
NETDISK_DIR="../servicees/netdisk/"

protoc --cpp_out=. NetdiskMessage.proto
mv *.pb.*  $(NETDISK_DIR)
protoc --java_out="/home/yangsongxiang/server/backend/services/netdisk/test/" NetdiskMessage.proto
mv *.java $(NETDISK_DIR)test/com/caredear
