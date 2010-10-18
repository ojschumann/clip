#include <QtGui/QApplication>
#include <ui/clip.h>

#include <tools/optimalrotation.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Clip w;
    w.show();
    return a.exec();
    return 0;
}


