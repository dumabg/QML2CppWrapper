#ifndef GENERATOR_H
#define GENERATOR_H
#include <QVector>
#include <QString>
#include <QStringList>
#include <QTextStream>

namespace QML2CppWrapper {

struct Parameter {
    QString type;
    QString name;
    bool readOnly;
};

struct Signal {
    QString name;
    QVector<Parameter> parameters;
};

struct Model {
    QString name;
    QVector<QString> roles;
};

struct ObjectName {
    QString objectName;
    bool isQQuickItem;
};

class Generator
{
public:
    Generator(const QString &rootPath);
    void generate(const QString& qmlFileName, const QString& outPath);
private:
    void findSignalsProperties(QTextStream &in, const QString& qmlFileName);
    void findModels(QTextStream& in);
    void findObjectNames(QTextStream& in, const QString &qmlFileName);
    void findContextProperties(QTextStream& in);
    void generateGeneratedH(const QString& outPath, const QString &className);
    void generateGeneratedC(const QString& outPath, const QString &className, const QString &relativeQmlFileName);
    void generateGeneratedModelH(const QString& outPath, const QString &className, const Model &model);
    void generateGeneratedModelC(const QString& outPath, const QString &className, const Model& model);
    void generateH(const QString& outPath, const QString &className);
    void generateC(const QString& outPath, const QString &className);
    void generateModelH(const QString& outPath, const QString &className);
    void generateModelC(const QString& outPath, const QString &className, const Model& model);
    void generateQmlPrototype(const QString &relativeQmlPath, const QString &className, const Model& model);
//    QString typeForProperty(const QString &qmlFileName, const QStringRef &name);
    QString extractRole(const QString &possibleRole);
    QString QMLTypeToC(const QStringRef &type);
    Signal toSignal(const QString &signal);
    QString m_rootPath;
    QVector<Signal> m_signals;
    QVector<Parameter> m_properties;
    QVector<Model> m_models;
    QVector<ObjectName> m_objectNames;
    QStringList m_contextProperties;
//    QQmlEngine *m_engine;
//    QObject *m_comp;
};

}
#endif // Generator_H
