#******************************************************************************/
#* CS519 - Operating Systems Design                                           */
#* Author: Hans Christian Woithe                                              */
#******************************************************************************/

default: all

.PHONY: all clean

PREEMPTIVE = 0
MYPTHREAD = 0

HW3A = hw3a
HW3A_SRC = $(HW3A).c mypthread.c
HW3A_OBJ = $(HW3A_SRC:.c=.o)

CC = gcc $(FLAGS)
COMOPTS = -ggdb -Wall -O0
CFLAGS = $(COMOPTS) -DPREEMPTIVE=${PREEMPTIVE} -DMYPTHREAD=${MYPTHREAD}
LDOPTS = $(COMOPTS) -pthread

OPTS = -I. -g -Wall -Werror

all: depend.make $(HW3A) $(HW3B)

$(HW3A): $(HW3A_OBJ)
	$(CC) -g -o $(HW3A) $(HW3A_OBJ) $(LDOPTS) 

depend: depend.make

depend.make: $(HW3A_SRC) $(HW3B_SRC)
	$(CC) -c -MM $(HW3A_SRC) > depend.make

clean:  
	rm -f $(HW3A)
	rm -f $(HW3A_OBJ)
	rm -f *~
	rm depend.make

-include depend.make
