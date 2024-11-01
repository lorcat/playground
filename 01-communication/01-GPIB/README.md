## Purpose
Test of communication with Keithley 6485 (6487) using GPIB bus.

## Notes

[Keythley 6485](https://www.tek.com/en/products/keithley/low-level-sensitive-and-specialty-instruments/series-6400-picoammeters) with [manual](https://download.tek.com/manual/6487-900-01(C-Mar2011)(User).pdf?_gl=1*epq0bz*_gcl_au*MTQzOTU0NDMyNS4xNzMwNDUwNTIz*_ga*Mjc5MTg5ODc2LjE3MzA0NTA1MjM.*_ga_1HMYS1JH9M*MTczMDQ1MDUyMy4xLjEuMTczMDQ1MTMzMi4yOS4wLjA.) is a picoammeter with external output.

Reference manual can be found [here](https://www.tek.com/-/media/documents/6487-901-01d_oct2020.pdf).

The code was run to test compability of pyvisa installation with the device. Code also puts the device back to local mode.

Communication statistics can be found below.

    # average times
    Average time for writing ( 0.016 s)
    Average time for reading ( 0.015 s)

    # gain values translated - only these values are accepted
    Setting the gain (8); write value (0.000000002);
    Setting the gain (7); write value (0.00000002);
    Setting the gain (6); write value (0.0000002);
    Setting the gain (5); write value (0.000002);
    Setting the gain (4); write value (0.00002);
    Setting the gain (3); write value (0.0002);
    Setting the gain (2); write value (0.002);
    Setting the gain (1); write value (0.02);

## Useful links
At this [link](https://linux-gpib.sourceforge.io/doc_html/gpib-protocol.html) one can see codes for bus commands.
Might be helpful for some.
