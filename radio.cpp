#
/*
 *    Copyright (C) 2013, 2014, 2015, 2016, 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the dabradio (formerly SDR-J, JSDR).
 *    dabradio is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    dabradio is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with dabradio; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	<QSettings>
#include	<QMessageBox>
#include	<QFileDialog>
#include	<QDebug>
#include	<QDateTime>
#include	<QFile>
#include	<QStringList>
#include	<QStringListModel>
#include	<QDir>
#include	<fstream>
#include	"dab-constants.h"
#include	<iostream>
#include	<numeric>
#include	<unistd.h>
#include	<vector>
#include	"radio.h"
#include	"band-handler.h"
#include	"wavfiles.h"
#include	"rawfiles.h"
#include	"audiosink.h"
#include	"spectrum-handler.h"
#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
#ifdef	HAVE_SDRPLAY
#include	"sdrplay-handler.h"
#endif
#ifdef	HAVE_AIRSPY
#include	"airspy-handler.h"
#endif
#include	"ui_technical_data.h"

std::vector<size_t> get_cpu_times (void) {
	std::ifstream proc_stat ("/proc/stat");
	proc_stat. ignore (5, ' ');    // Skip the 'cpu' prefix.
	std::vector<size_t> times;
	for (size_t time; proc_stat >> time; times. push_back (time));
	return times;
}
 
bool get_cpu_times (size_t &idle_time, size_t &total_time) {
	const std::vector <size_t> cpu_times = get_cpu_times ();
	if (cpu_times. size () < 4)
           return false;
	idle_time  = cpu_times [3];
	total_time = std::accumulate (cpu_times. begin (), cpu_times. end (), 0);
	return true;
}
/**
  *	We use the creation function merely to set up the
  *	user interface and make the connections between the
  *	gui elements and the handling agents. All real action
  *	is initiated by gui buttons
  */

	RadioInterface::RadioInterface (QSettings	*Si,
	                                bool		rawFile_flag,
	                                QWidget		*parent):
	                                        QMainWindow (parent) {
int16_t	latency;
int16_t k;
QString h;

	dabSettings		= Si;
	this	-> rawFile_flag	= rawFile_flag;
	running. store (false);
	scanning		= false;
	isSynced		= UNSYNCED;
        spectrumBuffer          = new RingBuffer<std::complex<float>> (2 * 32768);
	audioBuffer		= new RingBuffer<int16_t>(16 * 32768);

/**	threshold is used in the phaseReference class 
  *	as threshold for checking the validity of the correlation result
  *	3 is a reasonable value
  */
	threshold	=
	           dabSettings -> value ("threshold", 3). toInt ();
//
//	latency is used to allow different settings for different
//	situations wrt the output buffering. For windows and the RPI
//	this need to be a pretty large value (e.g. 5 to 10)
	latency		=
	           dabSettings -> value ("latency", 5). toInt ();
	diff_length	=
	           dabSettings	-> value ("diff_length", DIFF_LENGTH). toInt ();
        dabMode		= dabSettings   -> value ("dabMode", 1). toInt ();
	if ((dabMode != 1) && (dabMode != 2))
	   dabMode = 1;

	dataBuffer		= new RingBuffer<uint8_t>(32768);
	currentName		= QString ("");

	saveSlides	= dabSettings -> value ("saveSlides", 1). toInt ();
	showSlides	= dabSettings -> value ("showPictures", 1). toInt ();
	if (saveSlides != 0)
	   set_picturePath ();

///////////////////////////////////////////////////////////////////////////

//	The settings are done, now creation of the GUI parts
	setupUi (this);
//
        QString t       =
                dabSettings     -> value ("dabBand", "VHF Band III"). toString ();
        dabBand         = t == "VHF Band III" ?  BAND_III : L_BAND;
        theBand. setupChannels  (channelSelector, dabBand);

	dataDisplay	= new QFrame (NULL);
	techData. setupUi (dataDisplay);
	show_data		= false;
#ifdef	__MINGW32__
	techData. cpuLabel	-> hide ();
	techData. cpuMonitor	-> hide ();
#endif
//
// just sound out
	soundOut		= new audioSink		(latency);

	((audioSink *)soundOut)	-> setupChannels (streamoutSelector);
	streamoutSelector	-> show ();
	bool err;
	h	= dabSettings -> value ("soundchannel", "default"). toString ();
	k	= streamoutSelector -> findText (h);
	if (k != -1) {
	   streamoutSelector -> setCurrentIndex (k);
	   err = !((audioSink *)soundOut) -> selectDevice (k);
	}

	if ((k == -1) || err)
	   ((audioSink *)soundOut)	-> selectDefaultDevice ();
//
//	Showing a spectrum over the WiFi when running the dab software
//	on an RPI 2 may not be a great success
        spectrumHandler = new spectrumhandler (spectrumDisplay,
	                                       64,		// displaySize
	                                       spectrumBuffer);
//
	QPalette p	= techData. ficError_display -> palette ();
	p. setColor (QPalette::Highlight, Qt::green);
	techData. ficError_display	-> setPalette (p);
	techData. frameError_display	-> setPalette (p);
	techData. rsError_display	-> setPalette (p);
	techData. aacError_display	-> setPalette (p);
	techData. rsError_display	-> hide ();
	techData. aacError_display	-> hide ();
	techData. motAvailable		-> 
                           setStyleSheet ("QLabel {background-color : red}");
//
//
	ficBlocks		= 0;
	ficSuccess		= 0;
	pictureLabel		= NULL;
	syncedLabel		->
	               setStyleSheet ("QLabel {background-color : red}");

	ensemble. setStringList (Services);
	ensembleDisplay	-> setModel (&ensemble);
	Services << " ";
	ensemble. setStringList (Services);
	ensembleDisplay	-> setModel (&ensemble);
//
	connect (streamoutSelector, SIGNAL (activated (int)),
	         this,  SLOT (set_streamSelector (int)));
//	
//	display the version
	QString v = "dabradio -" + QString (CURRENT_VERSION);
	QString versionText = "dabradio version: " + QString(CURRENT_VERSION);
        versionText += " Build on: " + QString(__TIMESTAMP__) + QString (" ") + QString (GITHASH);
	versionName	-> setText (v);
	versionName	-> setToolTip (versionText);

//	and start the timer(s)
//	The displaytimer is there to show the number of
//	seconds running 
	displayTimer. setInterval (1000);
	connect (&displayTimer, SIGNAL (timeout (void)),
	         this, SLOT (updateTimeDisplay (void)));
	displayTimer. start (1000);
	numberofSeconds		= 0;
//
//	timer for channel settings
	channelTimer. setSingleShot (true);
	channelTimer. setInterval   (10000);
//	timer for scanning
	signalTimer. setSingleShot (true);
	signalTimer. setInterval (10000);
	connect (&signalTimer, SIGNAL (timeout (void)),
	         this, SLOT (No_Signal_Found (void)));

	inputDevice	= setDevice (" ");
	if (inputDevice == NULL) {
	   fprintf (stderr, "failure, no device\n");
	   exit (1);
	}
	else
	   doStart ();
}
//
//	when doStart is called, a device is available and selected
void	RadioInterface::doStart (void) {
bool	r = 0;
int32_t	frequency;

	QString h       = dabSettings -> value ("channel", "12C"). toString ();
        int k           = channelSelector -> findText (h);
        if (k != -1) {
           channelSelector -> setCurrentIndex (k);
        }
	frequency	= theBand. Frequency (dabBand,
	                                 channelSelector -> currentText ());
        inputDevice     -> setVFOFrequency (frequency);
	r = inputDevice		-> restartReader ();
	qDebug ("Starting %d\n", r);
	if (!r) {
	   QMessageBox::warning (this, tr ("Warning"),
	                               tr ("Opening  input stream failed\n"));
	   return;	// give it another try
	}
//	Some buttons should not be touched before we have a device

	connect (showProgramData, SIGNAL (clicked (void)),
	         this, SLOT (toggle_show_data (void)));
	connect (ensembleDisplay, SIGNAL (clicked (QModelIndex)),
	         this, SLOT (selectService (QModelIndex)));
	connect	(scanButton, SIGNAL (clicked (void)),
	         this, SLOT (set_Scanning (void)));

//	we avoided up till now connecting the channel selector
//	to the slot since that function does a lot more than we
//	want here
	connect (channelSelector, SIGNAL (activated (const QString &)),
	         this, SLOT (selectChannel (const QString &)));
//
//	It is time for some action
	my_dabProcessor = new dabProcessor   (this,
	                                      inputDevice,
	                                      dabMode,
	                                      threshold,
	                                      diff_length,
	                                      picturesPath,
	                                      spectrumBuffer
	                                      );

	clearEnsemble ();		// the display
	my_dabProcessor	-> start ();
	running. store (true);
}

	RadioInterface::~RadioInterface (void) {
	fprintf (stderr, "radioInterface is deleted\n");
}

