#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the  SDR-J series.
 *
 *    SDR-J is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef	__SERVICE_LIST__
#define	__SERVICE_LIST__

#include	<QWidget>
#include	<QScrollArea>
#include	<QTableWidget>
#include	<QStringList>
#include	<QTableWidgetItem>
#include	<QObject>
#include	<QString>

class	RadioInterface;

class serviceList:public QObject {
Q_OBJECT
public:
		serviceList	(RadioInterface *, QString);
		~serviceList	(void);
	void	addRow		(const QString &, const QString &,
	                         QString, QString);
	void	show		(void);
	void	hide		(void);
	void	loadTable	(void);
	void	saveTable	(void);
	bool	isEmpty		(void);
	void	reset		(void);
private slots:
	void	tableSelect	(int, int);
	void	removeRow	(int, int);
signals:
	void	newService	(const QString &, const QString &);
private:
	QScrollArea		*myWidget;
	QTableWidget	*tableWidget;
	QString		saveName;
};

#endif

