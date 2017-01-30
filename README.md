OpENer-Lib Version 0.1
======================

*ITS NOT WORKING YET*
---------------------

Welcome to OpENer-lib!
---------------------

OpENer-lib is a library based on OpENer (https://github.com/EIPStackGroup/OpENer) that implements the Common Industrial Protocol&trade;.
It supports (or will support) multiple I/O and explicit connections; includes objects and services to make EtherNet/IP&trade; and DeviceNet&trade;
compliant products defined in ODVA's specifications (http://www.odva.org).

OpENer-lib objectives are to port the OpENer to C++, reduce code coupling, add support to DeviceNET (using Linux's SocketCAN), 
increase maintainability and readability.


Requirements:
-------------
OpENer-lib has been developed to be portable, readable and easy to modify.
The default version targets PCs with a POSIX operating system and a BSD-socket network interface. 
To test this version we recommend a PC with Linux or Windows installed.
You will need to have the following installed:

* CMake
* gcc/mingw/clang/visual c++
* make
* binutils 
 
for normal building. 

For the development, its recommended to use an IDE that supports CMAKE based projects, like Visual Studio 2017 or Jetbrains CLion.

For configuring the project we recommend the use of a CMake GUI (e.g.,the cmake-gui oackage on Linux)

Compiling:
----------------
1. Directly in the shell:
	1. Go into the source directory
	2. For a standard configuration invoke cmake (GUI or shell application) and configure your project
	3. Invoke make
	4. For invoking opener type:

		./opener ipaddress subnetmask gateway domainname hostaddress macaddress

		e.g. ./opener 192.168.0.2 255.255.255.0 192.168.0.1 test.com testdevice 00 15 C5 BF D0 87
 
2. Within Visual Studio or CLion
	1. Open the source folder as a project
	2. Set OpENer to be built than build
	3. The resulting library will be in the directory /bin or the directoy you have choosen via CMake


Directory structure:
--------------------
- bin ...  The resulting binaries and make files for different ports
- doc ...  Doxygen generated documentation (has to be generated for the SVN version) and Coding rules
- data ... EDS file for the default application
- source

		