void	RadioInterface::dumpControlState	(void) {
	dabSettings	-> setValue ("channel",
	                              channelSelector -> currentText ());
	dabSettings	-> setValue ("soundchannel",
	                              streamoutSelector -> currentText ());
}
//
/**
  *	\brief At the end, we might save some GUI values
  *	The QSettings could have been the class variable as well
  *	as the parameter
  */
//
//
/////////////////////////////////////////////////////////////////////////////
//	If the ofdm processor has waited for a period of N frames
//	to get a start of a synchronization,
//	it sends a signal to the GUI handler
//	If "scanning" is "on" we hop to the next frequency on
//	the list

void	RadioInterface::signalTimer_out (void) {
	No_Signal_Found ();
}
//
//	No_Signal_Found might also come from the dabProcessor
void	RadioInterface::No_Signal_Found (void) {
	signalTimer. stop ();
	if (!scanning)
	   return;

//	we stop the thread from running,
//	Increment the frequency
//	and restart
	my_dabProcessor -> stop ();
	while (!my_dabProcessor -> isFinished ())
	   usleep (10000);
	Increment_Channel ();
	my_dabProcessor	-> start ();
	signalTimer. start (10000);
}
//

void	RadioInterface::set_Scanning	(void) {
	setStereo (false);
	if (!running. load ())
	   return;

	if (scanning) {
	   scanning	= false;
	   fprintf (stderr, "scanning will stop\n");
	   my_dabProcessor	-> set_scanMode (false);
	   ensembleId		-> display (0);
	   ensembleName		-> setText (" ");
	   scanButton		-> setText ("Scan band");
	   return;
	}
	
	scanning	= true;
//
//	the vector "services" is used to maintain the list of
//	services in all ensembles that we find when scanning.
//	services [0] is special in that it contains the name
//	of the program being selected from the list.
//	The location is always there when we did have a scan
//	or are scanning
	for (int i = 0; i < services. size (); i ++)
	   delete services [i];
	services. resize (1);
	serviceDescriptor *ss = new serviceDescriptor (" ", " ");
	services [0] = ss;	// a dummy

	clearEnsemble ();
	my_dabProcessor -> set_scanMode (true);
	disconnect (channelSelector, SIGNAL (activated (const QString &)),
	            this, SLOT (selectChannel (const QString &)));
	channelSelector -> setCurrentIndex (0);
	int tunedFrequency	=
	        theBand. Frequency (dabBand, channelSelector -> currentText ());
	inputDevice	-> setVFOFrequency (tunedFrequency);

	connect    (channelSelector, SIGNAL (activated (const QString &)),
	            this, SLOT (selectChannel (const QString &)));
	scanButton -> setText ("scanning");
	signalTimer. start (10000);
}
//
//	Increment channel is called during scanning.
//	The approach taken here is to increment the current index
//	in the combobox and select the new frequency.
//	To avoid disturbance, we disconnect the combobox
//	temporarily, since otherwise changing the channel would
//	generate a signal
void	RadioInterface::Increment_Channel (void) {
int32_t	tunedFrequency;
int	cc	= channelSelector -> currentIndex ();

	cc	+= 1;
	if (cc >= channelSelector -> count ()) {
	   if (scanning) {
	      set_Scanning ();
	      return;
	   }
	   cc = 0;
	}
//	To avoid reaction of the system on setting a different value
	disconnect (channelSelector, SIGNAL (activated (const QString &)),
	              this, SLOT (selectChannel (const QString &)));
	channelSelector -> setCurrentIndex (cc);
	tunedFrequency	=
	         theBand. Frequency (dabBand, channelSelector -> currentText ());
	inputDevice	-> setVFOFrequency (tunedFrequency);

	connect    (channelSelector, SIGNAL (activated (const QString &)),
	              this, SLOT (selectChannel (const QString &)));
}

