EXECUTABLE = asciibrot

CFLAGS += -Wall -Wextra -O3
LDFLAGS += -lm

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
datarootdir = $(prefix)/share
mandir = $(datarootdir)/man
man6dir = $(mandir)/man6


.PHONY: clean install installdirs

$(EXECUTABLE): $(EXECUTABLE).c
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(EXECUTABLE).c $(LDFLAGS)

clean:
	rm -fv $(EXECUTABLE)

install: $(EXECUTABLE) installdirs
	$(INSTALL_PROGRAM) $(EXECUTABLE) $(DESTDIR)$(bindir)/$(EXECUTABLE)
	$(INSTALL_DATA) man6/$(EXECUTABLE).6 $(DESTDIR)$(man6dir)/$(EXECUTABLE).6

installdirs:
	mkdir -p $(DESTDIR)$(bindir) $(DESTDIR)$(man6dir)
