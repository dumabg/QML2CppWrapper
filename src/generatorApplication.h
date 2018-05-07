#ifndef GENERATORAPPLICATION_H
#define GENERATORAPPLICATION_H

#include <QGuiApplication>

namespace QML2CppWrapper {

class ViewsGeneratorApplication : public QGuiApplication
{
    Q_OBJECT
public:
    explicit ViewsGeneratorApplication(int argc, char *argv[]);
public slots:
    void onRun();
private:
    QStringList m_args;
};

}
#endif // GENERATORAPPLICATION_H
