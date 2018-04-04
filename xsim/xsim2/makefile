# Targets & general dependencies
PROGRAM = xsim
HEADERS = xis.h xcpu.h 
OBJS = xsim.o xcpu.o xcpuprnt.o xmem.o
ADD_OBJS = 
GOLD = xsim_gold 

# compilers, linkers, utilities, and flags
CC = gcc
CFLAGS = -Wall -g
COMPILE = $(CC) $(CFLAGS)
LINK = $(CC) $(CFLAGS) -o $@ 

# implicit rule to build .o from .c files
%.o: %.c $(HEADERS)
	$(COMPILE) -c -o $@ $<


# explicit rules
all: xld xas xcc $(GOLD) xsim 

$(PROGRAM): $(OBJS) $(ADD_OBJS)
	$(LINK) $(OBJS) $(ADD_OBJS) -l pthread

xsim_gold: libxsim.a xmem.o
	$(LINK) xmem.o libxsim.a -l pthread

xas: xas.o xreloc.o
	$(LINK) xas.o xreloc.o

xld: xld.o xreloc.o
	$(LINK) xld.o xreloc.o

xcc: xcc.o 
	$(LINK) xcc.o
	
# build individual test executables
# adapted from Andre Gagne's idea & naming . 
xmem_test: xmem.o $(ADD_OBJS) xcpuprnt.o libxsim.a
	$(LINK) xmem.o $(ADD_OBJS) xcpuprnt.o libxsim.a 

xcpu_test: xcpu.o $(ADD_OBJS) xcpuprnt.o libxsim.a
	$(LINK) xcpu.o $(ADD_OBJS) xcpuprnt.o libxsim.a 

xsim_test: xsim.o $(ADD_OBJS) xcpuprnt.o libxsim.a
	$(LINK) xsim.o $(ADD_OBJS) xcpuprnt.o libxsim.a 


lib: xsim_gold.o xcpu_gold.o xcpuprnt_gold.o
	 ar -r libxsim.a xsim_gold.o xcpu_gold.o xcpuprnt_gold.o

clean:
	rm -f *.o *.xo *.xx $(PROGRAM) xas xld xcc $(GOLD)

zip:
	make clean
	rm -f xsim.zip
	zip xsim.zip *
