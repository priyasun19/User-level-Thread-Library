
CS519 Homework 3 Notes
----------------------

I have created several files that you should use for your third assignment.
For Part-A, please code your solution in the "hw3a.c" file.  Do not put
any code before the "my_pthread_begin" or after the "my_pthread_end" definitions.
Of course you may add any additional system header files that you may need.
However, do not add any additional files to your project.  You should only
submit the following files:

hw3a.c
makefile
mypthread.c
mypthread.h
report.txt

Please do not change the makefile as I will likely replace it later with
my own that will contain additional test cases.  By default, when you type
"make" the pthread library will be used.  In order for you to compile with
your implementation of the threading library, do a "make clean" followed by
a "make MYPTHREAD=1".  Once you have the non-preemptive version working,
you should use a macro to enable the preemtive version of your code.  For
example, it may look like this:

#if PREEMPTIVE > 0
	...
#else
	...
#endif

So, if you type "make MYPTHREAD=1 PREEMPTIVE=1" this should enable your user
level threading code with preemption.  Here is a summary:

make clean - clean
make - compile with pthreads
make MYPTHREAD=1 - compile with your user level thread library
make MYPTHREAD=1 PREEMPTIVE=1 - compile with your user level thread library
                                with preemption enabled

Why do we have the "my_pthread_begin" and "my_pthread_end" definitions?
We have them for the preemptive version of the code.  We need to make sure
that when a signal happens, and we want to perform a context switch of the
user level threads, that the instruction pointer lies within our "hw3a.c" code.
We do not want to perform a context switch if the current instruction is
somewhere else like in another library or even within the "mypthread.c" code
itself.  To make things easier, you are allowed to use the third argument
(ucontext_t) of the signal handler in this assignment.  I will be grading
this on a 64-bit Linux machine so please test it on one.  It will also be
tested automatically with a script so that I may test one component at a time,
for example, thread creation separately from mutexes.

Please let me know if there are any issues with the files I have sent you
so that I may update them and re-post them as soon as possible.

