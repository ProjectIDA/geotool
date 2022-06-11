INCLUDES = -I../../include

LIBS = -linterp -lmagnitude -lLP -lloc -lgdetect -lgamp -lgeog -laesir

libdir = @libdir@/plugins

noinst_HEADERS = \
	AmpMag.h \
	Arrivals.h \
	MeasureAmpPer.h \
	SearchParam.h \
	Stassocs.h

lib_LTLIBRARIES = libgarrival.la

libgarrival_la_SOURCES = \
	AmpMag.cpp \
	Arrivals.cpp \
	libgarrival.cpp \
	MeasureAmpPer.cpp \
	SearchParam.cpp \
	Stassocs.cpp

