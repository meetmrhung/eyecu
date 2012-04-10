Starterware quick start:
http://processors.wiki.ti.com/index.php/Quick_Start_Guide_StarterWare_02.00.XX.XX_(supports_AM335x)

Starterware getting started:
http://processors.wiki.ti.com/index.php/StarterWare_Getting_Started_02.00.XX.XX

Starterware user guide:
http://processors.wiki.ti.com/index.php/StarterWare_02.00.00.06_User_Guide

JTAG debugging with starterware and other environment setup:
http://processors.wiki.ti.com/index.php/AM335X_StarterWare_Environment_Setup#Debugging_on_BeagleBone_board_using_CCSv5



You should set your CCSv5 workspace to the ??\ti\AM335x_starterware\build\armv7a\cgttms470_ccs\am335x\beaglebone
or wherever you installed the starterware to. This will allow you to import the examples.

To build a project:
1 build the project in CCSv5
2 go to ??\ti\AM335x_starterware\binary\armv7a\cgttms470_ccs\am335x\beaglebone for the demo projects and open the folder
  for the project you want
3 copy the .bin from the folder to ??\ti\AM335x_starterware\tools\ti_image
4 using the command prompt go to ??\ti\AM335x_starterware\tools\ti_image
5 run the command: tiimage 0x80000000 NONE <project binary> <output path>\app
  example: tiimage 0x80000000 NONE uartEcho.bin app
6 take the output app and copy it to the SD card
7 if your SD card does not already have the MLO file go to ??\ti\AM335x_starterware\binary\armv7a\cgttms470_ccs\am335x\beaglebone\bootloader
  and copy it to the SD as well
8 boot and test