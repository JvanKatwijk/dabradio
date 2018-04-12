#
/*
 *    Copyright (C) 2013, 2014, 2015, 2016, 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the dabradio 
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
#include	<QDebug>
#include	<QDateTime>
#include	<QFile>
#include	<QDir>
#include	"dab-constants.h"
#include	<numeric>
#include	<unistd.h>
#include	<vector>
#include	"radio.h"
#include	"band-handler.h"
#include	"audiosink.h"
#include	<mutex>
/**
  *	We use the creation function merely to set up the
  *	user interface and make the connections between the
  *	gui elements and the handling agents. All real action
  *	is initiated by gui buttons
  */

	RadioInterface::RadioInterface (QSettings	*Si,
	                                QString		serviceNames,
	                                bandHandler	*theBand,
	                                virtualInput	*theDevice,
	                                QWidget		*parent):
	                                        QMainWindow (parent) {
int16_t	latency;
int16_t k;
QString h;
int	gain;

	dabSettings		= Si;
	inputDevice		= theDevice;
	this	-> theBand	= theBand;
	running. store (false);
	scanning		= false;
	isSynced		= UNSYNCED;
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

	gain		=
		   dabSettings	-> value ("gain", 90). toInt ();
	gainSlider	-> setValue (gain);
	gainvalueDisplay	-> display (gain);

	autogain =
	           dabSettings	-> value ("autogain", 0). toInt ();

	setColor (autogainButton, autogain == 0);
	syncedLabel		->
	               setStyleSheet ("QLabel {background-color : red}");
	strength_0_label	->
	               setStyleSheet ("QLabel {background-color : red}");
	strength_1_label	->
	               setStyleSheet ("QLabel {background-color : red}");
	strength_2_label	->
	               setStyleSheet ("QLabel {background-color : red}");
	strength_3_label	->
	               setStyleSheet ("QLabel {background-color : red}");
	strength_4_label	->
	               setStyleSheet ("QLabel {background-color : red}");
//
	motLabel		->
	               setStyleSheet ("QLabel {background-color : red}");

//	display the version
	QString v = "j-radio -" + QString (CURRENT_VERSION);
	QString versionText = "dabradio version: " + QString(CURRENT_VERSION);
        versionText += " Build on: " + QString(__TIMESTAMP__) + QString (" ") + QString (GITHASH);
	versionText += "\nCopyright 2018 J van Katwijk, Lazy Chair Computing.";

	copyrightLabel		-> setToolTip (versionText);

	ensembleDisplay	= new serviceList (this, serviceNames);
	ensembleDisplay	-> show ();

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
	connect (streamoutSelector, SIGNAL (activated (int)),
	         this,  SLOT (set_streamSelector (int)));
	ficBlocks		= 0;
	ficSuccess		= 0;
	pictureLabel		= NULL;
//	
//	and start the timer(s)
//	The displaytimer is there to show the number of
//	seconds running 
	displayTimer. setInterval (1000);
	numberofSeconds		= 0;
//
//	timer for channel settings
	channelTimer. setSingleShot (true);
	channelTimer. setInterval   (5000);
//	timer for scanning
	signalTimer. setSingleShot (true);
	signalTimer. setInterval (5000);
	selectedChannel	= QString ("");
//
	my_dabProcessor	= new dabProcessor (this,
	                                    inputDevice,
	                                    dabMode,
	                                    threshold, diff_length,
                                            picturesPath);
	connect (my_dabProcessor, SIGNAL (setSynced (char)),
                 this, SLOT (setSynced (char)));

//
	serviceDescriptor *ss = new serviceDescriptor (" ", " ");
	services. resize (1);
	services [0] = ss;	// a dummy
//
	connect	(gainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_gainSlider (int)));
	connect	(autogainButton, SIGNAL (clicked (void)),
	         this, SLOT (handle_autoButton (void)));

	startScanning ();
}

	RadioInterface::~RadioInterface (void) {
	fprintf (stderr, "radioInterface is deleted\n");
}
//
//	A little tricky, there are two signals that may trigger nextChannel
//	The "no dab is here" signal, which should arrive within
//	a second or so after starting the decoding,
//	or a timeout
void	RadioInterface:: startScanning (void) {
	connect (&signalTimer, SIGNAL (timeout (void)),
	         this, SLOT (nextChannel (void)));
        connect (my_dabProcessor, SIGNAL (No_Signal_Found (void)),
                 this, SLOT (nextChannel (void)));
	disconnect (ensembleDisplay,
	            SIGNAL (newService (const QString &, const QString &)),
	            this, SLOT (selectService (const QString &, const QString &)));
	serviceCount	= 0;
	serviceCountDisplay -> display (serviceCount);
	channelNumber = 0;
	while (channelNumber < theBand -> channels ()) {
	   QString channel = theBand -> channel (channelNumber);
	   if (dabSettings -> value (channel, 1). toInt () > 0) {
	      dabSettings -> setValue (channel, -1);
	      break;
	   }
	   channelNumber ++;
	}

	QString text = "scanning ch ";
	text. append (theBand -> channel (channelNumber. load ()));
	set_ensembleName (text);
	ensembleDisplay	-> reset ();
	services. resize (1);
	serviceDescriptor *ss = new serviceDescriptor (" ", " ");
	services [0] = ss;	// a dummy
	int tunedFrequency	=
	        theBand -> Frequency (channelNumber. load ());
	my_dabProcessor -> start (tunedFrequency, true);
	running. store (true);
	signalTimer. start (5000);
	scanning	= true;
	serviceLabel -> setText ("Wait for services");
	serviceLabel -> setStyleSheet ("QLabel {background-color : red}");
}

