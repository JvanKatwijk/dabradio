#
/*
 *    Copyright (C) 2013, 2014, 2015, 2016, 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the dabradio
 *
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
 *
 *	Main program
 */

#ifndef __DAB_RADIO__
#define __DAB_RADIO__

#include	"dab-constants.h"
#include	<QMainWindow>
#include	<QStringList>
#include	<QComboBox>
#include	<QLabel>
#include	<QTimer>
#include	<sndfile.h>
#include	"ui_dabradio.h"
#include	"dab-processor.h"
#include	"ringbuffer.h"
#include        "band-handler.h"
#include	"text-mapper.h"
#include	"service-list.h"
#include	<atomic>
class	QSettings;
class	virtualInput;
class	audioBase;
class	common_fft;

class	spectrumhandler;

/*
 *	GThe main gui object. It inherits from
 *	QDialog and the generated form
 */

class serviceDescriptor {
public:
QString name;
QString	channel;
int32_t	serviceId;
uint8_t	shortForm;
uint8_t	protLevel;
int32_t	bitRate;
int32_t	startAddr;
int32_t	length;
QString	language;
QString	programType;

	serviceDescriptor (QString name, QString channel, audiodata *d) {
	   textMapper	the_textMapper;
	   this -> name		= name;
	   this -> channel	= channel;
	   this	-> serviceId	= d	-> serviceId;
	   this	-> shortForm	= d	-> shortForm;
	   this	-> protLevel	= d	-> protLevel;
	   this	-> bitRate	= d	-> bitRate;
	   this	-> startAddr	= d	-> startAddr;
	   this	-> length	= d	-> length;
	   this	-> language	= the_textMapper.
                       get_programm_language_string (d -> language);
	   this	-> programType	= the_textMapper.
                       get_programm_type_string (d -> programType);
	}
	serviceDescriptor (QString name, QString channel) {
	   this -> name		= name;
	   this -> channel	= channel;
	   this	-> serviceId	= 0;
	   this	-> shortForm	= 0;
	   this	-> protLevel	= 0;
	   this	-> bitRate	= 0;
	   this	-> startAddr	= 0;
	   this	-> length	= 0;
	   this	-> language	= QString ("");
	   this	-> programType	= QString ("");
	}
	~serviceDescriptor (void) {}
};


class RadioInterface: public QMainWindow,
		      private Ui_dabradio {
Q_OBJECT
public:
		RadioInterface		(QSettings	*,
	                                 QString,
	                                 bandHandler	*,
	                                 virtualInput	*,
	                                 QWidget	*parent = NULL);
		~RadioInterface		(void);

private:
	QSettings	*dabSettings;
	QString         deviceName;
	void		set_picturePath		(void);
	void		Increment_Channel	(void);
	virtualInput	*setDevice		(QString);
	void		setColor		(QLabel *l, uint8_t b);
	void		setColor		(QPushButton *l, uint8_t b);

	std::atomic<int>	channelNumber;
	int		serviceCount;
	QString		selectedChannel;
	std::vector<serviceDescriptor *> services;
	uint8_t		dabBand;
	uint8_t		dabMode;
	uint8_t		isSynced;
	int16_t		threshold;
	int16_t		diff_length;
	bandHandler	*theBand;
	std::atomic<bool>	running;
	bool		scanning;
	virtualInput	*inputDevice;
	textMapper	the_textMapper;
	dabProcessor	*my_dabProcessor;
	audioBase	*soundOut;
	RingBuffer<int16_t>	*audioBuffer;
	RingBuffer<uint8_t>	*dataBuffer;
	bool		autoCorrector;
	QLabel		*pictureLabel;
	bool		saveSlides;
	bool		showSlides;
	QFrame		*dataDisplay;
	serviceList	*ensembleDisplay;
	QTimer		displayTimer;
	QTimer		signalTimer;
	QTimer		channelTimer;
	QString		currentName;
	bool		has_presetName;
	int32_t		numberofSeconds;
	int16_t		ficBlocks;
	int16_t		ficSuccess;

	int		autogain;
	QString		picturesPath;
	void		selectService		(QString);
	void		startScanning		(void);
	void		TerminateProcess	(void);
public slots:
	void		addtoEnsemble		(const QString &);
	void		nameofEnsemble		(int, const QString &);
	void		set_ensembleName	(const QString &);
	void		show_ficSuccess		(bool);
	void		setSynced		(char);
	void		showMOT			(QByteArray data,
                                                 int subtype,
	                                         QString pictureName);
	void		showLabel		(QString);
	void		changeinConfiguration	(void);
	void		newAudio		(int, int);
//
	void		setStereo		(bool);
	void		set_streamSelector	(int);
	void		nextChannel		(void);
	void		closeEvent		(QCloseEvent *event);

	void		show_motHandling	(bool);
	void		show_frameErrors	(int);
	void		show_rsErrors		(int);
	void		show_aacErrors		(int);
//	Somehow, these must be connected to the GUI
private slots:
	void		handle_gainSlider	(int);
	void		handle_autoButton	(void);
	void		reset			(void);
	void		updateTimeDisplay	(void);
	void		channelTimer_timeout	(void);
	void		selectService		(const QString &, const QString &);
};
#endif

