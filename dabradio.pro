######################################################################
# Automatically generated by qmake (2.01a) Tue Oct 6 19:48:14 2009
# but modified by me to accomodate for the includes for qwt, hamlib and
# portaudio
######################################################################

TEMPLATE	= app
TARGET		= dabradio-1.0

QT		+= widgets 
CONFIG		+= console
QMAKE_CXXFLAGS	+= -std=c++11
#QMAKE_CFLAGS	+=  -flto -ffast-math
#QMAKE_CXXFLAGS	+=  -flto -ffast-math
#QMAKE_LFLAGS	+=  -flto
QMAKE_CFLAGS	+=  -g
QMAKE_CXXFLAGS	+=  -g
QMAKE_LFLAGS	+=  -g
QMAKE_CXXFLAGS += -isystem $$[QT_INSTALL_HEADERS]
RC_ICONS	=  qt-dab.ico
RESOURCES	+= resources.qrc

TRANSLATIONS = i18n/de_DE.ts i18n/it_IT.ts i18n/hu_HU.ts

DEPENDPATH += . \
	      ./src \
	      ./dab-scope \
	      ./includes \
	      ./src/ofdm \
	      ./src/backend \
	      ./src/backend/viterbi_768 \
	      ./src/backend/audio \
	      ./src/backend/data \
	      ./src/output \
	      ./src/various \
	      ./devices \
	      ./devices/rawfiles \
	      ./devices/wavfiles \
	      ./includes/ofdm \
	      ./includes/backend \
	      ./includes/backend/audio \
	      ./includes/backend/data \
	      ./includes/output \
	      ./includes/various 

INCLUDEPATH += . \
	      ./ \
	      ./src \
	      ./dab-scope \
	      ./includes \
	      ./includes/ofdm \
	      ./includes/backend \
	      ./includes/backend/viterbi_768 \
	      ./includes/backend/audio \
	      ./includes/backend/data \
	      ./includes/output \
	      ./includes/various \
	      ./devices \
	      ./devices/rawfiles \
	      ./devices/wavfiles 

# Input
HEADERS += ./radio.h \
	   ./dab-processor.h \
	   ./dab-scope/spectrum-handler.h \
	   ./includes/dab-constants.h \
	   ./includes/country-codes.h \
	   ./includes/ofdm/sample-reader.h \
	   ./includes/ofdm/ofdm-decoder.h \
	   ./includes/ofdm/phasereference.h \
	   ./includes/ofdm/phasetable.h \
	   ./includes/ofdm/freq-interleaver.h \
#	   ./includes/backend/viterbi.h \
	   ./includes/backend/viterbi_768/viterbi-768.h \
	   ./includes/backend/fic-handler.h \
	   ./includes/backend/msc-handler.h \
	   ./includes/backend/fib-processor.h  \
	   ./includes/backend/galois.h \
	   ./includes/backend/reed-solomon.h \
	   ./includes/backend/rscodec.h \
	   ./includes/backend/charsets.h \
	   ./includes/backend/firecode-checker.h \
	   ./includes/backend/frame-processor.h \
	   ./includes/backend/virtual-backend.h \
	   ./includes/backend/audio-backend.h \
	   ./includes/backend/data-backend.h \
	   ./includes/backend/audio/mp2processor.h \
	   ./includes/backend/audio/mp4processor.h \
	   ./includes/backend/audio/faad-decoder.h \
	   ./includes/backend/data/data-processor.h \
	   ./includes/backend/data/pad-handler.h \
	   ./includes/backend/data/virtual-datahandler.h \
	   ./includes/backend/data/mot-databuilder.h \
	   ./includes/backend/data/mot-data.h \
	   ./includes/backend/protection.h \
	   ./includes/backend/eep-protection.h \
	   ./includes/backend/uep-protection.h \
#	   ./includes/output/fir-filters.h \
	   ./includes/output/audio-base.h \
	   ./includes/output/newconverter.h \
	   ./includes/output/audiosink.h \
           ./includes/various/fft-handler.h \
	   ./includes/various/ringbuffer.h \
	   ./includes/various/Xtan2.h \
	   ./includes/various/dab-params.h \
	   ./includes/various/band-handler.h \
	   ./includes/various/text-mapper.h \
	   ./devices/virtual-input.h \
	   ./devices/rawfiles/rawfiles.h \
           ./devices/wavfiles/wavfiles.h

FORMS	+= ./devices/filereader-widget.ui 
FORMS	+= ./forms/technical_data.ui

SOURCES += ./main.cpp \
	   ./radio.cpp \
	   ./dab-processor.cpp \
	   ./dab-scope/spectrum-handler.cpp \
	   ./src/ofdm/sample-reader.cpp \
	   ./src/ofdm/ofdm-decoder.cpp \
	   ./src/ofdm/phasereference.cpp \
	   ./src/ofdm/phasetable.cpp \
	   ./src/ofdm/freq-interleaver.cpp \
