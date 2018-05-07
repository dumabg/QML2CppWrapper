#include "generator.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStandardPaths>
#include <QQmlComponent>
#include <QByteArray>
#include <QQmlEngine>
#include <QCoreApplication>
#include <QQuickItem>
#include <QQmlContext>

namespace QML2CppWrapper {

Generator::Generator(const QString &rootPath):
    m_rootPath(rootPath) //, m_engine(nullptr), m_comp(nullptr)
{
}

void Generator::generate(const QString &qmlFileName, const QString &outPath)
{
    QFile file(qmlFileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        findSignalsProperties(in, qmlFileName);
        in.seek(0);
        findModels(in);
        in.seek(0);
        findObjectNames(in, qmlFileName);
        in.seek(0);
        findContextProperties(in);
    }
    QFileInfo info(qmlFileName);
    QString className = info.completeBaseName();
    QString classNameGenerated = className + "Generated";
    generateGeneratedH(outPath, classNameGenerated);
    QString relativeQmlFileName = qmlFileName.mid(m_rootPath.size());
    generateGeneratedC(outPath, classNameGenerated, relativeQmlFileName);
    generateH(outPath, className);
    generateC(outPath, className);
    QString relativeQmlPath = relativeQmlFileName.mid(0, relativeQmlFileName.lastIndexOf('/'));
    for (int i = 0; i < m_models.size(); i++) {
        const Model &model = m_models.at(i);
        const QString &name = model.name;
        QString classNameViewModel = name.at(0).toUpper() + name.mid(1) + "ViewModel";
        QString classNameViewModelGenerated = classNameViewModel + "Generated";
        generateGeneratedModelH(outPath, classNameViewModelGenerated, model);
        generateGeneratedModelC(outPath, classNameViewModelGenerated, model);
        generateModelH(outPath, classNameViewModel);
        generateModelC(outPath, classNameViewModel, model);
        generateQmlPrototype(relativeQmlPath, className, model);
    }
}

void Generator::findSignalsProperties(QTextStream &in, const QString &qmlFileName)
{
    int keyFound = 0;
    QQmlEngine* engine = nullptr;
    QObject* comp = nullptr;
    while ((!in.atEnd()) && (keyFound <= 1)) {
        QString line = in.readLine();
        int i = line.indexOf("signal ");
        if (i != -1) {
            QString ssignal = line.mid(i + 7).trimmed();
            Signal signal = toSignal(ssignal);
            m_signals.append(signal);
        }
        else {
            i = line.indexOf("property ");
            if (i != -1) {
                //QStringRef sproperty = line.midRef(i + 9).trimmed();
                QVector<QStringRef> parts = line.splitRef(' ', QString::SkipEmptyParts);
                // Possibilities:
                //  property type name
                //  property type name: value
                //  property type name : value
                //  property alias name: value
                //  property alias name : value
                //  readonly property type name
                //  readonly property type name: value
                //  readonly property type name : value
                //  readonly property alias name: value
                //  readonly property alias name : value
                Parameter parameter;
                parameter.readOnly = parts.at(0) == "readonly";
                int offset = parameter.readOnly ? 1 : 0;
                QStringRef name = parts.at(2 + offset);
                if (name.endsWith(':')) {
                    name = name.left(name.size() - 1);
                }
                parameter.name = name.toString();
                const QStringRef &type = parts.at(1  + offset);
                if (type == "alias") {
                    //                    parameter.type = typeForProperty(qmlFileName, name);
                    if (engine == nullptr) {
                        engine = new QQmlEngine(QCoreApplication::instance());
                        QQmlComponent component(engine, QUrl::fromLocalFile(qmlFileName));
                        comp = component.create();
                        if (comp == nullptr) {
                            QTextStream(stdout) << component.errorString();
                        }
                    }
                    if (comp) {
                        const QMetaObject *mo = comp->metaObject();
                        bool found = false;
                        int j = 0;
                        int count = mo->propertyCount();
                        while ((!found) && (j < count)) {
                            QMetaProperty property = mo->property(j);
                            QString propertyName(property.name());
                            found = propertyName == name;
                            if (found) {
                                QString type(property.typeName());
                                parameter.type = type;
                            }
                            else {
                                j++;
                            }
                        }
                        if (!found) {
                            parameter.type = "QVariant";
                        }
                    }
                }
                else {
                    parameter.type = QMLTypeToC(type);
                }
                m_properties.append(parameter);
            }
        }
        if (line.indexOf('{') != -1) {
            keyFound++;
        };
    }
    if (comp) {
        delete comp;
    }
    if (engine) {
        delete engine;
    }
}

void Generator::findModels(QTextStream &in)
{
    bool inModel = false;
    QString modelName;
    QVector<QString> roles;
    int numBrackets = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (inModel) {
            int i = line.indexOf("model.");
            if (i != -1) {
                const QString possibleRole = line.mid(i + 6).trimmed();
                QString role = extractRole(possibleRole);
                if (!roles.contains(role)) {
                    roles.append(role);
                }
            }
            i = line.indexOf('{');
            if (i != -1) {
                numBrackets++;
            }
            i = line.indexOf('}');
            if (i != -1) {
                numBrackets--;
            }
            inModel = numBrackets > 0;
            if (!inModel) {
                Model model;
                model.name = modelName;
                model.roles = roles;
                m_models.append(model);
            }
        }
        else {
            int i = line.indexOf("odel: c_");
            if (i != -1) {
                // Could be ...Model: c_... or model: c_...
                if (line.at(i - 1).toLower() == 'm') {
                    modelName = line.mid(i + 8).trimmed();
                    inModel = true;
                    numBrackets = 1;
                    roles.clear();
                }
            }
        }
    }
}

