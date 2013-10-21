SRC=main.c hydraslave.c

$(OUTDIR)/hydrasd: $(SRC) $(OUTDIR)/libhydrautil.a 
	gcc $(CFLAGS) $(INC) $(SRC) -o $@ -L$(OUTDIR) -lhydrautil

main.c hydramaster: hydraslave.h
