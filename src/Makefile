INC=-I$(CURDIR)/hydrautil
export INC

all: hydramd hydrarun hydrasd

hydrautil :
	@$(MAKE) -C hydrautil

hydramd hydrarun hydrasd: hydrautil
	@$(MAKE) -C $@

.PHONY: hydrautil hydramd hydrarun hydrasd all
