OBJECTS=$(OUTDIR)/hydracommon.o $(OUTDIR)/hydranet.o  $(OUTDIR)/hydrapacket.o

$(OUTDIR)/libhydrautil.a: $(OBJECTS)
	ar rcs $(OUTDIR)/libhydrautil.a $(OBJECTS)

$(OUTDIR)/hydracommon.o: hydracommon.c
	gcc -c $^ -o $@

$(OUTDIR)/hydranet.o : hydranet.c
	gcc -c $^ -o $@

$(OUTDIR)/hydrapacket.o: hydrapacket.c hydrapacket.h
	gcc -c hydrapacket.c -o $@

hydrapacket.c : hydrapacket.h hydrapacket.in
hydrapacket.h : hydrapacket.in gen_hydrapacket.py
	python3 ./gen_hydrapacket.py
hydracommon.c : hydracommon.h
hydranet.c : hydranet.h
