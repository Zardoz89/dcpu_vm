DCPU VM
=======

Another DCPU-16 VM

Based on Benedek Vartok VM (https://bitbucket.org/benedek/dcpu-16/overview)

COMPILING
---------

  make

RUN
---

  ./Debug/DCPU dcpuBinFileProgram
  
Each tieme you press any key, the VM executes 100 clock cycles.
The 'q' key finish the VM.

Actually the VM have attached a fake LEM1802 monitor that shows in terminal a 
B&W ASCII representation of the screen. Allow to see text on it, and respond to 
all commands of a real LEM1802, but don't uses palettes, color, border color or font maps.
It refresh at 60Hz (1666,7 cpu cycles), sou will need press a few times any key 
to see anything in the terminal.

TODO
----

- Implement more hardware devices
- Check how many VM can run at same time
- Realtime execution at 100KHz
