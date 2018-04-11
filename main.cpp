#
/*
 *    Copyright (C) 2014 .. 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of dabradio (formerly SDR-J, JSDR).
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
 *      Main program
 */
#include        <QApplication>
#include        <QSettings>
#include	<QTranslator>
#include        <QDir>
#include	<QDebug>
#include        <unistd.h>
#include        "dab-constants.h"
#include        "radio.h"

#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
#ifdef	HAVE_SDRPLAY
#include	"sdrplay-handler.h"
#endif
#ifdef	HAVE_AIRSPY
#include	"airspy-handler.h"
#endif
#define DEFAULT_INI     ".dabradio.ini"
#define	SERVICE_LIST	".dabradio-stations.bin"

#ifndef	GITHASH
#define	GITHASH	"      "
#endif

QString fullPathfor (QString v, QString aa) {
QString fileName;

	if (v == QString (""))
	   return QString ("/tmp/xxx");

	if (v. at (0) == QChar ('/'))           // full path specified
	   return v;

	fileName = QDir::homePath ();
	fileName. append ("/");
	fileName. append (v);
	fileName = QDir::toNativeSeparators (fileName);

	if (!fileName. endsWith (aa))
	   fileName. append (aa);

	return fileName;
}

void    	setTranslator (QString Language);
virtualInput	*setDevice (QSettings *dabSettings);

int     main (int argc, char **argv) {
QString initFileName	= fullPathfor (QString (DEFAULT_INI), ".ini");
QString serviceList	= fullPathfor (QString (SERVICE_LIST), ".bin");
RadioInterface  *MyRadioInterface;
virtualInput	*theDevice;
// Default values
QSettings       *dabSettings;           // ini file
int     opt;

	QCoreApplication::setOrganizationName ("Lazy Chair Computing");
	QCoreApplication::setOrganizationDomain ("Lazy Chair Computing");
	QCoreApplication::setApplicationName ("dabradio");
	QCoreApplication::setApplicationVersion (QString (CURRENT_VERSION) + " Git: " + GITHASH);

	while ((opt = getopt (argc, argv, "i:c:")) != -1) {
	   switch (opt) {
	      case 'i':
	         initFileName	= fullPathfor (QString (optarg), ".ini");
	         break;

	      default:
	         break;

	      case 'c':
	         serviceList	= fullPathfor (QString (optarg), ".bin");
	         break;
	   }
	}

	
	dabSettings =  new QSettings (initFileName, QSettings::IniFormat);
/*
 *      Before we connect control to the gui, we have to
 *      instantiate
 */
	QApplication a (argc, argv);
//	setting the language
	QString locale = QLocale::system (). name ();
	qDebug() << "main:" <<  "Detected system language" << locale;
	setTranslator (locale);

	theDevice	= setDevice (dabSettings);
	if (theDevice == NULL) {
	   fprintf (stderr, "sorry, no device found, fatal\n");
	   exit (1);
	}
//	a. setWindowIcon (QIcon (":/dab-radio.ico"));

	QString dabBand	= dabSettings -> value ("dabBand", "Band III"). toString ();
	bandHandler my_bandHandler (dabBand);
	MyRadioInterface = new RadioInterface (dabSettings,
	                                       serviceList,
	                                       &my_bandHandler,
	                                       theDevice);
	MyRadioInterface -> show ();

#if QT_VERSION >= 0x050600
	QGuiApplication::setAttribute (Qt::AA_EnableHighDpiScaling);
#endif
        a. exec ();
/*
 *      done:
 */
	dabSettings -> sync ();
	fprintf (stderr, "back in main program\n");
	fflush (stdout);
	fflush (stderr);
	qDebug ("It is done\n");
//	delete MyRadioInterface;
	delete dabSettings;
}

void	setTranslator (QString Language) {
QTranslator *Translator = new QTranslator;

//	German is special (as always)
	if ((Language == "de_AT") || (Language ==  "de_CH"))
	   Language = "de_DE";
//
//	what about Dutch?
	bool TranslatorLoaded =
	             Translator -> load (QString(":/i18n/") + Language);
	qDebug() << "main:" <<  "Set language" << Language;
	QCoreApplication::installTranslator (Translator);

	if (!TranslatorLoaded) {
	   qDebug() << "main:" <<  "Error while loading language specifics" << Language << "use English \"en_GB\" instead";
	   Language = "en_GB";
	}

	QLocale curLocale (QLocale ((const QString&)Language));
	QLocale::setDefault (curLocale);
}

virtualInput	*setDevice (QSettings *dabSettings) {
virtualInput	*inputDevice	= NULL;
int	gain;
///	OK, everything quiet, now let us see what to do
#ifdef	HAVE_AIRSPY
	try {
	   inputDevice	= new airspyHandler (dabSettings);
	   return inputDevice;
	} catch (int e) {}
#endif
#ifdef	HAVE_SDRPLAY
	try {
	   inputDevice	= new sdrplayHandler (dabSettings);
	   return inputDevice;
	} catch (int e) {}
#endif
#ifdef	HAVE_RTLSDR
	try {
	   inputDevice	= new rtlsdrHandler (dabSettings);
	   return inputDevice;
	} catch (int e) {}
#endif
	return NULL;
}