void Generator::findObjectNames(QTextStream &in, const QString &qmlFileName)
{
    QStringList imports;
    QQmlEngine *engine = nullptr;
    QString lastKey;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.indexOf("import ") != -1) {
            imports.append(line);
        }
        if (line.indexOf('{') != -1) {
            lastKey = line;
        }
        int i = line.indexOf("objectName:");
        if (i != -1) {
            if (engine == nullptr) {
                engine = new QQmlEngine(QCoreApplication::instance());
            }
            QQmlComponent component(engine);
            QByteArray qml;
            qml.append(imports.join('\n')).append('\n').append(lastKey).append('}');
            component.setData(qml, QUrl::fromLocalFile(qmlFileName));
            QObject *comp = component.create();
            if (comp == nullptr) {
                QTextStream(stdout) << component.errorString();
            }
            else {
                ObjectName objectName;
                objectName.objectName = line.mid(i + 11).trimmed().replace('\"', "");
                objectName.isQQuickItem = dynamic_cast<QQuickItem *>(comp);
                m_objectNames.append(objectName);
                delete comp;
            }
        }
    }
    if (engine != nullptr) {
        delete engine;
    }
}

void Generator::findContextProperties(QTextStream &in)
{
    while (!in.atEnd()) {
        QString line = in.readLine();
        int i = line.indexOf(": c_");
        if (i != -1) {
            // Could be ...Model: c_... or model: c_...
            QString property = line.at(i - 5).toLower() + line.mid(i - 4, 4);
            if (property != "model") {
                QString contextProperty = line.mid(i + 4).trimmed();
                if (!m_contextProperties.contains(contextProperty)) {
                    m_contextProperties.append(contextProperty);
                }
            }
        }
    }
}

