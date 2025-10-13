
# asmx_microdata
This is a fork of asmx that adds support for the microdata 1600, BasicFour 1200 as well as Basic Four 1300 and 1320 (e.g. for Basic Four 210/310/520/720, ...)  assembler. A disassembler is included as well.
The default output format is the hex format for Basic Four 13xx VDT bootstrap so the output can be pasted to a CPU where the sense switches  are in VDT bootstrap mode. (as e.g. documented here: [13xx CPU](http://basicfour.de/cpu/small/index.html))

It is based on the information found at

[69-2-0810-002_AP810_Assembly_Pgm_Jan70](http://bitsavers.informatik.uni-stuttgart.de/pdf/microdata/800/69-2-0810-002_AP810_Assembly_Pgm_Jan70.pdf)
as well as some basic information about the opcodes i scanned here

[S-HDBK-211_Sorbus_SR_Handbook_for_Basic_Four_1981](http://bitsavers.informatik.uni-stuttgart.de/pdf/basicFour/S-HDBK-211_Sorbus_SR_Handbook_for_Basic_Four_1981.pdf)
As the information for the 13xx is very limited and the parameters for some instructions are not known, the assembler may not work very well for the 13xx CPU. The disassebler should process all the Microdata as well as the 1200 and 1300 macrocode instructions.

I hope that at one point on time i will be able to find at least the following manuals
`1300 CPU Technical Manual`
` M1300 Series CPU Organisation and Description Reference Manual`

## Compile
It should compile on any linux system (for Windows as well) using make or ./mkwin32.
There are some tests/samples in the folder testmd


