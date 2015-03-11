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
.PHONY:clean
clean:
	$(MAKE) -C common/cds_base/ clean
	$(MAKE) -C services_2.0/ clean