///////////////////////////////////////////////////////////////////////////
/**
  *	clearEnsemble
  *	on changing settings, we clear all things in the gui
  *	related to the ensemble.
  *	The function is called from "deep" within the handling code
  *	Potentially a dangerous approach, since the fic handler
  *	might run in a separate thread and generate data to be displayed
  */
void	RadioInterface::clearEnsemble	(void) {
//
//	it obviously means: stop processing
	my_dabProcessor	-> clearEnsemble ();
	my_dabProcessor	-> reset ();
	clear_showElements	();
}

//
//	a slot, called by the fic/fib handlers
//	little tricky, since we do not want the data here
//	when setting the channel for selecting a service in
//	the large list
void	RadioInterface::addtoEnsemble (const QString &s) {
	if (!scanning && (services. size () > 0))
	   return;

	if (scanning) {
	   audiodata d;
	   QString t	= s;
	   my_dabProcessor -> dataforAudioService (t, &d, 0);
	   serviceDescriptor *service;
	   if (d. defined) 
	      service =
	           new serviceDescriptor (t,
	                                  channelSelector -> currentText(),
	                                  &d);
	   else
	      service =
	           new serviceDescriptor (t, 
	                                  channelSelector -> currentText ());
	   services. push_back (service);
	}

	Services << s;
	Services. removeDuplicates ();
	ensemble. setStringList (Services);
	ensembleDisplay	-> setModel (&ensemble);
}