void Generator::generateGeneratedH(const QString &outPath, const QString& className)
{
    QString fileName = outPath + "/" + className.at(0).toLower() + className.mid(1) + ".h";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream(stdout) << fileName.mid(m_rootPath.length()) << '\n';
        QTextStream out(&file);
        QString ifdefName = className.toUpper() + "_H";
        out << "#ifndef " << ifdefName << "\n";
        out << "#define " << ifdefName << "\n";
        out << "#include <base/qmlComponentWrapper.h>\n";
        QStringList includes;
        for (int i = 0; i < m_signals.size(); i++) {
            const Signal &signal = m_signals.at(i);
            const QVector<Parameter> &parameters = signal.parameters;
            for (int j = 0; j < parameters.size(); j++) {
                const Parameter &parameter = parameters.at(j);
                const QString &type = parameter.type;
                if ((type.startsWith("Q")) && (!includes.contains(type))) {
                    includes.append(type);
                }
            }
        }
        for (int i = 0; i < m_properties.size(); i++) {
            const Parameter &property = m_properties.at(i);
            const QString &type = property.type;
            if ((type.startsWith("Q")) && (!includes.contains(type))) {
                includes.append(type);
            }
        }
        for (int i = 0; i < includes.size(); i++) {
            out << "#include <" << includes.at(i) << ">\n";
        }
        for (int i = 0; i < m_models.size(); i++) {
            const QString &className = m_models.at(i).name;
            out << "#include \"" << className.at(0).toLower() << className.mid(1) << "ViewModel.h\"\n";
        }
        if (m_objectNames.size() > 0) {
            out << "#include <QQuickItem>\n";
        }
        if (m_contextProperties.size() > 0) {
            out << "#include <QVariant>\n";
        }
        out << '\n';
        out << "class " << className << " : public QmlComponentWrapper\n";
        out << "{\n";
        out << "    Q_OBJECT\n";
        out << "public:\n";
        out << "    explicit " << className << "(QObject *parent = 0);\n";
        out << "    virtual ~" << className << "() {}\n";
        out << "protected:\n";
        out << "    virtual QObject *createComponent() override;\n";
        for (int i = 0; i < m_contextProperties.size(); i++) {
            const QString &contextProperty = m_contextProperties.at(0);
            out << "    virtual QVariant initialValueFor" << contextProperty.at(0).toUpper() << contextProperty.mid(1) << "() = 0;\n";
        }
        for (int i = 0; i < m_properties.size(); i++) {
            Parameter property = m_properties.at(i);
            out << "    " << property.type << " " << property.name << "() const;\n";
            if (!property.readOnly) {
                out << "    void set" << property.name.at(0).toUpper() << property.name.mid(1) << "(const " << property.type << " &value);\n";
            }
        }
        for (int i = 0; i < m_objectNames.size(); i++) {
            const ObjectName on = m_objectNames.at(0);
            const QString &objectName = on.objectName;
            if (on.isQQuickItem) {
                out << "    void embedIn" << objectName.at(0).toUpper() << objectName.mid(1) << "(QQuickItem *item);\n";
                out << "    QQuickItem *" << objectName << "() const;\n";
            }
            else {
                out << "    QObject *" << objectName << "() const;\n";
            }
        }
        for (int i = 0; i < m_contextProperties.size(); i++) {
            const QString &contextProperty = m_contextProperties.at(0);
            out << "    void set" << contextProperty.at(0).toUpper() << contextProperty.mid(1) << "(const QVariant &value);\n";
            out << "    QVariant " << contextProperty << "() const;\n";
        }
        for (int i = 0; i < m_models.size(); i++) {
            const Model &model = m_models.at(i);
            const QString name = model.name;
            out << "    " << name.at(0).toUpper() << name.mid(1) << "ViewModel m_" << name << "ViewModel;\n";
        }
        if (m_signals.size() > 0) {
            out << "protected slots:\n";
            for (int i = 0; i < m_signals.size(); i++) {
                const Signal &signal = m_signals.at(i);
                out << "    virtual void " << signal.name + "(";
                const QVector<Parameter> &parameters = signal.parameters;
                for (int j = 0; j < parameters.size(); j++) {
                    if (j != 0) {
                        out << ", ";
                    }
                    const Parameter &parameter = parameters.at(j);
                    out << parameter.type << ' ' << parameter.name;
                }
                out << ") = 0;\n";
            }
        }
        out << "};\n";
        out << "\n";
        out << "#endif // " << ifdefName;
    }
}

