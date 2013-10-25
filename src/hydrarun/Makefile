SRC=hydrarun.c

$(OUTDIR)/hydrarun: $(SRC) $(OUTDIR)/libhydrautil.a
	$(CC) $(CFLAGS) $(INC) $(SRC) -o $@ -L$(OUTDIR) -lhydrautil
