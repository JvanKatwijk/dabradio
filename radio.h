#
/*
 *    Copyright (C) 2013, 2014, 2015, 2016, 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the dab3 (formerly SDR-J, JSDR).
 *
 *    dab3 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    dab3 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with dab3; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	Main program
 */

#ifndef __RADIO_DAB3__
#define __RADIO_DAB3__

#include	"dab-constants.h"
#include	<QMainWindow>
#include	<QStringList>
#include	<QStringListModel>
#include	<QComboBox>
#include	<QLabel>
#include	<QTimer>
#include	<sndfile.h>
#include	"ui_dabradio.h"
#include	"dab-processor.h"
#include	"ringbuffer.h"
#include        "band-handler.h"
#include	"text-mapper.h"
class	QSettings;
class	virtualInput;
class	audioBase;
class	common_fft;

#include	"ui_technical_data.h"

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

	serviceDescriptor (QString name, QString channel, audiodata *d) {
	   this -> name		= name;
	   this -> channel	= channel;
	   this	-> serviceId	= d	-> serviceId;
	   this	-> shortForm	= d	-> shortForm;
	   this	-> protLevel	= d	-> protLevel;
	   this	-> bitRate	= d	-> bitRate;
	   this	-> startAddr	= d	-> startAddr;
	   this	-> length	= d	-> length;
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
	}
	~serviceDescriptor (void) {}
};


class RadioInterface: public QMainWindow,
		      private Ui_dabradio {
Q_OBJECT
public:
		RadioInterface		(QSettings	*,
	                                 bool		,
	                                 QWidget	*parent = NULL);
		~RadioInterface		(void);

private:
	QSettings	*dabSettings;
	QString         deviceName;
	Ui_technical_data	techData;
private:
	void		clear_showElements	(void);
	void		set_picturePath		(void);
	void		Increment_Channel	(void);
	void		hideButtons		(void);
	void		showButtons		(void);
	virtualInput	*setDevice		(QString);
	void		doStart			(void);
	void		dumpControlState	(void);
	bool		rawFile_flag;
	std::vector<serviceDescriptor *> services;
	uint8_t		dabBand;
	uint8_t		dabMode;
	bool		thereisSound;
	uint8_t		isSynced;
	int16_t		threshold;
	int16_t		diff_length;
	bandHandler     theBand;
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
	bool		show_data;

	QStringList	soundChannels;
	QStringListModel	ensemble;
	QStringList	Services;
	QString		ensembleLabel;
	QTimer		displayTimer;
	QTimer		signalTimer;
	QTimer		channelTimer;
	QString		currentName;
	bool		has_presetName;
	int32_t		numberofSeconds;
	int16_t		ficBlocks;
	int16_t		ficSuccess;
        spectrumhandler         *spectrumHandler;
	RingBuffer<std::complex<float>>  *spectrumBuffer;

	QString		picturesPath;
public slots:
	void		set_Scanning		(void);
	void		set_CorrectorDisplay	(int);
	void		clearEnsemble		(void);
	void		addtoEnsemble		(const QString &);
	void		nameofEnsemble		(int, const QString &);
	void		show_frameErrors	(int);
	void		show_rsErrors		(int);
	void		show_aacErrors		(int);
	void		show_ficSuccess		(bool);
	void		show_snr		(int);
	void		setSynced		(char);
	void		showLabel		(QString);
	void		showMOT			(QByteArray, int, QString);
	void		changeinConfiguration	(void);
	void		newAudio		(int, int);
//
	void		show_techData		(QString,
	                                         QString,
	                                         int32_t,
	                                         audiodata *);

	void		setStereo		(bool);
	void		set_streamSelector	(int);
	void		No_Signal_Found		(void);
	void		show_motHandling	(bool);
	void		setSyncLost		(void);
	void		showSpectrum		(int);
	void		closeEvent		(QCloseEvent *event);

//	Somehow, these must be connected to the GUI
private slots:
	void		toggle_show_data	(void);
	void		TerminateProcess	(void);
	void		selectChannel		(QString);
	void		updateTimeDisplay	(void);
	void		signalTimer_out		(void);
	void		autoCorrector_on	(void);

	void		channelTimer_timeout	(void);
	void		selectService		(QModelIndex);
	void		selectService		(QString);
};
#endif

