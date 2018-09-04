
#ifndef	__AUDIO_DESCRIPTOR__
#define	__AUDIO_DESCRIPTOR__
#include        <QObject>
#include        <QFrame>
#include        <QSettings>
#include        <atomic>
#include        "dab-constants.h"
#include        "ui_audio-description.h"
#include	"text-mapper.h"

class	serviceDescriptor;

class	audioDescriptor : public Ui_audioDescription {
public:
		audioDescriptor		(serviceDescriptor *ad);
		~audioDescriptor	(void);
private:
	QFrame	myFrame;
	textMapper      the_textMapper;
};

#endif

	
