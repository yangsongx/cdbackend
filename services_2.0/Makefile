# Top-level of service-2.0 Makefile
# # È¡µÃµ±Äê×ÓÄ¿Â¼Éî¶ÈÎª1µÄËùÓÐÄ¿Â¼Ãû³Æ
#
#
#
## Reference http://www.gnu.org/software/make/manual/make.html
#
##

exclude_dirs := include bin

# È¡µÃµ±Äê×ÓÄ¿Â¼Éî¶ÈÎª1µÄËùÓÐÄ¿Â¼Ãû³Æ
#
dirs := $(shell find . -maxdepth 1 -type d)
dirs := $(basename $(patsubst ./%,%,$(dirs)))
dirs := $(filter-out $(exclude_dirs),$(dirs))
#
SUBDIRS := $(dirs)
#
clean_dirs := $(addprefix _clean_,$(SUBDIRS) )
#
.PHONY: subdirs $(SUBDIRS) clean
#
# make target
#
 $(SUBDIRS):    
	$(MAKE) -C $@
#
subdirs: $(SUBDIRS)
#
 #
#
$(clean_dirs):    
	$(MAKE) -C $(patsubst _clean_%,%,$@) clean
#
clean: $(clean_dirs)    
	@find . /        
	/( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' /
	-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' /
	-o -name '*.symtypes' /) /
	-type f -print | xargs rm -f