void Generator::generateGeneratedC(const QString &outPath, const QString &className, const QString &relativeQmlFileName)
{
    QString classNameFirstLower = className.at(0).toLower() + className.mid(1);
    QString fileName = outPath + "/" + classNameFirstLower + ".cpp";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream(stdout) << fileName.mid(m_rootPath.length()) << '\n';
        QTextStream out(&file);
        out << "#include \"" << classNameFirstLower << ".h\"\n";
        out << "\n";
        out << className << "::" << className << "(QObject *parent): QmlComponentWrapper(parent)\n";
        out << "{\n";
        out << "}\n";
        out << "\n";
        out << "QObject *" << className << "::createComponent()\n";
        out << "{\n";
        if ((m_models.size() > 0) || (m_contextProperties.size() > 0)) {
            out << "    QQmlContext *context = qmlContext();\n";
        }
        for (int i = 0; i < m_models.size(); i++) {
            const Model &model = m_models.at(i);
            const QString &name = model.name;
            out << "    context->setContextProperty(\"c_" << name << "\", &m_" << name << "ViewModel);\n";
        }
        for (int i = 0; i < m_contextProperties.size(); i++) {
            const QString &contextProperty = m_contextProperties.at(0);
            out << "    context->setContextProperty(\"c_" << contextProperty << "\", initialValueFor" << contextProperty.at(0).toUpper() << contextProperty.mid(1) << "());\n";
        }
        out << "    QObject *item = createComponentFromQrc(\"" << relativeQmlFileName << "\");\n";
        for (int i = 0; i < m_signals.size(); i++) {
            const Signal &signal = m_signals.at(i);
            out << "    connect(item, SIGNAL(";
            QString signature;
            signature.append(signal.name).append('(');
            const QVector<Parameter> &parameters = signal.parameters;
            for (int j = 0; j < parameters.size(); j++) {
                if (j != 0) {
                    signature.append(", ");
                }
                const Parameter &parameter = parameters.at(j);
                signature.append(parameter.type);
            }
            signature.append(')');
            out << signature << "), this, SLOT(" << signature << "));\n";
        }
        out << "    return item;\n";
        out << "}\n";
        out << "\n";
        for (int i = 0; i < m_properties.size(); i++) {
            const Parameter &property = m_properties.at(i);
            const QString &type = property.type;
            out << type << ' ' << className << "::" << property.name << "() const\n";
            out << "{\n";
            QString variantMethod = type.startsWith('Q') ?
                        type.mid(1) :
                        type.at(0).toUpper() + type.mid(1);
            out << "    return component()->property(\"" << property.name << "\").to" << variantMethod << "();\n";
            out << "}\n";
            out << '\n';
            if (!property.readOnly) {
                out << "void " << className << "::set" << property.name.at(0).toUpper() << property.name.mid(1) << "(const " << property.type << " &value)\n";
                out << "{\n";
                out << "    component()->setProperty(\"" << property.name << "\", value);\n";
                out << "}\n";
                out << '\n';
            }
        }
        for (int i = 0; i < m_objectNames.size(); i++) {
            const ObjectName &on = m_objectNames.at(0);
            const QString &objectName = on.objectName;
            if (on.isQQuickItem) {
                out << "void " << className << "::" << "embedIn" << objectName.at(0).toUpper() << objectName.mid(1) << "(QQuickItem *item)\n";
                out << "{\n";
                out << "    QQuickItem *parent = component()->findChild<QQuickItem *>(\"" << objectName << "\");\n";
                out << "    if (parent) {\n";
                out << "        item->setParentItem(parent);\n";
                out << "    }\n";
                out << "}\n";
                out << '\n';
                out << "QQuickItem *" << className << "::" << objectName << "() const\n";
                out << "{\n";
                out << "    return component()->findChild<QQuickItem *>(\"" << objectName << "\");\n";
                out << "}\n";
                out << '\n';
            }
            else {
                out << "QObject *" << className << "::" << objectName << "() const\n";
                out << "{\n";
                out << "    return component()->findChild<QObject *>(\"" << objectName << "\");\n";
                out << "}\n";
                out << '\n';
            }
        }
        for (int i = 0; i < m_contextProperties.size(); i++) {
            const QString &contextProperty = m_contextProperties.at(0);
            out << "void " << className << "::set" << contextProperty.at(0).toUpper() << contextProperty.mid(1) << "(const QVariant &value)\n";
            out << "{\n";
            out << "    qmlContext()->setContextProperty(\"c_" << contextProperty << "\", value);\n";
            out << "}\n";
            out << '\n';
            out << "QVariant " << className << "::" << contextProperty << "() const\n";
            out << "{\n";
            out << "    return qmlContext()->contextProperty(\"c_" << contextProperty << "\");\n";
            out << "}\n";
            out << '\n';
        }
    }
}

