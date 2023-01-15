# CS488 Winter 2023 Assignment 0

## Compilation
There are no changes relative to the default premake4/make combination.

I have tested the assignment on the VM, all changes required should be working as indicated in the **screenshot.png** file.

First you will need to build all
the static libraries that the projects depend on. To build the libraries, open up a
terminal, and **cd** to the top level of the CS488 project directory and then run the
following:

    $ premake4 gmake
    $ make

This will build the following static libraries, and place them in the top level **lib**
folder of your cs488 project directory.
* libcs488-framework.a
* libglfw3.a
* libimgui.a

Next we can build a specific project.  To do this, **cd** into the **A0** project folder, and run the following terminal commands in order to compile the A0 executable using all .cpp files in the A0 directory:

    $ cd A0/
    $ premake4 gmake
    $ make

## Manual
I did not add anything besides what was asked for in the specifiction. I believe I have completed all the objectives.
