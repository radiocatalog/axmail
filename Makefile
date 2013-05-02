all: axmail

CC = gcc
LD = gcc
CFLAGS = -Wall -static -Wstrict-prototypes -g -I../lib
LDFLAGS = -lcrypt
MODULES = utils.o config.o adduser.o command.o mailcmd.o mbox.o head.o lock.o axmail.o quit.o

.c.o:
	$(CC) $(CFLAGS) -c $<

install: installbin installconf installhelp

installbin: all
	install -m 0755 -s -o root -g root axmail	 /usr/sbin

installconf:
	install -m 755    -o root -g root -d		  /etc/ax25
	install -m 644    -o root -g root etc/axmail.conf /etc/ax25
	install -m 644    -o root -g root etc/welcome.txt /etc/ax25

installhelp:
	install -m 755    -o root -g root -d		  /var/ax25/axmail/help
	install -m 644    -o root -g root etc/help/*.hlp  /var/ax25/axmail/help

back:
	rm -f ../mail.tar.gz
	tar cvf ../mail.tar *
	gzip ../mail.tar

clean:
	rm -f axmail *.o *~ *.bak core etc/*~ etc/help/*~

distclean: clean
	rm -f axmail

axmail: $(MODULES)
	$(LD) $(LDFLAGS) -o axmail $(MODULES) $(LIBS)

utils.o:	utils.h utils.c mbox.h
config.o:	config.h config.c defines.h axmail.h utils.h
adduser.o:	adduser.h adduser.c utils.h config.h defines.h
command.o:	command.h command.c config.h mailcmd.h mbox.h utils.h quit.h
mailcmd.o:	mailcmd.h mailcmd.c defines.h utils.h mbox.h config.h
mbox.o:		mbox.h mbox.c utils.h defines.h config.h head.h
head.o:		head.h head.c defines.h
utils.o:	utils.h utils.c 
lock.o:		lock.h lock.c utils.h
quit.o:		quit.h quit.c config.h lock.h
axmail.o:	axmail.h axmail.c config.h adduser.h utils.h quit.h mbox.h
