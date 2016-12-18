.AUTODEPEND

#		*Translator Definitions*
CC = bcc +DEVNET.CFG
TASM = TASM
TLIB = tlib
TLINK = tlink
LIBPATH = D:\BORLANDC\LIB
INCLUDEPATH = D:\BORLANDC\INCLUDE


#		*Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#		*List Macros*


EXE_dependencies =  \
 devnet.obj

#		*Explicit Rules*
devnet.exe: devnet.cfg $(EXE_dependencies)
  $(TLINK) /v/s/c/d/P-/L$(LIBPATH) @&&|
c0l.obj+
devnet.obj
devnet,devnet
emu.lib+
mathl.lib+
cl.lib
|


#		*Individual File Dependencies*
devnet.obj: devnet.cfg devnet.cpp 

#		*Compiler Configuration File*
devnet.cfg: devnet.mak
  copy &&|
-ml
-3
-ff-
-j20
-g20
-v
-y
-G
-wamb
-w-par
-wasm
-wpro
-wdef
-wnod
-w-aus
-wstv
-weas
-wpre
-I$(INCLUDEPATH)
-L$(LIBPATH)
| devnet.cfg


