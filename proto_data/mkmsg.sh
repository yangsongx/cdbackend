#!/bin/bash

# For user registeration
protoc --cpp_out="../services_2.0/user_reg/" UserRegister.proto
if [ $? -eq 0 ]; then
  echo "User Reg proto IDL generation [OK]"
else
  echo "User Reg proto IDL generation [**failed]"
fi

# For user login
protoc --cpp_out="../services_2.0/user_login/" UserLogin.proto
if [ $? -eq 0 ]; then
  echo "User Login proto IDL generation [OK]"
else
  echo "User Login proto IDL generation [**failed]"
fi

# For opensips authentication
protoc --cpp_out="../services_2.0/opensips_account/" SipAccount.proto
if [ $? -eq 0 ]; then
  echo "OpenSIPS auth IDL generation [OK]"
else
  echo "OpenSIPS auth IDL generation [**failed]"
fi
#mv *.pb.*  $(NETDISK_DIR)
#protoc --java_out="/home/yangsongxiang/server/backend/services/netdisk/test/" NetdiskMessage.proto
#mv *.java $(NETDISK_DIR)test/com/caredear
