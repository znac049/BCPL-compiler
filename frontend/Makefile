# Makefile for bcplc/src

PREFIX?=/usr/local

#CG=./cg
CG=/usr/local/lib/bcpl/cg
AFLAGS=--32

all: st

st: su.o st.o blib.o global.o rt.o sys.o
	ld -m elf_i386 -o st su.o st.o blib.o global.o rt.o sys.o

su.o: su.s
	$(AS) $(AFLAGS) -o su.o su.s

st.o: st.s
	$(AS) $(AFLAGS) -o st.o st.s

st.s: cg op st.O
	$(CG) < st.O | ./op > st.s

blib.o: blib.s
	$(AS) $(AFLAGS) -o blib.o blib.s

blib.s: cg op blib.O
	$(CG) < blib.O | ./op > blib.s

global.o: global.s
	$(AS) $(AFLAGS) -o global.o global.s

rt.o: rt.s
	$(AS) $(AFLAGS) -o rt.o rt.s

sys.o: sys.s
	$(AS) $(AFLAGS) -o sys.o sys.s

check:	blib.O blib.chk st.O st.chk
	cmp blib.O blib.chk
	cmp st.O st.chk

blib.chk: st blib.bcpl
	./st < blib.bcpl > blib.chk

st.chk:	st st.bcpl
	./st < st.bcpl > st.chk

install: bcplc bcplc.1 st cg op LIBHDR su.o blib.o global.o rt.o sys.o
	mkdir -p $(PREFIX)/bin
	install -c  -m 755 bcplc     $(PREFIX)/bin
	mkdir -p $(PREFIX)/man/man1
	install -c  -m 644 bcplc.1   $(PREFIX)/man/man1
	mkdir -p $(PREFIX)/lib/bcplc
	install -cs -m 555 st	     $(PREFIX)/lib/bcplc
	install -c  -m 644 LIBHDR    $(PREFIX)/lib/bcplc
	install -c  -m 444 su.o	     $(PREFIX)/lib/bcplc
	install -c  -m 444 blib.o    $(PREFIX)/lib/bcplc
	install -c  -m 444 global.o  $(PREFIX)/lib/bcplc
	install -c  -m 444 rt.o	     $(PREFIX)/lib/bcplc
	install -c  -m 444 sys.o     $(PREFIX)/lib/bcplc
	rm -f $(PREFIX)/lib/bcplc/libhdr
	ln $(PREFIX)/lib/bcplc/LIBHDR $(PREFIX)/lib/bcplc/libhdr

clean:
	rm -f st st.o st.s blib.o blib.s su.o global.o rt.o sys.o
	rm -f blib.chk st.chk
