export MAKEFLAGS := --no-print-directory
export CFLAGS := $(CFLAGS) -Wall
export OUTDIR := $(CURDIR)/build

all: $(OUTDIR) src

src : extern
	@$(MAKE) -C src

extern :
	@$(MAKE) -C extern
clean:
	rm -rf $(OUTDIR)
	rm src/hydrautil/hydrapacket.h
	rm src/hydrautil/hydrapacket.c

$(OUTDIR):
	mkdir -p $(OUTDIR)

.PHONY: all src extern clean

