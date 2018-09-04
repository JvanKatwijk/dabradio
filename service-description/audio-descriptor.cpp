
#include	<QFont>
#include	"dab_tables.h"
#include	"audio-descriptor.h"
#include	"service-descriptor.h"

	audioDescriptor::audioDescriptor (serviceDescriptor *ad):
	                                       myFrame (NULL) {
        setupUi (&myFrame);
        myFrame. show ();
	serviceLabel	-> setText
	              (QString ().number (ad -> serviceId, 16). toUpper ());
	QFont font	= serviceLabel -> font ();
	font. setBold (true);
	serviceLabel	-> setFont (font);
	channelLabel	-> setText (ad -> channel);
	serviceName	-> setText (ad -> name);
	subChannelId	-> setText (QString::number (ad -> subchId));
	startAddress	-> setText (QString::number (ad -> startAddr));
	Length		-> setText (QString::number (ad -> length));
	bitrate		-> setText (QString::number (ad -> bitRate));
	QString protL	= getProtectionLevel (ad -> shortForm,
	                                      ad -> protLevel);
	protectionLevel   -> setText (protL);
	dabType		-> setText (ad -> ASCTy == 077 ? "DAB+" : "DAB");
	Language	-> setText (getLanguage (ad -> language));
	programType	->
	   setText (the_textMapper.
	               get_programm_type_string (ad -> programType));
}

	audioDescriptor::~audioDescriptor (void) {
}

