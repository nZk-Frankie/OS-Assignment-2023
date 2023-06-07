This implementation is tested and compiled using the Kubuntu operating system which are simulated using the Oracle VM virtual box.

The gcc version used is 12.2.0 (Ubuntu 12.2.0-3ubuntu1).

The program can be compiled using 'make'. To ensure proper compiling, please run 'make clean' first then run 'make'.

The program run as follows
./cq m tc tw td ti
where all the arguments are POSITIVE integers

m is the size of the customer queue
tc is the periodic time for the customer to arrive in the queue
tw,td,ti represent the time duration to serve withdrawal, deposit, and information.