//
///	a slot, called by the fib processor
void	RadioInterface::nameofEnsemble (int id, const QString &v) {
QString s;
	(void)v;
	ensembleId		-> display (id);
	ensembleLabel		= v;
	ensembleName		-> setText (v);
	my_dabProcessor	-> coarseCorrectorOff ();
}

///////////////////////////////////////////////////////////////////////

///	called from the ofdmDecoder, which computed this for each frame
void	RadioInterface::show_snr (int s) {
	techData. snrDisplay	-> display (s);
}

//	a slot called by the ofdmprocessor
void	RadioInterface::set_CorrectorDisplay (int v) {
	techData. correctorDisplay	-> display (v);
}
/**
  *	\brief show_successRate
  *	a slot, called by the MSC handler to show the
  *	percentage of frames that could be handled
  */
void	RadioInterface::show_frameErrors (int s) {
	techData. frameError_display	-> setValue (100 - 4 * s);
}

void	RadioInterface::show_rsErrors (int s) {
	techData. rsError_display	-> setValue (100 - 4 * s);
}
	
void	RadioInterface::show_aacErrors (int s) {
	techData. aacError_display	-> setValue (100 - 4 * s);
}
	
void	RadioInterface::show_ficSuccess (bool b) {
	if (b)
	   ficSuccess ++;
	if (++ficBlocks >= 100) {
	   techData. ficError_display	-> setValue (ficSuccess);
	   ficSuccess	= 0;
	   ficBlocks	= 0;
	}
}

void	RadioInterface::show_motHandling (bool b) {
	if (b) {
	   techData. motAvailable -> 
	               setStyleSheet ("QLabel {background-color : green}");
	}
	else {
	   techData. motAvailable ->
	               setStyleSheet ("QLabel {background-color : red}");
	}
}
	
///	just switch a color, obviously GUI dependent, but called
//	from the ofdmprocessor
void	RadioInterface::setSynced	(char b) {
	if (isSynced == b)
	   return;

	isSynced = b;
	switch (isSynced) {
	   case SYNCED:
	      syncedLabel -> 
	               setStyleSheet ("QLabel {background-color : green}");
	      break;

	   default:
	      syncedLabel ->
	               setStyleSheet ("QLabel {background-color : red}");
	      break;
	}
}

//	showLabel is triggered by the message handler
//	the GUI may decide to ignore this
void	RadioInterface::showLabel	(QString s) {
	if (running. load ())
	   dynamicLabel	-> setText (s);
}

