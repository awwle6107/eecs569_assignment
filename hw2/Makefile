include Makefile.defs

# If CFLAGS_CRAY is empty set it to CFLAGS
ifeq ($(CFLAGS_CRAY),)
CFLAGS_CRAY = ${CFLAGS}
endif

.c.o:
	${CC} ${CFLAGS} $(OMPFLAG) -c $*.c 

SYNCOBJS =  syncbench.o common.o 
SCHEDOBJS = schedbench.o common_sched.o 
SCHEDFLAGS = -DSCHEDBENCH

all:	syncbench schedbench 

syncbench: $(SYNCOBJS)
	$(CC) -o syncbench $(LDFLAGS) $(SYNCOBJS) $(CLOCKOBJS) $(LIBS) -lm

# Rule to ensure the lower optimisation level is picked up for common.c 
# with the Cray compiler
common.o:	
	${CC} ${CFLAGS_CRAY} $(OMPFLAG) -o common.o -c common_sched.c
	#$*.c 

# Separate rule to build common_sched.o as we need to ensure the correct 
# DEFAULT_DELAY_TIME is used. 
common_sched.o:	
	${CC} ${CFLAGS_CRAY} $(SCHEDFLAGS) $(OMPFLAG) -o common_sched.o -c common_sched.c

schedbench: $(SCHEDOBJS)
	$(CC) -o schedbench  $(LDFLAGS) $(SCHEDOBJS) $(CLOCKOBJS) $(LIBS) -lm 

clean: 
	-rm *.o syncbench schedbench 