#	   ./src/backend/viterbi.cpp \
	   ./src/backend/viterbi_768/viterbi-768.cpp \
	   ./src/backend/fic-handler.cpp \
	   ./src/backend/msc-handler.cpp \
	   ./src/backend/protection.cpp \
	   ./src/backend/eep-protection.cpp \
	   ./src/backend/uep-protection.cpp \
	   ./src/backend/fib-processor.cpp  \
	   ./src/backend/galois.cpp \
	   ./src/backend/reed-solomon.cpp \
	   ./src/backend/rscodec.cpp \
	   ./src/backend/charsets.cpp \
	   ./src/backend/firecode-checker.cpp \
	   ./src/backend/frame-processor.cpp \
	   ./src/backend/protTables.cpp \
	   ./src/backend/virtual-backend.cpp \
	   ./src/backend/audio-backend.cpp \
	   ./src/backend/data-backend.cpp \
	   ./src/backend/audio/mp2processor.cpp \
	   ./src/backend/audio/mp4processor.cpp \
	   ./src/backend/audio/faad-decoder.cpp \
	   ./src/backend/data/pad-handler.cpp \
	   ./src/backend/data/data-processor.cpp \
	   ./src/backend/data/virtual-datahandler.cpp \
	   ./src/backend/data/mot-databuilder.cpp \
	   ./src/backend/data/mot-data.cpp \
#	   ./src/output/fir-filters.cpp \
	   ./src/output/audio-base.cpp \
	   ./src/output/newconverter.cpp \
	   ./src/output/audiosink.cpp \
           ./src/various/fft-handler.cpp \
	   ./src/various/Xtan2.cpp \
	   ./src/various/dab-params.cpp \
	   ./src/various/band-handler.cpp \
	   ./src/various/text-mapper.cpp \
	   ./devices/virtual-input.cpp \
	   ./devices/rawfiles/rawfiles.cpp \
           ./devices/wavfiles/wavfiles.cpp
#
#	for unix systems this is about it. Adapt when needed for naming
#	and locating libraries. If you do not need a device as
#	listed, just comment the line out.
#
unix {
DESTDIR		= ./linux-bin
exists ("./.git") {
   GITHASHSTRING = $$system(git rev-parse --short HEAD)
   !isEmpty(GITHASHSTRING) {
       message("Current git hash = $$GITHASHSTRING")
       DEFINES += GITHASH=\\\"$$GITHASHSTRING\\\"
   }
}
isEmpty(GITHASHSTRING) {
    DEFINES += GITHASH=\\\"------\\\"
}

FORMS 		+= ./forms/dabradio.ui 
INCLUDEPATH	+= /usr/local/include
INCLUDEPATH	+= /usr/local/include /usr/include/qt4/qwt /usr/include/qt5/qwt /usr/include/qt4/qwt /usr/include/qwt /usr/local/qwt-6.1.4-svn/

LIBS		+= -lfftw3f  -lusb-1.0 -ldl  #
LIBS		+= -lportaudio
LIBS		+= -lz
LIBS		+= -lsndfile
LIBS		+= -lsamplerate
LIBS		+= -lfaad
LIBS		+= -lqwt-qt5

#
# comment or uncomment for the devices you want to have support for
# (you obviously have libraries installed for the selected ones)
CONFIG		+= dabstick
CONFIG		+= sdrplay
CONFIG		+= airspy

#if you want to listen remote, uncomment
#CONFIG		+= tcp-streamer		# use for remote listening
#otherwise, if you want to use the default qt way of soud out
#CONFIG		+= qt-audio
#comment both out if you just want to use the "normal" way

#for the raspberry you definitely want this one
#when this one is enabled, load is spread over different threads
DEFINES	+= __THREADED_BACKEND
#DEFINES	+= __THREADED_DECODING

#and this one is experimental
DEFINES		+= PRESET_NAME

#and these one is just experimental,
#CONFIG	+= NEON
CONFIG	+= SSE
}
#
# an attempt to have it run under W32 through cross compilation
win32 {
#DESTDIR	= ../../../dab-win
DESTDIR		= ../../windows-bin
# includes in mingw differ from the includes in fedora linux

exists ("./.git") {
   GITHASHSTRING = $$system(git rev-parse --short HEAD)
   !isEmpty(GITHASHSTRING) {
       message("Current git hash = $$GITHASHSTRING")
       DEFINES += GITHASH=\\\"$$GITHASHSTRING\\\"
   }
}
isEmpty(GITHASHSTRING) {
    DEFINES += GITHASH=\\\"------\\\"
}

INCLUDEPATH += /usr/i686-w64-mingw32/sys-root/mingw/include
INCLUDEPATH	+= /mingw32/include
INCLUDEPATH	+= /mingw32/include/qwt
LIBS		+= -L/usr/i686-w64-mingw32/sys-root/mingw/lib
LIBS		+= -lfftw3f
LIBS		+= -lportaudio
LIBS		+= -lsndfile
LIBS		+= -lsamplerate
LIBS		+= -lole32
LIBS		+= -lwinpthread
LIBS		+= -lwinmm
LIBS 		+= -lstdc++
LIBS		+= -lws2_32
LIBS		+= -lfaad
LIBS		+= -lusb-1.0
LIBS		+= -lz
FORMS 		+= ./forms/dabradio.ui 
FORMS		+= ./forms/technical_data.ui

CONFIG		+= extio
CONFIG		+= airspy
CONFIG		+= rtl_tcp
CONFIG		+= dabstick
CONFIG		+= sdrplay

CONFIG		+= NO_SSE


#for the raspberry you definitely want this one
#when this one is enabled, load is spread over different threads
DEFINES	+= __THREADED_BACKEND
#DEFINES	+= __THREADED_DECODING


#and this one is experimental
DEFINES		+= PRESET_NAME

}