void	RadioInterface::setStereo	(bool s) {
	if (s) 
	   stereoLabel -> 
	               setStyleSheet ("QLabel {background-color : green}");

	else
	   stereoLabel ->
	               setStyleSheet ("QLabel {background-color : red}");
}
//
//////////////////////////////////////////////////////////////////////////
//
void	checkDir (QString &s) {
int16_t	ind	= s. lastIndexOf (QChar ('/'));
int16_t	i;
QString	dir;
	if (ind == -1)		// no slash, no directory
	   return;

	for (i = 0; i < ind; i ++)
	   dir. append (s [i]);

	if (QDir (dir). exists ())
	   return;
	QDir (). mkpath (dir);
}
//	showMOT is triggered by the MOT handler,
//	the GUI may decide to ignore the data sent
//	since data is only sent whenever a data channel is selected
void	RadioInterface::showMOT		(QByteArray data,
	                                 int subtype, QString pictureName) {
	const char *type;
	if (!running. load ())
	   return;
	if (pictureLabel == NULL) 
	   pictureLabel	= new QLabel (NULL);

	type = subtype == 0 ? "GIF" :
	       subtype == 1 ? "JPG" :
//	       subtype == 1 ? "JPEG" :
	       subtype == 2 ? "BMP" : "PNG";

	QPixmap p;
	p. loadFromData (data, type);
	if (saveSlides && (pictureName != QString (""))) {
	   pictureName		= QDir::toNativeSeparators (pictureName);
	   FILE *x = fopen (pictureName. toLatin1 (). data (), "w+b");
	   if (x == NULL)
	      fprintf (stderr, "cannot write file %s\n",
	                            pictureName. toLatin1 (). data ());
	   else {
	      fprintf (stderr, "going to write file %s\n",
	                            pictureName. toLatin1 (). data ());
	      (void)fwrite (data. data (), 1, data.length (), x);
	      fclose (x);
	   }
	}

//	pictureLabel -> setFrameRect (QRect (0, 0, p. height (), p. width ()));

	if (showSlides) {
	   pictureLabel ->  setPixmap (p);
	   pictureLabel ->  show ();
	}
}
//
//

/**
  *	\brief changeinConfiguration
  *	No idea yet what to do, so just give up
  *	with what we were doing. The user will -eventually -
  *	see the new configuration from which he can select
  */
void	RadioInterface::changeinConfiguration	(void) {
	if (running. load ()) {
	   soundOut		-> stop ();
	   inputDevice		-> stopReader ();
	   inputDevice		-> resetBuffer ();
	   my_dabProcessor	-> reset ();
	}
	clear_showElements	();
}
//
//	In order to not overload with an enormous amount of
//	signals, we trigger this function at most 10 times a second
//
void	RadioInterface::newAudio	(int amount, int rate) {
	if (running. load ()) {
	   int16_t vec [amount];
	   while (audioBuffer -> GetRingBufferReadAvailable () > amount) {
	      audioBuffer -> getDataFromBuffer (vec, amount);
	      soundOut	-> audioOut (vec, amount, rate);
	   }
	}
}

