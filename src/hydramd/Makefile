SRC=main.c hydramaster.c dispatcher.c moniter.c

$(OUTDIR)/hydramd: $(SRC) $(OUTDIR)/libhydrautil.a
	$(CC) $(CFLAGS) $(INC) $(SRC) -o $@ -L$(OUTDIR) -lhydrautil -linih

hydramaster.c: hydramaster.h dispatcher.h
main.c: hydramaster.h dispatcher.h
dispatcher.c: dispatcher.h
moniter.c: moniter.h
