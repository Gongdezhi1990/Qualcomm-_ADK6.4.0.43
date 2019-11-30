Overview
--------

This package contains kalaccess.dll which provides various functions to
access and control the Kalimba core via a debug transport.


Compiler Support
----------------

The kalaccess library was built with Visual Studio 2015 Update 3 in 32-bit
and 64-bit release modes, using the multi-threaded DLL CRT (/MD, the default); the
example_app code was tested with the same.

At runtime the Visual C++ 2015 redistributable must be available on the user's
machine.


File list
---------

example_app/*      A simple Visual Studio 2015 example project.

common.h           Define the interface of the functions in the DLL.
highlevel.h

The following files have 32-bit and 64-bit variants, in the win32 and win64
subdirectories:

kalaccess_dll.lib  Import library to simplify linking to the DLL from a Visual
                   Studio C/C++ project.

kalaccess.dll      The kalaccess library.

EngineFrameworkCpp.dll  Support libraries for kalaccess, implementing the various
HydProtocols.dll        debug transports that are available. These must be loadable
Hydra_User.dll          at run time by the system. Place them in the same directory
kallockd.exe            as kalaccess.dll.
PtTransport.dll
PtUsbSpi.dll
spikalsim.dll
securlib.dll
libcrypto-1_1.dll or libcrypto-1_1-x64.dll
tctrans.dll


Building and Running the Example
--------------------------------

1. Load the example_app/example_app.vcxproj file into Visual Studio.
2. Press F5. The DLLs in the appropriate win32/win64 directory should automatically
   be loaded: the included minimal example_app.vcxproj.user file sets the debugger
   working directory.