//
//	This function is only used in the Gui to clear
//	the details of a selection
void	RadioInterface::clear_showElements (void) {
	Services		= QStringList ();
	ensemble. setStringList (Services);
	ensembleDisplay		-> setModel (&ensemble);
	my_dabProcessor		-> clearEnsemble ();

	ensembleLabel		= QString ();
	ensembleName		-> setText (ensembleLabel);
	dynamicLabel		-> setText ("");
	
//	Then the various displayed items
	ensembleName		-> setText ("   ");

	techData. frameError_display	-> setValue (0);
	techData. rsError_display	-> setValue (0);
	techData. aacError_display	-> setValue (0);
	techData. ficError_display	-> setValue (0);
	techData. ensemble 		-> setText (QString (""));
	techData. programName		-> setText (QString (""));
	techData. frequency		-> display (0);
	techData. bitrateDisplay	-> display (0);
	techData. startAddressDisplay	-> display (0);
	techData. lengthDisplay		-> display (0);
	techData. subChIdDisplay	-> display (0);
//	techData. protectionlevelDisplay -> display (0);
	techData. uepField		-> setText (QString (""));
	techData. ASCTy			-> setText (QString (""));
	techData. language		-> setText (QString (""));
	techData. programType		-> setText (QString (""));
	techData. motAvailable		-> 
	               setStyleSheet ("QLabel {background-color : red}");

	techData. snrDisplay		-> display (0);
	if (pictureLabel != NULL)
	   delete pictureLabel;
	pictureLabel = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//	

/**
  *	\brief TerminateProcess
  *	Pretty critical, since there are many threads involved
  *	A clean termination is what is needed, regardless of the GUI
  */
void	RadioInterface::TerminateProcess (void) {
	running. store (false);
#ifdef	DATA_STREAMER
	fprintf (stderr, "going to close the dataStreamer\n");
	delete		dataStreamer;
#endif
	displayTimer. stop ();
	signalTimer.  stop ();

	if (inputDevice != NULL) 
	   inputDevice		-> stopReader ();	// might be concurrent
	if (my_dabProcessor != NULL)
	   my_dabProcessor	-> stop ();		// definitely concurrent
	soundOut		-> stop ();
	dataDisplay		->  hide ();
//	everything should be halted by now
	dumpControlState ();
	delete		soundOut;
	if (inputDevice != NULL)
	   delete		inputDevice;
	fprintf (stderr, "going to delete dabProcessor\n");
	if (my_dabProcessor != NULL)
	   delete		my_dabProcessor;
	fprintf (stderr, "deleted dabProcessor\n");
	delete		dataDisplay;
#ifdef	HAVE_SPECTRUM
	delete	spectrumHandler;
	delete	spectrumBuffer;
#endif
	if (pictureLabel != NULL)
	   delete pictureLabel;
	pictureLabel = NULL;		// signals may be pending, so careful
	close ();
	fprintf (stderr, ".. end the radio silences\n");
}

//
/**
  *	\brief selectChannel
  *	we assume that we are only here whenever the user
  *	tocuhed the channelSelector button
  */
void	RadioInterface::selectChannel (QString s) {
int32_t	tunedFrequency;
bool	localRunning	= running. load ();

	channelTimer. stop ();
	setStereo (false);
	if (scanning)
	   set_Scanning ();	// switch it off
	for (int i = 0; i < services. size (); i ++)
	   delete services [i];
	services. resize (0);

	if (localRunning) {
	   soundOut		-> stop ();
	   inputDevice		-> stopReader ();
	}

	clear_showElements ();

	tunedFrequency	= theBand. Frequency (dabBand, s);
	inputDevice	-> setVFOFrequency (tunedFrequency);

	if (localRunning) {
	   inputDevice		-> restartReader ();
	   my_dabProcessor	-> reset ();
	   running. store (true);
	}
	dabSettings	-> setValue ("channel", s);
}

static size_t previous_idle_time	= 0;
static size_t previous_total_time	= 0;

void	RadioInterface::updateTimeDisplay (void) {
	numberofSeconds ++;
	int16_t	numberHours	= numberofSeconds / 3600;
	int16_t	numberMinutes	= (numberofSeconds / 60) % 60;
	QString text = QString ("runtime ");
	text. append (QString::number (numberHours));
	text. append (" hr, ");
	text. append (QString::number (numberMinutes));
	text. append (" min");
	timeDisplay	-> setText (text);
#ifndef	__MINGW32__
	if ((numberofSeconds % 2) == 0) {
	   size_t idle_time, total_time;
	   get_cpu_times (idle_time, total_time);
	   const float idle_time_delta = idle_time - previous_idle_time;
           const float total_time_delta = total_time - previous_total_time;
           const float utilization = 100.0 * (1.0 - idle_time_delta / total_time_delta);
	   techData. cpuMonitor -> display (utilization);
           previous_idle_time = idle_time;
           previous_total_time = total_time;
	}
#endif
}

void	RadioInterface::autoCorrector_on (void) {
//	first the real stuff
	clear_showElements	();
	my_dabProcessor		-> clearEnsemble ();
	my_dabProcessor		-> coarseCorrectorOn ();
	my_dabProcessor		-> reset ();
}

/**
  *	\brief setDevice
  *	setDevice is called during the start up phase
  *	of the dab program, 
  */
virtualInput	*RadioInterface::setDevice (QString s) {
QString	file;
virtualInput	*inputDevice	= NULL;
///	OK, everything quiet, now let us see what to do
#ifdef	HAVE_AIRSPY
	try {
	   inputDevice	= new airspyHandler (dabSettings);
	   showButtons ();
	   spectrumHandler	-> setBitDepth (inputDevice -> bitDepth ());
	   return inputDevice;
	} catch (int e) {}
#endif
#ifdef	HAVE_SDRPLAY
	try {
	   inputDevice	= new sdrplayHandler (dabSettings);
	   showButtons ();
	   spectrumHandler	-> setBitDepth (inputDevice -> bitDepth ());
	   return inputDevice;
	} catch (int e) {}
#endif
#ifdef	HAVE_RTLSDR
	try {
	   inputDevice	= new rtlsdrHandler (dabSettings);
	   showButtons ();
	   spectrumHandler	-> setBitDepth (inputDevice -> bitDepth ());
	   return inputDevice;
	} catch (int e) {}
#endif
	if (rawFile_flag) {
	   file		= QFileDialog::getOpenFileName (this,
	                                               tr ("Open file ..."),
	                                                QDir::homePath (),
	                                                tr ("raw data (*.iq)"));
	   file		= QDir::toNativeSeparators (file);
	   try {
	      inputDevice	= new rawFiles (file);
	      hideButtons ();
	   } catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("file not found"));
	      return NULL;
	   }
	}
	else {		// apparently ".sdr" files
	   file		= QFileDialog::getOpenFileName (this,
	                                               tr ("Open file ..."),
	                                                QDir::homePath (),
	                                                tr ("raw data (*.sdr)"));
	   file		= QDir::toNativeSeparators (file);
	   try {
	      inputDevice	= new wavFiles (file);
	      hideButtons ();
	   } catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("file not found"));
	      return NULL;
	   }
	}

	spectrumHandler	-> setBitDepth (inputDevice -> bitDepth ());
	return inputDevice;
}