void Generator::generateH(const QString &outPath, const QString &className)
{
    QString classNameFirstLower = className.at(0).toLower() + className.mid(1);
    QString fileName = outPath + "/" + classNameFirstLower + ".h";
    QFile file(fileName);
    if ((!file.exists()) && (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))) {
        QTextStream(stdout) << fileName.mid(m_rootPath.length()) << '\n';
        QTextStream out(&file);
        QString ifdefName = className.toUpper() + "_H";
        out << "#ifndef " << ifdefName << "\n";
        out << "#define " << ifdefName << "\n";
        out << "#include \"" << classNameFirstLower << "Generated.h\"\n";
        out << "\n";
        out << "class " << className << " : public " << className << "Generated\n";
        out << "{\n";
        out << "    Q_OBJECT\n";
        out << "public:\n";
        out << "    explicit " << className << "(QObject *parent = 0);\n";
        out << "    virtual ~" << className << "() {}\n";
        out << "protected slots:\n";
        for (int i = 0; i < m_signals.size(); i++) {
            const Signal &signal = m_signals.at(i);
            out << "    void " << signal.name + "(";
            const QVector<Parameter> &parameters = signal.parameters;
            for (int j = 0; j < parameters.size(); j++) {
                if (j != 0) {
                    out << ", ";
                }
                const Parameter &parameter = parameters.at(j);
                out << parameter.type << ' ' << parameter.name;
            }
            out << ") override;\n";
        }
        for (int i = 0; i < m_contextProperties.size(); i++) {
            const QString &contextProperty = m_contextProperties.at(0);
            out << "    QVariant initialValueFor" << contextProperty.at(0).toUpper() << contextProperty.mid(1) << "() override;\n";
        }
        out << "};\n";
        out << "\n";
        out << "#endif // " << ifdefName;
    }
}

void Generator::generateC(const QString &outPath, const QString &className)
{
    QString classNameFirstLower = className.at(0).toLower() + className.mid(1);
    QString fileName = outPath + "/" + classNameFirstLower + ".cpp";
    QFile file(fileName);
    if ((!file.exists()) && (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))) {
        QTextStream(stdout) << fileName.mid(m_rootPath.length()) << '\n';
        QTextStream out(&file);
        out << "#include \"" << classNameFirstLower << ".h\"\n";
        out << "\n";
        out << className << "::" << className << "(QObject *parent): " << className << "Generated(parent)\n";
        out << "{\n";
        out << "}\n";
        out << "\n";
        for (int i = 0; i < m_signals.size(); i++) {
            const Signal &signal = m_signals.at(i);
            out << "void " << className << "::";
            QString signature;
            signature.append(signal.name).append('(');
            const QVector<Parameter> &parameters = signal.parameters;
            for (int j = 0; j < parameters.size(); j++) {
                if (j != 0) {
                    signature.append(", ");
                }
                const Parameter &parameter = parameters.at(j);
                signature.append(parameter.type).append(' ').append(parameter.name);
            }
            out << signature << ")\n";
            out << "{\n";
            out << "}\n";
            out << '\n';
        }
        for (int i = 0; i < m_contextProperties.size(); i++) {
            const QString &contextProperty = m_contextProperties.at(0);
            out << "QVariant " << className << "::" << "initialValueFor" << contextProperty.at(0).toUpper() << contextProperty.mid(1) << "()\n";
            out << "{\n";
            out << "    return value;\n";
            out << "}\n";
            out << '\n';
        }
    }
}

void Generator::generateModelH(const QString &outPath, const QString &className)
{
    QString classNameFirstLower = className.at(0).toLower() + className.mid(1);
    QString fileName = outPath + "/" + classNameFirstLower + ".h";
    QFile file(fileName);
    if ((!file.exists()) && (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))) {
        QTextStream(stdout) << fileName.mid(m_rootPath.length()) << '\n';
        QTextStream out(&file);
        QString ifdefName = className.toUpper() + "_H";
        out << "#ifndef " << ifdefName << "\n";
        out << "#define " << ifdefName << "\n";
        out << "#include \"" << classNameFirstLower << "Generated.h\"\n";
        out << "\n";
        out << "class " << className << " : public " << className << "Generated\n";
        out << "{\n";
        out << "public:\n";
        out << "    explicit " << className << "(QObject *parent = 0);\n";
        out << "    virtual ~" << className << "() {}\n";
        out << "    int rowCount(const QModelIndex &parent) const override;\n";
        out << "    QVariant data(const QModelIndex &index, int role) const override;\n";
        out << "};\n";
        out << "\n";
        out << "#endif // " << ifdefName;
    }
}

