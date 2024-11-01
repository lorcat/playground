"""
Code to test the limits of Keithey 6485/6487 for picoammeter

Average time for writing ( 0.016 s)
Average time for reading ( 0.015 s)

# gain values translated
Setting the gain (8); write value (0.000000002);
Setting the gain (7); write value (0.00000002);
Setting the gain (6); write value (0.0000002);
Setting the gain (5); write value (0.000002);
Setting the gain (4); write value (0.00002);
Setting the gain (3); write value (0.0002);
Setting the gain (2); write value (0.002);
Setting the gain (1); write value (0.02);

Exceptions (e.g. VisaIOError) can be found under pyvisa.errors.* 
"""

import pyvisa
import time

rm = pyvisa.ResourceManager()

tdgains = {
    8: "0."+8*"0"+"2",
    7: "0."+7*"0"+"2",
    6: "0."+6*"0"+"2",
    5: "0."+5*"0"+"2",
    4: "0."+4*"0"+"2",
    3: "0."+3*"0"+"2",
    2: "0."+2*"0"+"2",
    1: "0."+1*"0"+"2",
}


error = None
try:
    inst = rm.open_resource('GPIB::14::INSTR')
    test = inst.query("*IDN?")
    test = inst.write("*CLS")
    test = inst.write("SYST:ZCH OFF")           # turn off zero check
    test = inst.write("SYST:AZER:STAT OFF")     # turn off auto zero
    test = inst.write("DISP:ENAB ON")           # enable display
    

    print(inst.write("RANG?"))
    print(inst.read())

    write_ops = []
    read_ops = []
    msgs = []

    for gain, write in tdgains.items():
        msgs.append(f"Setting the gain ({gain}); write value ({write});")
        print(msgs[-1])

        ts = time.time()
        print(inst.write(f"RANG {write}"))
        ts1 = time.time()
        print(inst.write("RANG?"))
        print(inst.read())

        tref = time.time()
        dt = tref-ts
        dt1 = tref-ts1

        write_ops.append(dt)
        read_ops.append(dt1)
        
        print(f"Write operation took ({dt:6.3f} s)")
        print(f"Read operation took ({dt1:6.3f} s)")

        time.sleep(1)

    print(f"Average time for writing ({sum(write_ops)/len(write_ops):6.3f} s)")
    print(f"Average time for reading ({sum(read_ops)/len(read_ops):6.3f} s)")

    print("\r\n", "\r\n".join(msgs))

    
    # returns the device to the local mode
    inst.control_ren(0)
except pyvisa.errors.VisaIOError as e:
    error = e
except pyvisa.errors.VisaTypeError as e:
    error = e
except pyvisa.errors.UnknownHandler as e:
    error = e
except pyvisa.errors.OSNotSupported as e:
    error = e
except pyvisa.errors.InvalidBinaryFormat as e:
    error = e
except pyvisa.errors.InvalidSession as e:
    error = e
except pyvisa.errors.LibraryError as e:
    error = e  
finally:
    print(type(error), error)
