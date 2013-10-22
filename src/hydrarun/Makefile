SRC=hydrarun.c

$(OUTDIR)/hydrarun: $(SRC) $(OUTDIR)/libhydrautil.a
	gcc $(CFLAGS) $(INC) $(SRC) -o $@ -L$(OUTDIR) -lhydrautil
