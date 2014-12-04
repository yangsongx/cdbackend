#!/bin/bash

# TODO  below script not working yet!
NETDISK_DIR=../servicees/netdisk/

protoc --cpp_out="${NETDISK_DIR}" NetdiskMessage.proto
protoc --java_out="/home/yangsongxiang/server/backend/services/netdisk/test/" NetdiskMessage.proto
