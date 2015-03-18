#!/bin/bash

# TODO only one common code needed, we will delete each common code
# in each components...
protoc --cpp_out="../services_2.0/" CommonUserCenter.proto
if [ $? -eq 0 ]; then
  echo "CommonUserCenter proto IDL generation [OK]"
else
  echo "CommonUserCenter proto IDL generation [**failed]"
fi

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

# For user authentication
protoc --cpp_out="../services_2.0/user_auth/" UserAuth.proto
if [ $? -eq 0 ]; then
  echo "User Auth proto IDL generation [OK]"
else
  echo "User Auth proto IDL generation [**failed]"
fi

# For user activation
protoc --cpp_out="../services_2.0/user_activate/" UserActivation.proto
if [ $? -eq 0 ]; then
  echo "User Activation proto IDL generation [OK]"
else
  echo "User Activation proto IDL generation [**failed]"
fi

# For opensips authentication
protoc --cpp_out="../services_2.0/opensips_account/" SipAccount.proto
if [ $? -eq 0 ]; then
  echo "OpenSIPS auth IDL generation [OK]"
else
  echo "OpenSIPS auth IDL generation [**failed]"
fi

# For Password Manager
protoc --cpp_out="../services_2.0/password_manager/" PasswordManager.proto
if [ $? -eq 0 ]; then
  echo "PasswordMgr IDL generation [OK]"
else
  echo "PasswordMgr IDL generation [**failed]"
fi

# For Attribute Modification
protoc --cpp_out="../services_2.0/attribute_modification/" AttributeModify.proto
if [ $? -eq 0 ]; then
  echo "Attribute IDL generation [OK]"
else
  echo "Attribute IDL generation [**failed]"
fi

#mv *.pb.*  $(NETDISK_DIR)
#protoc --java_out="/home/yangsongxiang/server/backend/services/netdisk/test/" NetdiskMessage.proto
#mv *.java $(NETDISK_DIR)test/com/caredear
