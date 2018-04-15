

#include	"service-display.h"
#include	<QLabel>
#include	<QVBoxLayout>
#include	"text-mapper.h"

	serviceDisplay::serviceDisplay (audiodata *d, QWidget *parent):
	                                      QFrame (parent)  {
	   QVBoxLayout	*mainLayout = new QVBoxLayout ();
	   QLabel 	*nameofService  = new QLabel (d -> serviceName);
	   QLabel	*bitrateDisplay	= new QLabel;
	   QString t1	= "bitrate ";
	   t1. append (QString::number (d -> bitRate));
	   bitrateDisplay -> setText (t1);
           QLabel	*lengthDisplay  = new QLabel;
	   t1		= "segment length ";
	   t1. append (QString::number (d -> length));
	   lengthDisplay	-> setText (t1);
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
	   QLabel	* protectionLabel = new QLabel;
	   t1		= "protection ";
	   t1. append (protL);
	   protectionLabel	-> setText (t1);

	   QLabel	*typeLabel	= new QLabel;
	   typeLabel	-> setText (d -> ASCTy == 077 ? "DAB+" : "DAB");
	   QLabel	*languageLabel	= new QLabel;
 	   languageLabel ->
           setText (the_textMapper.
                       get_programm_language_string (d -> language));
	   QLabel	*programtypeLabel = new QLabel;
	   programtypeLabel ->
                       setText (the_textMapper.
                            get_programm_type_string (d -> programType));
	   qualityIndicator	= new QLabel ("Quality ");

	   mainLayout	-> addWidget (nameofService);
	   mainLayout	-> addWidget (bitrateDisplay);
	   mainLayout	-> addWidget (lengthDisplay);
	   mainLayout	-> addWidget (protectionLabel);
	   mainLayout	-> addWidget (typeLabel);
	   mainLayout	-> addWidget (languageLabel);
	   mainLayout	-> addWidget (programtypeLabel);
	   mainLayout	-> addWidget (qualityIndicator);
	   setLayout (mainLayout);
	   this -> setVisible (true);
}

	serviceDisplay::~serviceDisplay (void) {
}

void	serviceDisplay::set_qualityIndicator (int q) {
QString t	= "Quality ";
	t. append (QString::number (q));
	qualityIndicator	-> setText (t);
}