void	RadioInterface::nextChannel (void) {
	disconnect (&signalTimer, SIGNAL (timeout (void)),
	            this, SLOT (nextChannel (void)));
	disconnect (my_dabProcessor, SIGNAL (No_Signal_Found (void)),
                    this, SLOT (nextChannel (void)));
	signalTimer. stop ();
	my_dabProcessor -> stop ();
	channelNumber++;
	while (channelNumber < theBand -> channels ()) {
	   QString channel = theBand -> channel (channelNumber);
	   if (dabSettings -> value (channel, 1). toInt () > 0) {
	      dabSettings -> setValue (channel, -1);
	      int tunedFrequency  =
                 theBand -> Frequency (channelNumber);
	      QString text = "scanning ch ";
	      text. append (theBand -> channel (channelNumber));
	      set_ensembleName (text);
	      connect (&signalTimer, SIGNAL (timeout (void)),
	               this, SLOT (nextChannel (void)));
              connect (my_dabProcessor, SIGNAL (No_Signal_Found (void)),
                       this, SLOT (nextChannel (void)));
	      my_dabProcessor	-> start (tunedFrequency, true);
	      signalTimer. start (5000);
	      return;
	   }
	   channelNumber ++;
	}

	scanning = false;	
	set_ensembleName ("end of scan");
	serviceLabel -> setText ("select a services");
	serviceLabel -> setStyleSheet ("QLabel {background-color : green}");
	connect (ensembleDisplay,
	         SIGNAL (newService (const QString &, const QString &)),
	         this, SLOT (selectService (const QString &, const QString &)));
	connect (resetButton, SIGNAL (clicked (void)),
	         this, SLOT (reset (void)));
}

void	RadioInterface::reset (void) {
	my_dabProcessor	-> stop ();
	disconnect (ensembleDisplay,
	         SIGNAL (newService (const QString &, const QString &)),
	         this, SLOT (selectService (const QString &, const QString &)));
	disconnect (resetButton, SIGNAL (clicked (void)),
	            this, SLOT (reset (void)));
	for (int i = 0; i < theBand -> channels (); i ++) {
	   QString channel = theBand -> channel (i);
	   dabSettings -> setValue (channel, 1);
	}
	startScanning ();
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
	   serviceCountDisplay	-> display (serviceCount ++);
	   audiodata d;
	   QString t	= s;
	   my_dabProcessor -> dataforAudioService (t, &d, 0);
	   serviceDescriptor *service;
	   if (d. defined) {
	      QString channel = theBand -> channel (channelNumber);
	      service =
	           new serviceDescriptor (t,
	                                  channel,
	                                  &d);
	      services. push_back (service);
	      ensembleDisplay -> addRow (t,
	                                 channel,
	                                 QString::number (d. bitRate),
	                                 service -> programType);
	      dabSettings	-> setValue (channel, 1);
	   }
	}
}
//
///	a slot, called by the fib processor
void	RadioInterface::nameofEnsemble (int id, const QString &v) {
QString s;
	(void)v;
	ensembleName		-> setText (v);
	my_dabProcessor	-> coarseCorrectorOff ();
}

