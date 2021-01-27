### L.E. Terminal (Lightweight Emulated Terminal)

![Logo](res/logo.png)

L.E. Terminal (*let* for short) is a little UNIX-inspired terminal for the Numworks Calculator. 

![Screenshot](res/screenshot)

*It does not use escher (Numworks' GUI Library)*.

## Features

### Bare bones multi-user system
Each user have an username, a UID (universal id), and an execution level (*low* to *root*)

*Let* introduces four default user accounts, each assigned to a specific execution level : 

+ **root** is, obviously, the **root** user : it have the highest execution level (well, named **root**)
+ **boat**, who takes the lowest execution level (**low**)
+ **cacahuete**, who takes the normal execution level (**normal**)
+ **coconut**, who takes the **high** (right below root) execution level.

Users can also be added dynamically using the `useradd` command.

### Filesystem

The standard UNIX filesystem commands that are implemented currently interfaces with the Numworks' storage records (where are saved Python scripts for example). A text editor is actually in development, called **kilo** (inspired from **nano**)

## Commands

Here are the implemented commands : 
+ `uname` : same command as UNIX : displays kernel info such as Epsilon's version, etc...
+ `echo` : print the passed arguments to the screen
+ `clear` : clear the terminal
+ `history` : displays the commands history
+ `whoami`: displays the current user name
+ `ion` : interfaces with Ion (Numworks low level library)

    Arguments : 
    + `-s` : put the calculator in sleep mode (like the power button)
    + `-D` : enter DFU mode (needs **root** privileges)
    + `-S` : shutdown the calculator
    + `-r` : reboot the calculator (not working for the moment)
    + `-b` : shows the battery level as a nice bar
    + `-u` : shows USB infos
+ `rm`, `touch`, `cp`, `cat` : same as UNIX
+ `ls` : list all files in the Numworks' storage (because of a limitation within Ion, `ls` needs a file extension to list files, default is set to `py`)
+ `pyscr` : creates the default samples Python scripts in *Epsilon* (i.e. `squares.py`, etc...)
+ `args` : displays every arguments on screen, including the command
+ `chars` : displays every char in the range 0-255 (for debug purposes)
+ `poincare` : an interactive *poincare* shell (for calculation) (*Warning : it crashes if you throw it letters in input*)
+ `su` : means here **switch user**, who switches to **root** if no arguments passed, or the user who is passed in arguments (user name)
+ `useradd` : add an user to the user repository
+ `users` : list available users, use `-d` as argument for detailed output

## Building instructions

*Note : the terminal does not support the simulator*

### As a "*subfirmware*"
If you want to use *let* as a "subfirmware" (kind of a firmware in a firmware), follow these steps : 

+ Clone the source of the firmware you want to use (ex: [Epsilon](https://github.com/numworks/epsilon))
+ Clone this repository in the `apps/terminal` folder of the firmware source (the `terminal` folder needs to be created, of course)
+ Add the following line to the *makefile* `apps/Makefile` at the top : 
```
include apps/terminal/Makefile
```
+ Go to the `main.cpp` file and add this line to the top :
```
#include "terminal/startup.h"
```
+ In the same file, call the terminal's main function right below `Poincare::Init();` : 
```
void ion_main(int argc, const char * const argv[]) {
  // Initialize Poincare::TreePool::sharedPool
  Poincare::Init();

  terminal_startup_check(argc, argv);
```
And done ! You will be able to launch the terminal at startup when pressing the *HOME* key !