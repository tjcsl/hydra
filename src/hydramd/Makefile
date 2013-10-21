SRC=main.c hydramaster.c

$(OUTDIR)/hydramd: $(SRC) $(OUTDIR)/libhydrautil.a
	gcc $(CFLAGS) $(INC) $(SRC) -o $@ -L$(OUTDIR) -lhydrautil

main.c hydramaster.c: hydramaster.h 
