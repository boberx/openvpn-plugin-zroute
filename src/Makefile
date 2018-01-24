CC = gcc
BUILDDIR = build

PLGNSRC = openvpn-plugin-zroute.c zroute.c
PLGNOBJ = $(patsubst %.c,$(BUILDDIR)/%.o,$(PLGNSRC))
PLGNSNM = openvpn-plugin-zroute
PLGNCFL = -O2 -fPIC -Wall -Wextra $(CFLAGS) -I/usr/include/quagga

all: dir $(BUILDDIR)/$(PLGNSNM).so

$(BUILDDIR)/$(PLGNSNM).so: $(PLGNOBJ)
	$(CC) $(PLGNCFL) -shared $^ -o $@ -lc -lzebra

$(PLGNOBJ): $(BUILDDIR)/%.o : %.c dir
	$(CC) $(PLGNCFL) -c $< -o $@

test: $(PLGNOBJ)
	$(CC) $(PLGNCFL) -o $(BUILDDIR)/zroute test.c $^ -lc -lzebra

dir:
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