void Generator::generateModelC(const QString &outPath, const QString &className, const Model &model)
{
    QString classNameFirstLower = className.at(0).toLower() + className.mid(1);
    QString fileName = outPath + "/" + classNameFirstLower + ".cpp";
    QFile file(fileName);
    if ((!file.exists()) && (file.open(QIODevice::WriteOnly | QIODevice::Truncate))) {
        QTextStream(stdout) << fileName.mid(m_rootPath.length()) << '\n';
        QTextStream out(&file);
        out << "#include \"" << classNameFirstLower << ".h\"\n";
        out << "\n";
        out << className << "::" << className << "(QObject *parent): " << className << "Generated(parent)\n";
        out << "{\n";
        out << "}\n";
        out << '\n';
        out << "int " << className << "::rowCount(const QModelIndex &parent) const\n";
        out << "{\n";
        out << "    Q_UNUSED(parent);\n";
        out << "    return size;\n";
        out << "}\n";
        out << '\n';
        out << "QVariant " << className << "::data(const QModelIndex &index, int role) const\n";
        out << "{\n";
        out << "    int row = index.row();\n";
        out << "    if ((row < 0) || (row >= size)) {\n";
        out << "        return QVariant();\n";
        out << "    }\n";
        out << "    switch (role) {\n";
        const QString &name = model.name;
        const QString enumName = name.at(0).toUpper() + name.mid(1) + "Roles";
        const QVector<QString> &roles = model.roles;
        for (int i = 0; i < roles.size(); i++) {
            const QString &role = roles.at(i);
            out << "    case " <<  enumName << "::" << role << "Role: {\n";
            out << "        return 0;\n";
            out << "    }\n";
        }
        out << "    }\n";
        out << "    return QVariant();\n";
        out << "}\n";
    }
}

void Generator::generateQmlPrototype(const QString &relativeQmlPath, const QString &className, const Model &model)
{
    QString filePath = m_rootPath + "/../qmlPrototype" + relativeQmlPath;
    QDir dir;
    dir.mkpath(filePath);
    QString fileName = filePath + "/" + className + " c_" + model.name + ".qml";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream(stdout) << fileName.mid(m_rootPath.length()) << '\n';
        QTextStream out(&file);
        out << "import QtQuick 2.3\n";
        out << '\n';
        out << "ListModel {\n";
        const QVector<QString> &roles = model.roles;
        bool containsSrc = false;
        for (int j = 0; j < roles.size(); j++) {
            if (roles.at(j).contains("Src")) {
                containsSrc = true;
            }
        }
        QStringList images;
        QString imagesPath;
        if (containsSrc) {
            imagesPath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0);
            QStringList filters;
            filters << "*.png";
            filters << "*.jpg";
            QDir imagesDir(imagesPath);
            images = imagesDir.entryList(filters, QDir::Files);
            if (images.size() == 0) {
                images << "dummy.jpg";
            }
        }
        int iImages = 0;
        for (int i = 0; i < 5; i ++) {
            out << "    ListElement {\n";
            for (int j = 0; j < roles.size(); j++) {
                const QString &role = roles.at(j);
                out << "        " << role << ": \"";
                if (role.contains("Src")) {
                    out << "file:///" << imagesPath << '/' << images.at(iImages) << "\"\n";
                    iImages++;
                    iImages %= images.size();
                }
                else {
                    out << role << ' ' << i + 1 << "\"\n";
                }
            }
            out << "    }\n";
        }
        out << "}\n";
    }
}

//QString Generator::typeForProperty(const QString &qmlFileName, const QStringRef &name)
//{
//    if (m_engine == nullptr) {
//        m_engine = new QQmlEngine(QCoreApplication::instance());
//        QQmlComponent component(m_engine, QUrl::fromLocalFile(qmlFileName));
//        m_comp = component.create();
//        m_comp->setParent(m_engine);
//        if (m_comp == nullptr) {
//            QTextStream(stdout) << component.errorString();
//        }
//    }
//    if (m_comp) {
//        const QMetaObject *mo = m_comp->metaObject();
//        bool found = false;
//        int j = 0;
//        int count = mo->propertyCount();
//        while ((!found) && (j < count)) {
//            QMetaProperty property = mo->property(j);
//            QString propertyName(property.name());
//            found = propertyName == name;
//            if (found) {
//                return property.typeName();
//            }
//            else {
//                j++;
//            }
//        }
//    }
//    return "QVariant";
//}

QString Generator::extractRole(const QString &possibleRole)
{
    for (int i = 0; i < possibleRole.size(); i++) {
        if (!possibleRole.at(i).isLetterOrNumber()) {
            return possibleRole.mid(0, i);
        }
    }
    return possibleRole;
}

