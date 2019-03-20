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
#include	<QStringListModel>
#include	<QListView>
#include	<QComboBox>
#include	<QLabel>
#include	<QTimer>
#include	<sndfile.h>
#include	<atomic>
#include	<map>
#include	"ui_dabradio.h"
#include	"dab-processor.h"
#include	"ringbuffer.h"
#include        "band-handler.h"
#include	"text-mapper.h"
#include	"service-descriptor.h"
class	QSettings;
class	virtualInput;
class	audioBase;
class	common_fft;
class	audioDescriptor;
class	spectrumhandler;

/*
 *	GThe main gui object. It inherits from
 *	QDialog and the generated form
 */

class RadioInterface: public QMainWindow,
		      private Ui_dabradio {
Q_OBJECT
public:
		RadioInterface		(QSettings	*,
	                                 QString,
	                                 bandHandler	*,
	                                 QWidget	*parent = NULL);
		~RadioInterface		(void);

private:
	QSettings	*dabSettings;
	QString         deviceName;
	void		set_picturePath		(void);
	void		Increment_Channel	(void);
	deviceHandler	*setDevice		(QSettings *,
	                                         QSpinBox *,
	                                         QSpinBox *,
	                                         QCheckBox *);
	void		setColor		(QLabel *l, uint8_t b);
	void		setColor		(QPushButton *l, uint8_t b);

	QTimer          secondsTimer;

	typedef std::pair <QString, serviceDescriptor *> mapElement;
	std::map<QString, serviceDescriptor *> serviceMap;
	std::atomic<int>	channelNumber;
	int		serviceCount;
	QString		selectedChannel;
	QStringListModel        ensemble;
        QStringList     Services;
	QListView	*ensembleDisplay;
	uint8_t		dabBand;
	uint8_t		dabMode;
	uint8_t		isSynced;
	int16_t		threshold;
	int16_t		diff_length;
	bandHandler	*theBand;
	int		channels;
	std::atomic<bool>	running;
	bool		scanning;
	deviceHandler	*inputDevice;
	textMapper	the_textMapper;
	dabProcessor	*my_dabProcessor;
	audioBase	*soundOut;
	RingBuffer<int16_t>	*audioBuffer;
	RingBuffer<uint8_t>	*dataBuffer;
	bool		autoCorrector;
	QLabel		*pictureLabel;
	bool		saveSlides;
	bool		showSlides;
	QFrame		*serviceCharacteristics;
	QTimer		displayTimer;
	QTimer		signalTimer;
	QTimer		channelTimer;
	QString		currentService;
	bool		has_presetName;
	int32_t		numberofSeconds;
	int16_t		ficBlocks;
	int16_t		ficSuccess;

	int		autogain;
	QString		picturesPath;
	void		startService		(QString);
	void		startScanning		(void);
	void		TerminateProcess	(void);
	audioDescriptor	*serviceDescription;
protected:
        bool    eventFilter (QObject *obj, QEvent *event);

public slots:
	void		set_CorrectorDisplay	(int);
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
        void            updateTime              (void);

	void		show_motHandling	(bool);
	void		showImpulse		(int);
	void		showSpectrum		(int);
	void		showIQ			(int);
	void		show_audioQuality	(int);
	void		showQuality		(float);
	void		show_snr		(int);
//	Somehow, these must be connected to the GUI
private slots:
	void		reset			(void);
	void		channelTimer_timeout	(void);
	void		selectService		(QModelIndex);
signals:
	void		set_quality		(int);
};
#endif

