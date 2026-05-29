AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
  -I$(includedir) \
  -isystem$(OFFLINE_MAIN)/include \
  -isystem$(ROOTSYS)/include

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib \
  -L$(OFFLINE_MAIN)/lib64

pkginclude_HEADERS = \
  AnaConf.h \
  AnaTree.h \
  AnaUtils.h \
  cppBash.h \
  PlotUtils.h \
  sphenix_style.h \
  CentCut.h \
  EventCut.h \
  EventCutReport.h \
  EventSelector.h \
  MinBiasCut.h \
  MissingSebFilter.h \
  TriggerSelect.h \
  ZVertexCut.h \
  CaloSpy.h \
  CaloManip.h

lib_LTLIBRARIES = \
  libdijetana.la 

libdijetana_la_SOURCES = \
  AnaTree.cc \
  AnaConf.cc \
  AnaUtils.cc \
  cppBash.cc \
  PlotUtils.cc \
  sphenix_style.cc \
  CentCut.cc \
  EventCutReport.cc \
  EventSelector.cc \
  MinBiasCut.cc \
  MissingSebFilter.cc \
  TriggerSelect.cc \
  ZVertexCut.cc \
  CaloSpy.cc \
  CaloManip.cc  

libdijetana_la_LIBADD = \
  -lcalo_io \
  -lcalotrigger_io \
  -lcentrality_io \
  -ljetbase \
  -lfun4all \
  -lglobalvertex_io \
  -lglobalvertex \
  -lffarawobjects \
  -ljetbackground_io \
  -ljetbackground \
  -lphhepmc_io \
  -lphool \
  -lepd_io \
  -lSubsysReco

BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS = \
  testexternals

testexternals_SOURCES = testexternals.cc
testexternals_LDADD   = libdijetana.la


testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f $(BUILT_SOURCES)
