# TOP-LEVEL Makefile of the whole server backend building
#
# type :
#   make       - build cds base component(libcds.so, etc...), then all sub-modules
#   make clean - clean all above build
#
#   You can also build each component one-by-one in each sub-directory.
#
all:
	$(MAKE) -C common/cds_base/
	$(MAKE) -C services_2.0/

.PHONY:dist
dist:
	-rm -rf bin/
	-mkdir -p bin/
	-cp common/cds_base/libcds.so bin/
	-cp services_2.0/user_reg/urs bin/
	-cp services_2.0/user_login/uls bin/
	-cp services_2.0/user_activate/acts bin/
	-cp services_2.0/user_auth/uauth bin/
	-cp services_2.0/password_manager/passwdmgr bin/
	-cp services_2.0/attribute_modification/attrmodify bin/
	-tar -zcf allbin.tar.gz bin/*

.PHONY:clean
clean:
	-rm -rf bin/
	$(MAKE) -C common/cds_base/ clean
	$(MAKE) -C services_2.0/ clean