void	RadioInterface::set_ensembleName (const QString &v) {
	ensembleName	-> setText (v);
}

///////////////////////////////////////////////////////////////////////

void	RadioInterface::show_ficSuccess (bool b) {
	if (b)
	   ficSuccess ++;
	if (++ficBlocks >= 100) {
	   switch ((ficSuccess + 10) / 20) {
	      default:			// should not happen
	      case 0:	// 5 red blocks
	         setColor (strength_0_label, 0);
	         setColor (strength_1_label, 0);
	         setColor (strength_2_label, 0);
	         setColor (strength_3_label, 0);
	         setColor (strength_4_label, 0);
	         break;
	      case 1:	// 1 green, four red
	         setColor (strength_0_label, 1);
	         setColor (strength_1_label, 0);
	         setColor (strength_2_label, 0);
	         setColor (strength_3_label, 0);
	         setColor (strength_4_label, 0);
	         break;
	      case 2:	// 2 green, three red
	         setColor (strength_0_label, 1);
	         setColor (strength_1_label, 1);
	         setColor (strength_2_label, 0);
	         setColor (strength_3_label, 0);
	         setColor (strength_4_label, 0);
	         break;
	      case 3:	// 3 green, two red
	         setColor (strength_0_label, 1);
	         setColor (strength_1_label, 1);
	         setColor (strength_2_label, 1);
	         setColor (strength_3_label, 0);
	         setColor (strength_4_label, 0);
	         break;
	      case 4:	// 4 green, one red
	         setColor (strength_0_label, 1);
	         setColor (strength_1_label, 1);
	         setColor (strength_2_label, 1);
	         setColor (strength_3_label, 1);
	         setColor (strength_4_label, 0);
	         break;
	      case 5:	// 5 green
	         setColor (strength_0_label, 1);
	         setColor (strength_1_label, 1);
	         setColor (strength_2_label, 1);
	         setColor (strength_3_label, 1);
	         setColor (strength_4_label, 1);
	         break;
	   }
	   ficSuccess	= 0;
	   ficBlocks	= 0;
	}
}

void	RadioInterface::setColor (QLabel *l, uint8_t b) {
	if (b)
	   l -> setStyleSheet ("QLabel {background-color : green}");
	else
	   l -> setStyleSheet ("QLabel {background-color : red}");
}

void	RadioInterface::setColor (QPushButton *l, uint8_t b) {
	if (b)
	   l -> setStyleSheet ("QPushButton {background-color : green}");
	else
	   l -> setStyleSheet ("QPushButton {background-color : red}");
}

void	RadioInterface::show_motHandling	(bool b) {
	setColor (motLabel, b);
}

void	RadioInterface::show_frameErrors	(int e) {
	(void)e;
}

void	RadioInterface::show_rsErrors		(int e) {
	(void)e;
}

void	RadioInterface::show_aacErrors		(int e) {
	(void)e;
}

///	just switch a color, obviously GUI dependent, but called
//	from the ofdmprocessor
void	RadioInterface::setSynced	(char b) {
	if (isSynced == b)
	   return;

	isSynced = b;
	setColor (syncedLabel, b == SYNCED);
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
	   my_dabProcessor	-> reset ();
	}
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

////////////////////////////////////////////////////////////////////////////

/**
  *	\brief TerminateProcess
  *	Pretty critical, since there are many threads involved
  *	A clean termination is what is needed, regardless of the GUI
  */
