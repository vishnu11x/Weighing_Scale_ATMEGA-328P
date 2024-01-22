# Weighing_Scale_ATMEGA-328P
* This includes Registry-level C program and ".hex" to measure the weight and to find the Calibration scale factor of the Load cell.
* This program's are written for ATMEGA-328P MICROCONTROLLER using MPLAB X IDE

# CAL.X
* This folder contains the C program and ".hex" to find the Calibration Scale Factor of the load cell.
* This Scale Factor is required for the calculation of the weight in grams.
* The output value from this program should be entered into the main program (main.c).
* This program calculate the scale factor by placing a know weight on the Load Cell.
* Please update the "ref_w" in the program with the weight you are using to calibrate in grams.
* After flashing this program please follow the commands given in the LCD till Avg Scale Factor is displayed.
* This program will calculate scale factor 5 time and then take its average.

# MAIN.X
* This folder contains the C program and ".hex" to measure the weight.
* The Scale Factor from the " cal.c " should be updated in the variable " cal_val " of the program.
* After updating the " cal_val " flash the program to your Atmega-328P to measure weight.

# Components Used
* Arduino Uno (Atmega-328p)
* HX711 Load Cell Amplifier
* Straight Bar Load Cell
* 16x2 I2C LCD

# Software Used
* MPLAB X IDE
* AVRDUDESS 
