SRC=main.c hydraslave.c system.c network.c

$(OUTDIR)/hydrasd: $(SRC) $(OUTDIR)/libhydrautil.a 
	$(CC) $(CFLAGS) $(INC) $(SRC) -o $@ -L$(OUTDIR) -lhydrautil -lm

main.c hydramaster: hydraslave.h system.h network.h
