# ---------------------------------------------------------------------------
VERSION = BCB.01
# ---------------------------------------------------------------------------
!ifndef BCB
BCB = $(MAKEDIR)\..
!endif
# ---------------------------------------------------------------------------
PROJECT = RexecShell.exe
OBJFILES = RexecShell.obj Main.obj LoginData.obj Parform.obj Exclude.obj \
   Include.obj About.obj Client.obj inSocket.obj Encryption.obj RexecClient.obj \
   NetState.obj Details.obj cluma_c.obj Environment.obj IndividualConfig.obj \
   ProcWindow.obj PluginManager.obj FileFunctions.obj AccountManager.obj \
   RexHelp.obj MsgBox.obj ConfigDlg.obj
RESFILES = RexecShell.res
RESDEPEN = $(RESFILES) Main.dfm LoginData.dfm Parform.dfm Exclude.dfm \
   Include.dfm About.dfm MsgBox.dfm ConfigDlg.dfm
LIBFILES =
DEFFILE = 
# ---------------------------------------------------------------------------
CFLAG1 = -O2 -Hc -w -d -k- -r -vi- -tWM -c -a4 -b- -w-par -w-inl -Vx -Ve -x \
   -w-aus
CFLAG2 = -D-w-aus -I$(BCB)\projects;$(BCB)\include;$(BCB)\include\vcl \
   -H=$(BCB)\lib\vcl.csm 
PFLAGS = -AWinTypes=Windows;WinProcs=Windows;DbiTypes=BDE;DbiProcs=BDE;DbiErrs=BDE \
   -D-w-aus -U$(BCB)\projects;$(BCB)\lib\obj;$(BCB)\lib \
   -I$(BCB)\projects;$(BCB)\include;$(BCB)\include\vcl -H -W -$U -$S -$R -$Q -$L- \
   -$D- -JPHNV -M
RFLAGS = -D-w-aus -i$(BCB)\projects;$(BCB)\include;$(BCB)\include\vcl 
LFLAGS = -L$(BCB)\projects;$(BCB)\lib\obj;$(BCB)\lib -aa -Tpe -x -V4.0 
IFLAGS =-g 
LINKER = tlink32
# ---------------------------------------------------------------------------
ALLOBJ = c0w32.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) vcl.lib import32.lib cw32mt.lib ws2_32.lib
# ---------------------------------------------------------------------------
.autodepend

$(PROJECT): $(OBJFILES) $(RESDEPEN) $(DEFFILE)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES)
!

.pas.hpp:
    $(BCB)\BIN\dcc32 $(PFLAGS) { $** }

.pas.obj:
    $(BCB)\BIN\dcc32 $(PFLAGS) { $** }

.cpp.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $*

.c.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $**

.rc.res:
    $(BCB)\BIN\brcc32 $(RFLAGS) $<
#-----------------------------------------------------------------------------
