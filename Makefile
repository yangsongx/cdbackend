# TOP-LEVEL Makefile of the whole server backend building
all:
	$(MAKE) -C common/cds_base/
	$(MAKE) -C service_2.0/
