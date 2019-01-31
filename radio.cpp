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
#include        <QMouseEvent>
#include	"radio.h"
#include	"band-handler.h"
#include	"audiosink.h"
#include	"audio-descriptor.h"
#include	<mutex>

#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
#ifdef	HAVE_SDRPLAY
#include	"sdrplay-handler.h"
#endif
#ifdef	HAVE_AIRSPY
#include	"airspy-handler.h"
#endif
#ifdef	HAVE_HACKRF
#include	"hackrf-handler.h"
#endif
/**
  *	We use the creation function merely to set up the
  *	user interface and make the connections between the
  *	gui elements and the handling agents. All real action
  *	is initiated by gui buttons
  */

	RadioInterface::RadioInterface (QSettings	*Si,
	                                QString		serviceNames,
	                                bandHandler	*theBand,
	                                QWidget		*parent):
	                                        QMainWindow (parent) {
int16_t	latency;
int16_t k;
QString h;

	dabSettings		= Si;
	this	-> theBand	= theBand;
	channels		= theBand	-> channels ();
	running. store (false);
	scanning		= false;
	isSynced		= UNSYNCED;
	audioBuffer		= new RingBuffer<int16_t>(16 * 32768);
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

	saveSlides	= dabSettings -> value ("saveSlides", 1). toInt ();
	showSlides	= dabSettings -> value ("showPictures", 1). toInt ();
	if (saveSlides != 0)
	   set_picturePath ();

///////////////////////////////////////////////////////////////////////////

//	The settings are done, now creation of the GUI parts
	setupUi (this);

	inputDevice		= setDevice (dabSettings,
	                                     gainSelector,
	                                     lnaSelector,
	                                     agcControl);
	if (inputDevice == NULL) {
	   delete audioBuffer;
	   throw (33);
	}

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

	ensembleDisplay	= new QListView (NULL);
	ensembleDisplay	-> show ();
	ensembleDisplay	-> setToolTip ("Right clicking on a service name will make some technical details on the selected service visible");

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
	displayTimer. start (1000);
	numberofSeconds		= 0;
//
//	timer for channel settings
	channelTimer. setSingleShot (true);
	channelTimer. setInterval   (5000);
//	timer for scanning
	signalTimer. setSingleShot (true);
	selectedChannel	= QString ("");
//
	my_dabProcessor	= new dabProcessor (this,
	                                    inputDevice,
	                                    dabMode,
	                                    threshold,
	                                    diff_length,
                                            picturesPath);
	connect (my_dabProcessor, SIGNAL (setSynced (char)),
                 this, SLOT (setSynced (char)));
	connect (my_dabProcessor, SIGNAL (show_snr (int)),
	         this, SLOT (show_snr (int)));
//
	serviceCharacteristics	= NULL;
	       secondsTimer. setInterval (1000);
        connect (&secondsTimer, SIGNAL (timeout (void)),
                 this, SLOT (updateTime (void)));
        secondsTimer. start (1000);

	startScanning ();
	qApp    -> installEventFilter (this);
	serviceDescription	= NULL;
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
	disconnect (&signalTimer, SIGNAL (timeout (void)),
	            this, SLOT (nextChannel (void)));
	disconnect (my_dabProcessor, SIGNAL (No_Signal_Found (void)),
                    this, SLOT (nextChannel (void)));
	serviceCount	= 0;
	serviceCountDisplay -> display (serviceCount);
	channelNumber = 0;
	while (channelNumber < channels) {
	   QString channel = theBand -> channel (channelNumber);
	   if (dabSettings -> value (channel, 1). toInt () > 0) {
	      dabSettings -> setValue (channel, -1);
	      break;
	   }
	   channelNumber ++;
	}

	if (channelNumber >= theBand -> channels ()) {
	   set_ensembleName ("end of scan");
	   serviceLabel -> setText ("No services found, retry scan");
	   serviceLabel -> setStyleSheet ("QLabel {background-color : green}");
	   disconnect (&signalTimer, SIGNAL (timeout (void)),
	               this, SLOT (nextChannel (void)));
           disconnect (my_dabProcessor, SIGNAL (No_Signal_Found (void)),
                       this, SLOT (nextChannel (void)));
	   connect (resetButton, SIGNAL (clicked (void)),
	            this, SLOT (reset (void)));
	   return;
	}
//
//	"normal behaviour"
	QString text = "scanning ch ";
	text. append (theBand -> channel (channelNumber. load ()));
	set_ensembleName (text);
	int tunedFrequency	=
	        theBand -> Frequency (channelNumber. load ());
	connect (&signalTimer, SIGNAL (timeout (void)),
	         this, SLOT (nextChannel (void)));
	connect (my_dabProcessor, SIGNAL (No_Signal_Found (void)),
	         this, SLOT (nextChannel (void)));
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
	while (!(channelNumber >= channels)) {
	   QString channel = theBand -> channel (channelNumber);
	   if (dabSettings -> value (channel, 1). toInt () > 0) {
	      dabSettings -> setValue (channel, -1);
	      break;
	   }
	   channelNumber ++;
	}

	if (channelNumber >= channels) {
	   scanning = false;	
	   set_ensembleName ("end of scan");
	   serviceLabel -> setText ("select a services");
	   serviceLabel -> setStyleSheet ("QLabel {background-color : green}");
	   connect (ensembleDisplay,
	            SIGNAL (clicked (QModelIndex)),
	            this, SLOT (selectService (QModelIndex)));
	   connect (resetButton, SIGNAL (clicked (void)),
	            this, SLOT (reset (void)));
	   return;
	}

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
}

