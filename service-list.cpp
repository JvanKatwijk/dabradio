#
/*
 *    Copyright (C) 2018
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
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
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	<QFile>
#include	<QDataStream>
#include	"service-list.h"
#include	"radio.h"

	serviceList::serviceList (RadioInterface *mr,
	                          QString saveName) {
	this	-> saveName	= saveName;
	myWidget	= new QScrollArea (NULL);
	myWidget	-> resize (240, 200);
	myWidget	-> setWidgetResizable(true);

	tableWidget 	= new QTableWidget (0, 4);
	myWidget	-> setWidget(tableWidget);
	tableWidget 	-> setHorizontalHeaderLabels (
	            QStringList () << tr ("station") << tr ("channel") << tr ("bitrate") << tr ("program type"));
	connect (tableWidget, SIGNAL (cellClicked (int, int)),
	         this, SLOT (tableSelect (int, int)));
	connect (tableWidget, SIGNAL (cellDoubleClicked (int, int)),
	         this, SLOT (removeRow (int, int)));
//	loadTable ();
}

	serviceList::~serviceList (void) {
int16_t	i;
int16_t	rows	= tableWidget -> rowCount ();

//	saveTable ();
	for (i = rows; i > 0; i --)
	   tableWidget -> removeRow (i);
	delete	tableWidget;
	delete	myWidget;
}

void	serviceList::reset	(void) {
int16_t	i;
int16_t	rows	= tableWidget -> rowCount ();

	saveTable ();
	for (i = rows; i > 0; i --)
	   tableWidget -> removeRow (i - 1);
}

bool	serviceList::isEmpty	(void) {
	return tableWidget -> rowCount () < 2;
}

void	serviceList::show	(void) {
	myWidget	-> show ();
}

void	serviceList::hide	(void) {
	myWidget	-> hide ();
}

void	serviceList::addRow (const QString &name,
	                     const QString &channel,
	                     QString bitRate,
	                     QString programType) {
int16_t	row	= tableWidget -> rowCount ();

	tableWidget	-> insertRow (row);
	QTableWidgetItem *item0	= new QTableWidgetItem;
	item0		-> setTextAlignment (Qt::AlignRight |Qt::AlignVCenter);
	tableWidget	-> setItem (row, 0, item0);

	QTableWidgetItem *item1 = new QTableWidgetItem;
	item1		-> setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	tableWidget	-> setItem (row, 1, item1);

	QTableWidgetItem *item2 = new QTableWidgetItem;
	item2		-> setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	tableWidget	-> setItem (row, 2, item2);

	QTableWidgetItem *item3 = new QTableWidgetItem;
	item3		-> setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	tableWidget	-> setItem (row, 3, item3);

	tableWidget	-> setCurrentItem (item0);
	tableWidget	-> item (row, 0) -> setText (name);
	tableWidget	-> item (row, 1) -> setText (channel);
	tableWidget	-> item (row, 2) -> setText (bitRate);
	tableWidget	-> item (row, 3) -> setText (programType);
}
//	Locally we dispatch the "click" and "translate"
//	it into a frequency and a call to the main gui to change
//	the frequency

void	serviceList::tableSelect (int row, int column) {
QTableWidgetItem* serviceItem = tableWidget  -> item (row, 0);
QTableWidgetItem* channelItem = tableWidget  -> item (row, 1);

	emit newService (serviceItem -> text (), channelItem -> text ());
}

void	serviceList::removeRow (int row, int column) {
	tableWidget	-> removeRow (row);
	(void)column;
}

void	serviceList::saveTable (void) {
QFile	file (saveName);

	if (file. open (QIODevice::WriteOnly)) {
	   QDataStream stream (&file);
	   int32_t	n = tableWidget -> rowCount ();
	   int32_t	m = tableWidget -> columnCount ();
	   stream << n << m;

	   for (int i = 0; i < n; i ++) 
	      for (int j = 0; j < m; j ++) 
	         tableWidget -> item (i, j) -> write (stream);
	   file. close ();
	}
}

void	serviceList::loadTable (void) {
QFile	file (saveName);

	if (file. open (QIODevice::ReadOnly)) {
	   QDataStream stream (&file);
	   int32_t	n, m;
	   stream >> n >> m;
	   tableWidget	-> setRowCount (n);
	   tableWidget	-> setColumnCount	(m);

	   for (int i = 0; i < n; i ++) {
	      for (int j = 0; j < m; j ++) {
	         QTableWidgetItem *item = new QTableWidgetItem;
	         item -> read (stream);
	         tableWidget -> setItem (i, j, item);
	      }
	   }
	   file. close ();
	}
}

