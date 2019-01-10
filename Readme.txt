CSC 360: Operating Systems (FALL 2018)
Programming Assignment 1: A Process Manager (PMan)

Student: Xiaole (Wendy) Tan
Vnumber: V00868734

Introduction:
Program PMan prompt and accept input from user, execute command in the background.

HOW:
To start program PMan, type "make" in command line, then "./pman"
NOTE: Makefile included only complie progam pman.c and inf.c. If there is other program that user wish to run in the background, make sure the program is complied before trying to execute in pman.

PMan accept commands: bg, bgkill, bgstop, bgstart, pstat, end

bg: To use command bg, user type bg [process]. Process need to be complied before executing in pman.
    Example: to run inf.c in the background, type "bg ./inf"
bglist: User only need to enter bglist, pman will return list of running program with total running job count.
bgkill: User enter bgkill [pid]. pid have to be a running/paused process, if pid not found pman will return error.
bgstop: User enter bgstop [pid]. pid have to be a running process, if pid not found pman will return error.
bgstart: User enter bgstart [pid]. pid have to be a paused process, if pid not found pman will return error.
pstat: User enter pstat [pid]. pid have to be a running/paused process, if pid not found pman will return error.
       pman will return list of info regarding given process.
end: User enter "end", to exit pman. 

Citation:
Some of the linkedlist code referenced
SENG265 SPRING 2018 Assignment 4 submission.
see code in folder: seng265a4