void Generator::generateGeneratedModelH(const QString &outPath, const QString &className, const Model &model)
{
    QString classNameFirstLower = className.at(0).toLower() + className.mid(1);
    QString fileName = outPath + "/" + classNameFirstLower + ".h";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream(stdout) << fileName.mid(m_rootPath.length()) << '\n';
        QTextStream out(&file);
        QString ifdefName = className.toUpper() + "_H";
        out << "#ifndef " << ifdefName << "\n";
        out << "#define " << ifdefName << "\n";
        out << "#include <QAbstractListModel>\n";
        out << "\n";
        out << "class " << className << " : public QAbstractListModel\n";
        out << "{\n";
        out << "public:\n";
        out << "    explicit " << className << "(QObject *parent = 0);\n";
        out << "    virtual ~" << className << "() {}\n";
        out << "    QHash<int, QByteArray> roleNames() const override;\n";
        out << "protected:\n";
        const QString &name = model.name;
        out << "    enum " << name.at(0).toUpper() << name.mid(1) << "Roles {\n";
        const QVector<QString> &roles = model.roles;
        const int numRoles = roles.size();
        for (int i = 0; i < numRoles; i++) {
            const QString &role = roles.at(i);
            out << "        " << role << "Role";
            if (i == 0) {
                out << " = Qt::UserRole + 1";
            }
            if (i < numRoles - 1) {
                out << ',';
            }
            out << '\n';
        }
        out << "    };\n";
        out << "};\n";
        out << '\n';
        out << "#endif // " << ifdefName;
    }
}

void Generator::generateGeneratedModelC(const QString &outPath, const QString &className, const Model &model)
{
    QString classNameFirstLower = className.at(0).toLower() + className.mid(1);
    QString fileName = outPath + "/" + classNameFirstLower + ".cpp";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream(stdout) << fileName.mid(m_rootPath.length()) << '\n';
        QTextStream out(&file);
        out << "#include \"" << classNameFirstLower << ".h\"\n";
        out << "\n";
        out << className << "::" << className << "(QObject *parent): QAbstractListModel(parent)\n";
        out << "{\n";
        out << "}\n";
        out << '\n';
        out << "QHash<int, QByteArray>" << className << "::roleNames() const\n";
        out << "{\n";
        out << "    QHash<int, QByteArray> roles;\n";
        const QString &name = model.name;
        const QString enumName = name.at(0).toUpper() + name.mid(1) + "Roles";
        const QVector<QString> &roles = model.roles;
        for (int i = 0; i < roles.size(); i++) {
            const QString &role = roles.at(i);
            out << "    roles[" << enumName << "::" << role << "Role] = \"" << role << "\";\n";
        }
        out << "    return roles;\n";
        out << "}\n";
        out << '\n';
    }
}

QString Generator::QMLTypeToC(const QStringRef &type)
{
    // http://doc.qt.io/qt-5/qtqml-cppintegration-data.html
    if (type == "int") {
        return "int";
    }
    if (type == "string") {
        return "QString";
    }
    if (type == "bool") {
        return "bool";
    }
    if (type == "real") {
        return "qreal";
    }
    if (type == "double") {
        return "double";
    }
    if (type == "var") {
        return "QVariant";
    }
    if (type == "date") {
        return "QDateTime";
    }
    if (type == "url") {
        return "QUrl";
    }
    if (type == "color") {
        return "QColor";
    }
    return type.toString();
}

Signal Generator::toSignal(const QString &signal)
{
    Signal result;
    int i = signal.indexOf('(');
    if (i == -1) {
        result.name = signal;
    }
    else {
        int j = signal.indexOf(')', i + 1);
        result.name = signal.mid(0, i);
        QStringRef allParameters = signal.midRef(i + 1, j - i - 1);
        QVector<QStringRef> parameters = allParameters.split(',');
        result.parameters.reserve(parameters.size());
        for (int k = 0; k < parameters.size(); k++) {
            QVector<QStringRef> sParameter = parameters.at(k).split(" ", QString::SkipEmptyParts);
            if (!sParameter.isEmpty()) {
                Parameter parameter;
                parameter.type = QMLTypeToC(sParameter.at(0));
                parameter.name = sParameter.at(1).toString();
                result.parameters.append(parameter);
            }
        }
    }
    return result;
}

}
