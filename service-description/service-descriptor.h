
#ifndef	__SERVICE_DESCRIPTOR__
#define	__SERVICE_DESCRIPTOR__
#include	<QString>
#include	<stdio.h>

#include	"dab-constants.h"

class serviceDescriptor  {
public:
	bool	defined;
	QString name;
	QString	channel;
        int32_t serviceId;
        int16_t subchId;
        int16_t startAddr;
        bool    shortForm;
        int16_t protLevel;
        int16_t length;
        int16_t bitRate;
        int16_t ASCTy;
        int16_t language;
        int16_t programType;

	serviceDescriptor (QString name, QString channel, audiodata *d);
	~serviceDescriptor (void);
};

#endif

