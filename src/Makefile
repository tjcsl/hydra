export INC := -I$(CURDIR)/hydrautil -I$(CURDIR)/../extern/inih

all: hydramd hydrarun hydrasd

hydrautil :
	@$(MAKE) -C hydrautil

hydramd hydrarun hydrasd: hydrautil
	@$(MAKE) -C $@

.PHONY: hydrautil hydramd hydrarun hydrasd all
