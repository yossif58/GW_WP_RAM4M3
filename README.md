# GW_WP_RAM4M3
GW_WP_RAM4M3 is the firmware embedded in R7FA4M3****FB RENESSAS CHIP. It is part of the GW WP card it is responsable to communicate between the efr312g13 RF card and the quectel EC200S LTE modem.
We are using estudio for creating and debugging the project. the final product is programmed in both ways:
you must have Jlink Plus of segger with at leat version 11.0

a. estudio by lanching the project file in the main

b. programming gw_ra4m3.hex file that is under main\debug by JFLASH OF SEGGER configuration file gw_ra4m3.jflash is located at main.


