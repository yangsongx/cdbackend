# Top-level of service-2.0 Makefile
##

BUILD_NUMBER=$(shell date +%Y%m%d)
export BUILD_NUMBER

# FIXME - currently, we don't contain any excluding dir
exclude_dirs := include bin

dirs := $(shell find . -maxdepth 1 -type d)
dirs := $(basename $(patsubst ./%,%,$(dirs)))
dirs := $(filter-out $(exclude_dirs),$(dirs))

SUBDIRS := $(dirs)
clean_dirs := $(addprefix _clean_,$(SUBDIRS) )

# default target
all:a22301
a22301:$(SUBDIRS)

.PHONY: subdirs $(SUBDIRS) clean
$(SUBDIRS):    
	$(MAKE) -C $@

subdirs: $(SUBDIRS)
$(clean_dirs):    
	$(MAKE) -C $(patsubst _clean_%,%,$@) clean
	-rm *.o
	
clean: $(clean_dirs)    