//	Selecting a service is easy, the fib is asked to
//	hand over the relevant data in two steps
void	RadioInterface::selectService (QModelIndex s) {
QString	currentProgram = ensemble. data (s, Qt::DisplayRole). toString ();
	if (services. size () == 0) {	// apparently a channel selected
	   selectService (currentProgram);
	   return;
	}
	if (scanning)	// be polite, wait until we finished scanning
	   return;
//
//	The complex part is if/when we have to select the channel where the
//	service is to be found.
	for (int i = 1; i < services. size (); i ++) {
	   if (services [i] -> name == currentProgram) {
	      QString channel = services [i] -> channel;
	      if (channel == channelSelector -> currentText ()) {
	         selectService (currentProgram);
	         return;
	      }
	      delete services [0];
	      services [0] = new serviceDescriptor (currentProgram,
	                                            services [i] -> channel);
	      disconnect (channelSelector,
	                       SIGNAL (activated (const QString &)),
	                  this, SLOT (selectChannel (const QString &)));
	      int k = channelSelector -> findText (services [i] -> channel);
	      channelSelector -> setCurrentIndex (k);
	      int tunedFrequency	=
	              theBand. Frequency (dabBand, services [i] -> channel);
	      inputDevice	-> setVFOFrequency (tunedFrequency);
	      my_dabProcessor	-> reset ();
	      connect    (channelSelector,
	                  SIGNAL (activated (const QString &)),
	                  this, SLOT (selectChannel (const QString &)));
	      connect (&channelTimer, SIGNAL (timeout (void)),
	               this, SLOT (channelTimer_timeout (void)));
	      channelTimer. start (1000);
	      return;
	   }
	}
	fprintf (stderr, "big failure\n");
}

void	RadioInterface::channelTimer_timeout (void) {
//	so, we were looking for a channel. Let us see if
//	it has provide dus with data
QString currentProgram = services [0] -> name;

	if (my_dabProcessor -> kindofService (currentProgram) != 
	               AUDIO_SERVICE)
	   return;
	selectService (currentProgram);
}

//	Might be called from the GUI as well as from an internal call
void	RadioInterface::selectService (QString s) {
	if ((my_dabProcessor -> kindofService (s) != AUDIO_SERVICE) &&
	    (my_dabProcessor -> kindofService (s) != PACKET_SERVICE))
	return;

	my_dabProcessor -> reset_msc ();
	currentName = s;
	setStereo (false);
//	soundOut	-> stop ();

	dataDisplay	-> hide ();
	techData. rsError_display	-> hide ();
	techData. aacError_display	-> hide ();
	techData. motAvailable		-> 
	               setStyleSheet ("QLabel {background-color : red}");

	int k = my_dabProcessor -> kindofService (s);

	switch (k) {
	   case AUDIO_SERVICE:
	      {  audiodata d;
	         my_dabProcessor -> dataforAudioService (s, &d);
	         if (!d. defined) {
                    QMessageBox::warning (this, tr ("Warning"),
 	                               tr ("unknown bitrate for this program\n"));
 	            return;
 	         }

	         show_techData (ensembleLabel, s, 
	                       (int32_t)(inputDevice -> getVFOFrequency () / 1000000.0),
	                       &d);

	         my_dabProcessor -> set_audioChannel (&d, audioBuffer);
	         for (int i = 1; i < 10; i ++) {
	            packetdata pd;
	            my_dabProcessor -> dataforDataService (s, &pd, i);
	            if (pd. defined) {
	               my_dabProcessor -> set_dataChannel (&pd, dataBuffer);
	               break;
	            }
	         }

	         soundOut	-> restart ();
	         showLabel (QString (" "));
	         break;
	      }

	   case PACKET_SERVICE:
	      {  packetdata pd;
	         my_dabProcessor -> dataforDataService (s, &pd);
	         if ((!pd. defined) ||
	             (pd.  DSCTy == 0) || (pd. bitRate == 0)) {
	            fprintf (stderr, "d. DSCTy = %d, d. bitRate = %d\n",
	                               pd. DSCTy, pd. bitRate);
	            QMessageBox::warning (this, tr ("sdr"),
 	                               tr ("still insufficient data for this service\n"));

	            return;
	         }
	         my_dabProcessor -> set_dataChannel (&pd, dataBuffer);
	         switch (pd. DSCTy) {
	            default:
	               showLabel (QString ("unimplemented Data"));
	               break;
	            case 60:
	               showLabel (QString ("MOT partially implemented"));
	               break;
	         }
	        break;
	      }

	   default:
               QMessageBox::warning (this, tr ("Warning"),
 	                               tr ("unknown service\n"));
	      return;
	}

	if (pictureLabel != NULL)
	   delete pictureLabel;
	pictureLabel = NULL;
}
//

