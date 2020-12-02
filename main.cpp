#include <QApplication>
#include "SignalGenerator.h"
#include "ConnectToAgora.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	qRegisterMetaType<com_ptr<IDeckLinkVideoFrame>>("com_ptr<IDeckLinkVideoFrame>");

    //if (connectAgora() > 0);  //printf("success connect to agora");
    //*********************************************

	SignalGenerator cp;
	cp.setup();

    int temp = app.exec();

    //if (disconnectAgora() > 0);  //printf("success disconect to agora");
    return temp;
}

