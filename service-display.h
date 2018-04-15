#ifndef	__SERVICE_DISPLAY__
#define	__SERVICE_DISPLAY__

#include	"dab-constants.h"
#include	<QFrame>
#include	<QLabel>
#include	<QVBoxLayout>
#include	"text-mapper.h"

class	serviceDisplay : public QFrame {
Q_OBJECT
public:
	serviceDisplay	(audiodata *d, QWidget *parent = NULL);
	~serviceDisplay	(void);
private:
	textMapper	the_textMapper;
	QLabel		*qualityIndicator;
public slots:
	void	set_qualityIndicator (int);
};
#endif