void	RadioInterface::reset (void) {
	my_dabProcessor	-> stop ();
	
	disconnect (ensembleDisplay,
	         SIGNAL (clicked (QModelIndex)),
	         this, SLOT (selectService (QModelIndex)));
	disconnect (resetButton, SIGNAL (clicked (void)),
	            this, SLOT (reset (void)));
	for (int i = 0; i < channels; i ++) {
	   QString channel = theBand -> channel (i);
	   dabSettings -> setValue (channel, 1);
	}
	if (serviceDescription != NULL)
	   delete serviceDescription;
	serviceDescription	= NULL;
	Services 	= QStringList ();
	ensemble. setStringList (Services);
        ensembleDisplay         -> setModel (&ensemble);
	for (std::map<QString, serviceDescriptor *>
	                    ::iterator it = serviceMap. begin ();
	     it != serviceMap. end (); it ++)
	   delete (it -> second);
	serviceMap. clear ();
	startScanning ();
}
//
//	a slot, called by the fic/fib handlers
//	little tricky, since we do not want the data here
//	when setting the channel for selecting a service in
//	the large list
void	RadioInterface::addtoEnsemble (const QString &s) {
	if (!scanning)
//	if (!scanning && (services. size () > 0))
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
	      Services << s;

	      Services. removeDuplicates ();
	      ensemble. setStringList (Services);
	      ensembleDisplay -> setModel (&ensemble);
	      
	      dabSettings	-> setValue (channel, 1);
	      serviceMap. insert (mapElement (t, service));
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

void	RadioInterface::showSpectrum		(int s) {
	(void)s;
}

void	RadioInterface::showIQ			(int s) {
	(void)s;
}

void	RadioInterface::showQuality		(float f) {
	(void)f;
}

void	RadioInterface::show_snr		(int s) {
	snrDisplay	-> display (s);
}

void	RadioInterface::set_CorrectorDisplay	(int c) {
	(void)c;
}

void	RadioInterface::show_frameErrors	(int e) {
	emit set_quality (e);
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
void	RadioInterface::showImpulse (int a) {
	(void)a;
}


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

	inputDevice	-> stopReader ();
	my_dabProcessor	-> stop ();		// definitely concurrent
	soundOut	-> stop ();
//	everything should be halted by now
	fprintf (stderr, "going to delete dabProcessor\n");
	delete	my_dabProcessor;
	fprintf (stderr, "deleted dabProcessor\n");
	delete		soundOut;
	if (inputDevice != NULL)
	   delete	inputDevice;
	if (ensembleDisplay != NULL)
	   delete	ensembleDisplay;
	if (serviceDescription != NULL)
	   delete serviceDescription;
	if (pictureLabel != NULL)
	   delete pictureLabel;
	pictureLabel = NULL;		// signals may be pending, so careful
	if (serviceCharacteristics != NULL)
	   delete serviceCharacteristics;
	close ();
	fprintf (stderr, ".. end the radio silences\n");
}

//
//	Signals from the GUI
/////////////////////////////////////////////////////////////////////////

//	Selecting a service is easy, the fib is asked to
//	hand over the relevant data in two steps
void	RadioInterface::selectService (QModelIndex ind ) {
	if (scanning)	// be polite, wait until we finished scanning
	   return;

QString currentProgram = ensemble. data (ind, Qt::DisplayRole). toString ();

	running. store (true);
	serviceLabel	-> setStyleSheet ("QLabel {background-color : white}");
	serviceLabel	-> setText (currentProgram);

	std::map<QString, serviceDescriptor *>::iterator it;
	it	= serviceMap. find (currentProgram);
	if (it == serviceMap. end ())
	   return;
	serviceDescriptor *sd = it -> second;
	fprintf (stderr, "channel = %s, currentChannel %s\n",
	          sd -> channel. toLatin1 (). data (),
	          selectedChannel. toLatin1 (). data ());
	if (selectedChannel != sd -> channel) {
	   my_dabProcessor	-> stop ();
	   connect (&channelTimer, SIGNAL (timeout (void)),
	                    this, SLOT (channelTimer_timeout (void)));
	   channelTimer. start (5000);
	   int tunedFrequency	= theBand -> Frequency (sd -> channel);
	   my_dabProcessor	-> start (tunedFrequency, false);
	   selectedChannel = sd -> channel;
	   fprintf (stderr, "ready to start %s (%s)\n",
	                        currentProgram. toLatin1 (). data (),
	                        (sd -> channel). toLatin1 (). data ());
	   currentService	= currentProgram;
	}
	else
	   startService (currentProgram);
}

void	RadioInterface::channelTimer_timeout (void) {
//	so, we were looking for a channel. Let us see if
//	it has provided us with data
	disconnect (&channelTimer, SIGNAL (timeout (void)),
	                    this, SLOT (channelTimer_timeout (void)));
	if (my_dabProcessor -> kindofService (currentService) != 
	                                                   AUDIO_SERVICE)
	   return;
	startService (currentService);
}

void	RadioInterface::startService (QString currentService) {
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

void    RadioInterface::updateTime              (void) {
QDateTime currentTime = QDateTime::currentDateTime ();

        timeDisplay     -> setText (currentTime.
                                    toString (QString ("dd.MM.yy:hh:mm:ss")));
}


bool	RadioInterface::eventFilter (QObject *obj, QEvent *event) {
	if ((obj == ensembleDisplay -> viewport ()) &&
	    (event -> type () == QEvent::MouseButtonPress )) {
	   QMouseEvent *ev = static_cast<QMouseEvent *>(event);
           if (ev -> buttons () & Qt::RightButton) {
	      audiodata ad;
	      QString serviceName =
	           this -> ensembleDisplay -> indexAt (ev -> pos()). data ().toString ();
	      if (serviceDescription != NULL)
	         delete serviceDescription;
	      std::map<QString, serviceDescriptor *>::iterator it;
	      it	= serviceMap. find (serviceName);
	      if (it == serviceMap. end ())
	         return true;
	      serviceDescriptor *sd = it -> second;
              if (sd -> defined) {
	         serviceDescription = new audioDescriptor (sd);
	         return true;
	      }
	   }
	}
	return QMainWindow::eventFilter (obj, event);
}

deviceHandler	*RadioInterface::setDevice (QSettings	*dabSettings,
	                                    QSpinBox	*gainSelector,
	                                    QSpinBox	*lnaSelector,
	                                    QCheckBox	*agcControl) {
deviceHandler	*inputDevice	= NULL;
int	gain;
///	OK, everything quiet, now let us see what to do
	gainSelector	-> hide ();
	lnaSelector	-> hide ();
	agcControl	-> hide ();
#ifdef	HAVE_AIRSPY
	try {
	   inputDevice	= new airspyHandler (dabSettings,
	                                     gainSelector,
	                                     agcControl);
	   gainSelector	-> show ();
	   agcControl	-> show ();
	   return inputDevice;
	} catch (int e) {
	}
#endif
#ifdef	HAVE_SDRPLAY
	try {
	   inputDevice	= new sdrplayHandler (dabSettings,
	                                      gainSelector,
	                                      lnaSelector,
	                                      agcControl);
	   gainSelector	-> show ();
	   lnaSelector	-> show ();
	   agcControl -> show ();
	   return inputDevice;
	} catch (int e) {}
#endif
#ifdef	HAVE_RTLSDR
	try {
	   inputDevice	= new rtlsdrHandler (dabSettings,
	                                     gainSelector,
	                                     agcControl);
	   gainSelector	-> show ();
	   agcControl	-> show ();
	   return inputDevice;
	} catch (int e) {}
#endif
#ifdef	HAVE_HACKRF
	try {
	   inputDevice	= new hackrfHandler (dabSettings,
	                                     gainSelector,
	                                     lnaSelector);
	   gainSelector	-> show ();
	   lnaSelector	-> show ();
	   return inputDevice;
	} catch (int e) {}
#endif
	return NULL;
}
