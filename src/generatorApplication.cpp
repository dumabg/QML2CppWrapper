#include "generatorApplication.h"
#include "generator.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

namespace QML2CppWrapper {

ViewsGeneratorApplication::ViewsGeneratorApplication(int argc, char *argv[]): QGuiApplication(argc, argv)
{
    m_args = arguments();
}

void ViewsGeneratorApplication::onRun()
{
    if (m_args.size() == 3) {
        const QString projectPath = QDir::fromNativeSeparators(m_args.at(1));
        const QString qml = QDir::fromNativeSeparators(m_args.at(2));
        if (qml.endsWith(".qml")) {
            if (qml.startsWith(projectPath)) {
                QFileInfo info(qml);
                QString outPath = info.absolutePath();
                Generator vg(projectPath);
                vg.generate(qml, outPath);
            }
            else {
                exit(-2);
            }
        }
        else {
            qWarning() << qml << " it's not a qml file";
        }
    }
    else {
        exit(-1);
    }
    exit(0);
}

}