#	devices
#
#	dabstick
dabstick {
	DEFINES		+= HAVE_RTLSDR
	DEPENDPATH	+= ./devices/rtlsdr-handler
	INCLUDEPATH	+= ./devices/rtlsdr-handler
	HEADERS		+= ./devices/rtlsdr-handler/rtlsdr-handler.h \
	                   ./devices/rtlsdr-handler/rtl-dongleselect.h
	SOURCES		+= ./devices/rtlsdr-handler/rtlsdr-handler.cpp \
	                   ./devices/rtlsdr-handler/rtl-dongleselect.cpp
	FORMS		+= ./devices/rtlsdr-handler/rtlsdr-widget.ui
}
#
#	the SDRplay
#
sdrplay {
	DEFINES		+= HAVE_SDRPLAY
	DEPENDPATH	+= ./devices/sdrplay-handler 
	INCLUDEPATH	+= ./devices/sdrplay-handler 
	HEADERS		+= ./devices/sdrplay-handler/sdrplay-handler.h \
	                   ./devices/sdrplay-handler/sdrplayselect.h
	SOURCES		+= ./devices/sdrplay-handler/sdrplay-handler.cpp \
	                   ./devices/sdrplay-handler/sdrplayselect.cpp
	FORMS		+= ./devices/sdrplay-handler/sdrplay-widget.ui
}
#
#
# airspy support
#
airspy {
	DEFINES		+= HAVE_AIRSPY
	DEPENDPATH	+= ./devices/airspy 
	INCLUDEPATH	+= ./devices/airspy-handler \
	                   ./devices/airspy-handler/libairspy
	HEADERS		+= ./devices/airspy-handler/airspy-handler.h \
	                   ./devices/airspy-handler/airspyfilter.h \
	                   ./devices/airspy-handler/libairspy/airspy.h
	SOURCES		+= ./devices/airspy-handler/airspy-handler.cpp \
	                   ./devices/airspy-handler/airspyfilter.cpp
	FORMS		+= ./devices/airspy-handler/airspy-widget.ui
}

#
rtl_tcp {
	DEFINES		+= HAVE_RTL_TCP
	QT		+= network
	INCLUDEPATH	+= ./devices/rtl_tcp
	HEADERS		+= ./devices/rtl_tcp/rtl_tcp_client.h
	SOURCES		+= ./devices/rtl_tcp/rtl_tcp_client.cpp
	FORMS		+= ./devices/rtl_tcp/rtl_tcp-widget.ui
}

qt-audio	{
	DEFINES		+= QT_AUDIO
	QT		+= multimedia
	HEADERS		+= ./includes/output/Qt-audio.h \
	                   ./includes/output/Qt-audiodevice.h
	SOURCES		+= ./src/output/Qt-audio.cpp \
	                   ./src/output/Qt-audiodevice.cpp
}

NEON	{
	DEFINES		+= NEON_AVAILABLE
	QMAKE_CFLAGS	+=  -mfpu=neon-vfpv4
	QMAKE_CXXFLAGS	+=  -mfpu=neon-vfpv4
	HEADERS		+= ./src/backend/viterbi_768/spiral-neon.h
	SOURCES		+= ./src/backend/viterbi_768/spiral-neon.c
}

SSE	{
	DEFINES		+= SSE_AVAILABLE
	HEADERS		+= ./src/backend/viterbi_768/spiral-sse.h
	SOURCES		+= ./src/backend/viterbi_768/spiral-sse.c
}

NO_SSE	{
	HEADERS		+= ./src/backend/viterbi_768/spiral-no-sse.h
	SOURCES		+= ./src/backend/viterbi_768/spiral-no-sse.c
}

