# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ../gnu/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles passport_office network_test client1 client2 client3 client4 client5 app_clerk pic_clerk pass_clerk cashier manager customer passport passport0 passport1 servers network_test2

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

passport_office.o: passport_office.c
	$(CC) $(CFLAGS) -c passport_office.c
passport_office: passport_office.o start.o
	$(LD) $(LDFLAGS) start.o passport_office.o -o passport_office.coff
	../bin/coff2noff passport_office.coff passport_office

network_test.o: network_test.c
	$(CC) $(CFLAGS) -c network_test.c
network_test: network_test.o start.o
	$(LD) $(LDFLAGS) start.o network_test.o -o network_test.coff
	../bin/coff2noff network_test.coff network_test
	
	
network_test2.o: network_test2.c
	$(CC) $(CFLAGS) -c network_test2.c
network_test2: network_test2.o start.o
	$(LD) $(LDFLAGS) start.o network_test2.o -o network_test2.coff
	../bin/coff2noff network_test2.coff network_test2
	
client1.o: client1.c
	$(CC) $(CFLAGS) -c client1.c
client1: client1.o start.o
	$(LD) $(LDFLAGS) start.o client1.o -o client1.coff
	../bin/coff2noff client1.coff client1

client2.o: client2.c
	$(CC) $(CFLAGS) -c client2.c
client2: client2.o start.o
	$(LD) $(LDFLAGS) start.o client2.o -o client2.coff
	../bin/coff2noff client2.coff client2
	
client3.o: client3.c
	$(CC) $(CFLAGS) -c client3.c
client3: client3.o start.o
	$(LD) $(LDFLAGS) start.o client3.o -o client3.coff
	../bin/coff2noff client3.coff client3

client4.o: client4.c
	$(CC) $(CFLAGS) -c client4.c
client4: client4.o start.o
	$(LD) $(LDFLAGS) start.o client4.o -o client4.coff
	../bin/coff2noff client4.coff client4
	
client5.o: client5.c
	$(CC) $(CFLAGS) -c client5.c
client5: client5.o start.o
	$(LD) $(LDFLAGS) start.o client5.o -o client5.coff
	../bin/coff2noff client5.coff client5
	
app_clerk.o: app_clerk.c
	$(CC) $(CFLAGS) -c app_clerk.c
app_clerk: app_clerk.o start.o
	$(LD) $(LDFLAGS) start.o app_clerk.o -o app_clerk.coff
	../bin/coff2noff app_clerk.coff app_clerk
	
pic_clerk.o: pic_clerk.c
	$(CC) $(CFLAGS) -c pic_clerk.c
pic_clerk: pic_clerk.o start.o
	$(LD) $(LDFLAGS) start.o pic_clerk.o -o pic_clerk.coff
	../bin/coff2noff pic_clerk.coff pic_clerk
	
pass_clerk.o: pass_clerk.c
	$(CC) $(CFLAGS) -c pass_clerk.c
pass_clerk: pass_clerk.o start.o
	$(LD) $(LDFLAGS) start.o pass_clerk.o -o pass_clerk.coff
	../bin/coff2noff pass_clerk.coff pass_clerk
	
cashier.o: cashier.c
	$(CC) $(CFLAGS) -c cashier.c
cashier: cashier.o start.o
	$(LD) $(LDFLAGS) start.o cashier.o -o cashier.coff
	../bin/coff2noff cashier.coff cashier
	
manager.o: manager.c
	$(CC) $(CFLAGS) -c manager.c
manager: manager.o start.o
	$(LD) $(LDFLAGS) start.o manager.o -o manager.coff
	../bin/coff2noff manager.coff manager
	
customer.o: customer.c
	$(CC) $(CFLAGS) -c customer.c
customer: customer.o start.o
	$(LD) $(LDFLAGS) start.o customer.o -o customer.coff
	../bin/coff2noff customer.coff customer
	
passport.o: passport.c
	$(CC) $(CFLAGS) -c passport.c
passport: passport.o start.o
	$(LD) $(LDFLAGS) start.o passport.o -o passport.coff
	../bin/coff2noff passport.coff passport	
	
passport0.o: passport0.c
	$(CC) $(CFLAGS) -c passport0.c
passport0: passport0.o start.o
	$(LD) $(LDFLAGS) start.o passport0.o -o passport0.coff
	../bin/coff2noff passport0.coff passport0
	
passport1.o: passport1.c
	$(CC) $(CFLAGS) -c passport1.c
passport1: passport1.o start.o
	$(LD) $(LDFLAGS) start.o passport1.o -o passport1.coff
	../bin/coff2noff passport1.coff passport1	
	
servers.o: servers.c
	$(CC) $(CFLAGS) -c servers.c
servers: servers.o start.o
	$(LD) $(LDFLAGS) start.o servers.o -o servers.coff
	../bin/coff2noff servers.coff servers
	
clean:
	rm -f *.o *.coff
