
#include	"service-descriptor.h"
#include	"dab-constants.h"

	serviceDescriptor::serviceDescriptor (QString name,
	                                      QString channel, audiodata *d) {
	   this -> defined = d -> defined;
	   if (d -> defined) {
	      this -> name	= name;
	      this -> channel	= channel;
	      this -> serviceId	= d	-> serviceId;
	      this -> subchId	= d	-> subchId;
	      this -> startAddr	= d	-> startAddr;
	      this -> shortForm	= d	-> shortForm;
	      this -> protLevel	= d	-> protLevel;
	      this -> length	= d	-> length;
	      this -> bitRate	= d	-> bitRate;
	      this -> ASCTy	= d	-> ASCTy;
	      this -> language	= d	-> language;
	      this -> programType	= d	-> programType;
	   }
	}
	serviceDescriptor::~serviceDescriptor (void) {}