void	RadioInterface::TerminateProcess (void) {
	running. store (false);
	displayTimer. stop ();
	signalTimer.  stop ();

	my_dabProcessor	-> stop ();		// definitely concurrent
	soundOut	-> stop ();
//	everything should be halted by now
	delete		soundOut;
	if (inputDevice != NULL)
	   delete	inputDevice;
	fprintf (stderr, "going to delete dabProcessor\n");
	delete	my_dabProcessor;
	fprintf (stderr, "deleted dabProcessor\n");
	if (ensembleDisplay != NULL)
	   delete	ensembleDisplay;
	if (pictureLabel != NULL)
	   delete pictureLabel;
	pictureLabel = NULL;		// signals may be pending, so careful
	close ();
	fprintf (stderr, ".. end the radio silences\n");
}


void	RadioInterface::updateTimeDisplay (void) {
	time_t now = time (0);
	char * dt = ctime (&now);
	timeDisplay	-> setText (QString (dt));
}
//
//	Signals from the GUI
/////////////////////////////////////////////////////////////////////////

//	Selecting a service is easy, the fib is asked to
//	hand over the relevant data in two steps
void	RadioInterface::selectService (const QString &currentProgram,
	                               const QString &channel) {
	if (scanning)	// be polite, wait until we finished scanning
	   return;

	running. store (true);
	serviceLabel	-> setStyleSheet ("QLabel {background-color : white}");
	serviceLabel	-> setText (currentProgram);
	services [0] = new serviceDescriptor (currentProgram, channel);
	fprintf (stderr, "channel = %s, currentChannel %s\n",
	          channel. toLatin1 (). data (),
	          selectedChannel. toLatin1 (). data ());
	if (selectedChannel != channel) {
	   my_dabProcessor	-> stop ();
	   connect (&channelTimer, SIGNAL (timeout (void)),
	                    this, SLOT (channelTimer_timeout (void)));
	   channelTimer. start (5000);
	   int tunedFrequency	= theBand -> Frequency (channel);
	   my_dabProcessor	-> start (tunedFrequency, false);
	   selectedChannel = channel;
	   fprintf (stderr, "ready to start %s (%s)\n",
	                        currentProgram. toLatin1 (). data (),
	                        channel. toLatin1 (). data ());
	}
	else
	   selectService (currentProgram);
}

void	RadioInterface::channelTimer_timeout (void) {
//	so, we were looking for a channel. Let us see if
//	it has provided us with data
QString currentService = services [0] -> name;
	disconnect (&channelTimer, SIGNAL (timeout (void)),
	                    this, SLOT (channelTimer_timeout (void)));
	if (my_dabProcessor -> kindofService (currentService) != 
	                                                   AUDIO_SERVICE)
	   return;
	selectService (currentService);
}

void	RadioInterface::selectService (QString currentService) {
	my_dabProcessor -> reset_msc ();
	setStereo (false);

	audiodata d;
	my_dabProcessor -> dataforAudioService (currentService, &d);
	if (!d. defined) {
	   QMessageBox::warning (this, tr ("Warning"),
 	                        tr ("insufficient data to run this service\n"));
	   return;
	}

	my_dabProcessor -> set_audioChannel (&d, audioBuffer, dataBuffer);
	soundOut	-> restart ();
	showLabel (QString (" "));

	if (pictureLabel != NULL)
	   delete pictureLabel;
	pictureLabel = NULL;
}
//

void	RadioInterface:: set_streamSelector (int k) {
	((audioSink *)(soundOut)) -> selectDevice (k);
}
//
void	RadioInterface::handle_gainSlider (int k) {
	inputDevice		-> set_Gain (k);
	gainvalueDisplay	-> display (k);
	dabSettings		-> setValue ("gain", k);
}

void	RadioInterface::handle_autoButton (void) {
	autogain	= !autogain;

	setColor (autogainButton, autogain == 0);
	inputDevice	-> set_autoGain (autogain != 0);
	dabSettings	-> setValue ("autogain", autogain);
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


