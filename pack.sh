#!/bin/bash

# MUST NOT called directly, should embeded in Makefile
#

function pre_init()
{
    rm -rf bin/
    mkdir -p bin
}

# FIXME - do we need the .so or the .deb ?
function pre_baseso()
{
    echo -n "prepare the base SO..."
    cp common/cds_base/libcds.so bin/
    cp common/cds_base/cds_cfg.xml bin/
    echo "\n the base so library creation [OK]"
}

function pre_user_reg()
{
    mkdir -p bin/user_reg
    echo "#!/bin/bash"> bin/user_reg/run
    echo "./urs -u root -vvv -p 13000 >reg.log" >> bin/user_reg/run
    chmod a+x bin/user_reg/run

    cp services_2.0/user_reg/urs bin/user_reg/
}

function pre_user_login()
{
    mkdir -p bin/user_login
    echo "#!/bin/bash"> bin/user_login/run
    echo "./uls -u root -vvv -p 13001 >login.log" >> bin/user_login/run
    chmod a+x bin/user_login/run

    cp services_2.0/user_login/uls bin/user_login/
}

function pre_user_activate()
{
    mkdir -p bin/user_activate
    echo "#!/bin/bash"> bin/user_activate/run
    echo "./acts -u root -vvv -p 13002 >activation.log" >> bin/user_activate/run
    chmod a+x bin/user_activate/run

    cp services_2.0/user_activate/acts bin/user_activate/
}

function pre_user_auth()
{
    mkdir -p bin/user_auth
    echo "#!/bin/bash"> bin/user_auth/run
    echo "./acts -u root -vvv -p 13003 >user_authentication.log" >> bin/user_auth/run
    chmod a+x bin/user_auth/run

    cp services_2.0/user_auth/uauth bin/user_auth/
}

function pre_user_passwd()
{
    mkdir -p bin/password_manager/
    echo "#!/bin/bash"> bin/password_manager/run
    echo "./passwdmgr -u root -vvv -p 13004 >user_authentication.log" >> bin/password_manager/run
    chmod a+x bin/password_manager/run

    cp services_2.0/password_manager/passwdmgr bin/password_manager/
}

function pre_user_attr()
{
    mkdir -p bin/attribute_modification/
    echo "#!/bin/bash"> bin/attribute_modification/run
    echo "./attrmodify -u root -vvv -p 13005 >attr.log" >> bin/attribute_modification/run
    chmod a+x bin/attribute_modification/run

    cp services_2.0/attribute_modification/attrmodify bin/attribute_modification/
}

function pre_update_profile()
{
    mkdir -p bin/update_profile/
    echo "#!/bin/bash"> bin/update_profile/run
    echo "./upusr -u root -vvv -p 13006 >up.log" >> bin/update_profile/run
    chmod a+x bin/update_profile/run

    cp services_2.0/update_profile/upusr bin/update_profile/
}

function pre_verifycode()
{
    mkdir -p bin/verify_code/
    echo "#!/bin/bash"> bin/verify_code/run
    echo "./vcs -u root -vvv -p 13007 >vc.log" >> bin/verify_code/run
    chmod a+x bin/verify_code/run

    cp services_2.0/verify_code/vcs bin/verify_code/
}

function pre_sips()
{
    mkdir -p bin/opensips_account/
    echo "#!/bin/bash"> bin/opensips_account/run
    echo "./opas -u root -vvv -p 12002 >vc.log" >> bin/opensips_account/run
    chmod a+x bin/opensips_account/run

    cp services_2.0/opensips_account/opas bin/opensips_account/
}

function gen_package_sh()
{
    echo "#!/bin/bash"> bin/startall
    echo "svscan . &" >> bin/startall
    chmod a+x bin/startall


    echo "#!/bin/bash" > backend.sh
    echo "echo \"Will deploy the daemon services... \"" >> backend.sh
    echo "function extract() {">>backend.sh
    echo "  uudecode \$0">>backend.sh
    echo "  tar -zxvf t">>backend.sh
    echo "">>backend.sh
    echo "  if [ -n \"\$NEEDSO\" ]; then" >> backend.sh
    echo "    echo \"You need libcds.so\"">>backend.sh
    echo "    cp bin/libcds.so /usr/lib/">>backend.sh
    echo "  else">>backend.sh
    echo "    echo \"You don't need libcds.so\"">>backend.sh
    echo "  fi">>backend.sh
    echo "  #rm bin/libcds.so">>backend.sh
    echo "  #rm bin/cds_cfg.xml">>backend.sh
    echo "  rm t">>backend.sh
    echo "}">>backend.sh
    echo "case \"\$1\" in">>backend.sh
    echo "  '-b')">>backend.sh
    echo "  NEEDSO=1;">>backend.sh
    echo "  ;;">>backend.sh
    echo "esac">>backend.sh
    echo "extract;">>backend.sh
    echo "exit">>backend.sh

    tar -zcf temp.tar.gz bin/*
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

    gen_package_sh;
}


release_backend;

