#!/bin/bash

# MUST NOT called directly, should embeded in Makefile
#

function pre_init()
{
    rm -rf native/
    mkdir -p native
    mkdir -p native/dep/
    cp 3rd_party/prebuilt/*.so native/dep/
}

# FIXME - do we need the .so or the .deb ?
function pre_baseso()
{
    echo -n "prepare the base SO..."
    cp common/cds_base/libcds.so native/
    cp common/cds_base/cds_cfg.xml native/
    echo "\n the base so library creation [OK]"
}

function pre_user_reg()
{
    mkdir -p native/user_reg
    echo "#!/bin/bash"> native/user_reg/run
    echo "./urs -u root -vvv -p 13000 >reg.log" >> native/user_reg/run
    chmod a+x native/user_reg/run

    cp services_2.0/user_reg/urs native/user_reg/
}

function pre_user_login()
{
    mkdir -p native/user_login
    echo "#!/bin/bash"> native/user_login/run
    echo "./uls -u root -vvv -p 13001 >login.log" >> native/user_login/run
    chmod a+x native/user_login/run

    cp services_2.0/user_login/uls native/user_login/
}

function pre_user_activate()
{
    mkdir -p native/user_activate
    echo "#!/bin/bash"> native/user_activate/run
    echo "./acts -u root -vvv -p 13002 >activation.log" >> native/user_activate/run
    chmod a+x native/user_activate/run

    cp services_2.0/user_activate/acts native/user_activate/
}

function pre_user_auth()
{
    mkdir -p native/user_auth
    echo "#!/bin/bash"> native/user_auth/run
    echo "./uauth -u root -vvv -p 13003 >user_authentication.log" >> native/user_auth/run
    chmod a+x native/user_auth/run

    cp services_2.0/user_auth/uauth native/user_auth/
}

function pre_user_passwd()
{
    mkdir -p native/password_manager/
    echo "#!/bin/bash"> native/password_manager/run
    echo "./passwdmgr -u root -vvv -p 13004 >passwd.log" >> native/password_manager/run
    chmod a+x native/password_manager/run

    cp services_2.0/password_manager/passwdmgr native/password_manager/
}

function pre_user_attr()
{
    mkdir -p native/attribute_modification/
    echo "#!/bin/bash"> native/attribute_modification/run
    echo "./attrmodify -u root -vvv -p 13005 >attr.log" >> native/attribute_modification/run
    chmod a+x native/attribute_modification/run

    cp services_2.0/attribute_modification/attrmodify native/attribute_modification/
}

function pre_update_profile()
{
    mkdir -p native/update_profile/
    echo "#!/bin/bash"> native/update_profile/run
    echo "./upusr -u root -vvv -p 13006 >up.log" >> native/update_profile/run
    chmod a+x native/update_profile/run

    cp services_2.0/update_profile/upusr native/update_profile/
}

function pre_verifycode()
{
    mkdir -p native/verify_code/
    echo "#!/bin/bash"> native/verify_code/run
    echo "./vcs -u root -vvv -p 13007 >vc.log" >> native/verify_code/run
    chmod a+x native/verify_code/run

    cp services_2.0/verify_code/vcs native/verify_code/
}

function pre_sips()
{
    mkdir -p native/opensips_account/
    echo "#!/bin/bash"> native/opensips_account/run
    echo "./opas -u root -vvv -p 12002 >vc.log" >> native/opensips_account/run
    chmod a+x native/opensips_account/run

    cp services_2.0/opensips_account/opas native/opensips_account/
}

function pre_netdisk()
{
    mkdir -p native/netdisk/
    echo "#!/bin/bash"> native/netdisk/run
    echo "./nds -u root -vvv -p 12001 >nds.log" >> native/netdisk/run
    chmod a+x native/netdisk/run

    cp services_2.0/netdisk/nds native/netdisk/
}

function gen_package_sh()
{
    # create start and stop script
    echo "#!/bin/bash"> native/startall
    echo "svscan . &" >> native/startall
    chmod a+x native/startall

    echo "#!/bin/bash" > native/stopall
    echo "for file in ./*">>native/stopall
    echo "do">>native/stopall
    echo "  if test -d \$file">>native/stopall
    echo "  then">>native/stopall
    echo "    echo \"dir is \$file\"">>native/stopall
    echo "  fi">>native/stopall
    echo "done">>native/stopall
    chmod a+x native/stopall


    echo "#!/bin/bash" > backend.sh
    echo "#DO NOT MODIFY this script, it is auto-generated">>backend.sh
    echo "echo \"Will deploy the daemon services... \"" >> backend.sh
    echo "function extract() {">>backend.sh
    echo "  uudecode \$0">>backend.sh
    echo "  tar -zxvf t">>backend.sh
    echo "  cp native/dep/*.so /usr/lib/">>backend.sh
    echo "">>backend.sh
    echo "  if [ -n \"\$NEEDSO\" ]; then" >> backend.sh
    echo "    echo \"You need libcds.so\"">>backend.sh
    echo "    cp native/libcds.so /usr/lib/">>backend.sh
    echo "  else">>backend.sh
    echo "    echo \"You don't need libcds.so\"">>backend.sh
    echo "  fi">>backend.sh
    echo "">>backend.sh
    echo "  if [ -d \"/opt/native\" ]; then">>backend.sh
    echo "    cp -rf native/* /opt/native/">>backend.sh
    echo "  else">>backend.sh
    echo "    mv native/ /opt">>backend.sh
    echo "  fi">>backend.sh
    echo "">>backend.sh
    echo "  #rm native/libcds.so">>backend.sh
    echo "  #rm native/cds_cfg.xml">>backend.sh
    echo "  /sbin/ldconfig &">>backend.sh
    echo "  rm t">>backend.sh
    echo "}">>backend.sh
    echo "case \"\$1\" in">>backend.sh
    echo "  '-b')">>backend.sh
    echo "  NEEDSO=1;">>backend.sh
    echo "  ;;">>backend.sh
    echo "esac">>backend.sh
    echo "extract;">>backend.sh
    echo "exit">>backend.sh

    tar -zcf temp.tar.gz native/*
    uuencode temp.tar.gz t >>backend.sh

    chmod a+x backend.sh

    #delete the zip file
    rm temp.tar.gz
}

function release_backend()
{
    pre_init;

    pre_baseso;
# all daemons...
    pre_user_reg;
    pre_user_login;
    pre_user_activate;
    pre_user_auth;
    pre_user_passwd;
    pre_user_attr;

    pre_update_profile;
    pre_verifycode;

    pre_sips;
    pre_netdisk;

    gen_package_sh;
}


release_backend;

