#include "generatorApplication.h"
#include <QTimer>
#include <QDebug>

int main(int argc, char *argv[])
{
    qputenv("QML_DISABLE_DISK_CACHE", "1");
    QML2CppWrapper::ViewsGeneratorApplication a(argc, argv);
    qInfo() <<"Starting QML2CppWrapper";
    QTimer::singleShot(0, &a, &QML2CppWrapper::ViewsGeneratorApplication::onRun);
    return a.exec();
}