void	RadioInterface::showSpectrum	(int32_t amount) {
	if (spectrumHandler != NULL ) 
	   spectrumHandler -> showSpectrum (amount,
				            inputDevice -> getVFOFrequency ());
}

void	RadioInterface:: set_streamSelector (int k) {
	((audioSink *)(soundOut)) -> selectDevice (k);
}

void	RadioInterface::toggle_show_data (void) {
	show_data	= !show_data;
	if (show_data)
	   dataDisplay -> show ();
	else
	   dataDisplay -> hide ();
}

void	RadioInterface::showButtons	(void) {
	scanButton		-> show ();
	channelSelector		-> show	();
	techData. frequency	-> show ();

}

void	RadioInterface::hideButtons	(void) {
	scanButton		-> hide ();
	channelSelector		-> hide ();
	techData. frequency	-> hide ();
}

void	RadioInterface::setSyncLost	(void) {
}

void	RadioInterface::set_picturePath (void) {
QString defaultPath	= QDir::tempPath ();

	if (defaultPath. endsWith ("/"))
	   defaultPath. append ("qt-pictures/");
	else
	   defaultPath. append ("/qt-pictures/");

	picturesPath	=
	        dabSettings	-> value ("pictures", defaultPath). toString ();

	if ((picturesPath != "") && (!picturesPath. endsWith ("/")))
	   picturesPath. append ("/");
	QDir testdir (picturesPath);

	if (!testdir. exists ())
	   testdir. mkdir (picturesPath);
}

void	RadioInterface::show_techData (QString		ensembleLabel, 
	                               QString 		serviceName, 
	                               int32_t 		Frequency,
	                               audiodata	*d) {
	techData. ensemble	-> setText (ensembleLabel);
	techData. programName	-> setText (serviceName);
	techData. frequency	-> display (Frequency);
	techData. bitrateDisplay -> display (d -> bitRate);
	techData. startAddressDisplay -> display (d -> startAddr);
	techData. lengthDisplay	-> display (d -> length);
	techData. subChIdDisplay -> display (d -> subchId);
	uint16_t h = d -> protLevel;
	QString protL;
	if (!d -> shortForm) {
	   protL = "EEP ";
	   protL. append (QString::number ((h & 03) + 1));
	   if ((h & (1 << 2)) == 0)
	      protL. append ("-A");
	   else
	      protL. append ("-B");
	}
	else  {
	   protL = "UEP ";
	   protL. append (QString::number (h));
	}
	techData. uepField	-> setText (protL);
	techData. ASCTy		-> setText (d -> ASCTy == 077 ? "DAB+" : "DAB");
	if (d -> ASCTy == 077) {
	   techData. rsError_display -> show ();
	   techData. aacError_display -> show ();
	}
	techData. language ->
	   setText (the_textMapper.
	               get_programm_language_string (d -> language));
	techData. programType ->
	   setText (the_textMapper.
	               get_programm_type_string (d -> programType));
	if (show_data)
	   dataDisplay -> show ();
}

#include <QCloseEvent>
void RadioInterface::closeEvent (QCloseEvent *event) {

	QMessageBox::StandardButton resultButton =
	                QMessageBox::question (this, "dabRadio",
                                               tr("Are you sure?\n"),
                                               QMessageBox::No | QMessageBox::Yes,
                                               QMessageBox::Yes);
	if (resultButton != QMessageBox::Yes) {
	   event -> ignore();
	} else {
	   TerminateProcess ();
	   event -> accept ();
	}
}

