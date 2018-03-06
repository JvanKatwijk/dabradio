#
/*
 *    Copyright (C) 2014 .. 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the Qt-DAB.
 *    Qt-DAB is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    Qt-DAB is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Qt-DAB; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//
//
#ifndef		__SPECTRUM_HANDLER__
#define		__SPECTRUM_HANDLER__

#include        "dab-constants.h"
#include	<QObject>
#include	<fftw3.h>
#include	"ringbuffer.h"
#include	<qwt.h>
#include	<qwt_plot.h>
#include	<qwt_plot_marker.h>
#include	<qwt_plot_grid.h>
#include	<qwt_plot_curve.h>
#include	<qwt_plot_marker.h>


class	spectrumhandler {
public:
			spectrumhandler		(QwtPlot	*,
	                                         int32_t	,
	                                         RingBuffer<std::complex<float>> *);
			~spectrumhandler	(void);
	void		showSpectrum		(int32_t, int32_t);
	void		setBitDepth		(int16_t);
private:
	RingBuffer<std::complex<float>>	*scopeBuffer;
	int16_t		displaySize;
	int16_t		spectrumSize;
	std::complex<float> 	*spectrum;
	std::vector<double>	displayBuffer;
	std::vector<float>	Window;
	fftwf_plan	plan;
	QwtPlotMarker	*Marker;
	QwtPlot		*plotgrid;
	QwtPlotGrid	*grid;
	QwtPlotCurve	*spectrumCurve;
	QBrush		*ourBrush;
	int32_t		indexforMarker;
	void		ViewSpectrum		(double *, double *, double, int);
	float		get_db 			(float);
	int32_t		normalizer;
};

#endif

