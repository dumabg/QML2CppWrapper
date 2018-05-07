# StudentsView.qml sample

```
import QtQuick 2.9
import QtQuick.Controls 2.3
Page {
    signal importStudents
    signal deleteCheckeds
    signal exportCsv
    signal addUser
    signal itemChecked(int index, bool checked)
    id: r
    anchors.fill: parent
    focus: true
    Keys.forwardTo: [students]
    Action {
        id: aExportCsv
        enabled: students.count !== 0
        icon {
            source: "csv.png"
        }
        onTriggered: r.exportCsv()
    }
    Action {
        id: aAdd
        icon {
            source: "add_user.png"
        }
        onTriggered: r.addUser()
    }
    Action {
        id: aImportStudents
        objectName: "actionImportStudents"
        icon {
            source: "import.png"
        }
        onTriggered: {
            enabled = false
            r.importStudents()
        }
    }
    header: ToolBar {
        Row {
            height: parent.height
            TableButtons {
                table: students
            }
            ToolButton {
                action: aExportCsv
            }
            ToolButton {
                action: aAdd
            }
            AsyncToolButton {
                action: aImportStudents
            }
        }
        Row {
            height: parent.height
            anchors.right: parent.right
            anchors.rightMargin: 10
            Text2 {
                text: qsTr("<b>%1</b> alumnos").arg(students.count)
                verticalAlignment: Text.AlignVCenter
                height: parent.height
            }
        }
    }
    Table {
        id: students
        anchors.fill: parent
        anchors.topMargin: 10
        anchors.leftMargin: 10
        rowHeight: 50
        model: c_students
        columns: [
            TableColumn {
                width: 300
                header: TableColumnHeader {
                    text: qsTr("Alumno")
                }
                content: TableColumnContent {
                    anchors.leftMargin: 60
                    text: model.name
                    Image {
                        id: img
                        x: -60
                        y: 2
                        width: 45
                        height: 45
                        source: model.photoSrc
                        }
                    }
                }
            },
            TableColumn {
                width: 150
                header: TableColumnHeader {
                    text: qsTr("Matrícula")
                }
                content: TableColumnContent {
                    text: model.studentId
                }
            },
            TableColumn {
                width: 150
                header: TableColumnHeader {
                    text: qsTr("Correo")
                }
                content: TableColumnContent {
                    text: model.email
                }
            }
        ]
        Text2 {
            visible: students.count === 0
            font.pointSize: 14
            text: qsTr("Sin alumnos")
            anchors.centerIn: parent
        }
    }
    Component.onCompleted: students.itemChecked.connect(itemChecked)
}
```

## What is generated and why?

For this qml eight files are generated: **studentsView.h**, **studentsView.cpp**, **studentsViewGenerated.h**, **studentsViewGenerated.cpp**, **studentsViewModel.h**, **studentsViewModel.cpp**, **studentsViewModelGenerated.h** and **studentsViewModelGenerated.cpp**.

### studentsViewGenerated.h

```
#ifndef STUDENTSVIEWGENERATED_H
#define STUDENTSVIEWGENERATED_H
#include <base/qmlComponentWrapper.h>
#include "studentsViewModel.h"
#include <QQuickItem>

class StudentsViewGenerated : public QmlComponentWrapper
{
    Q_OBJECT
public:
    explicit StudentsViewGenerated(QObject *parent = 0);
    virtual ~StudentsViewGenerated() {}
protected:
    virtual QObject *createComponent() override;
    QObject *actionImportStudents() const;
    StudentsViewModel m_studentsViewModel;
protected slots:
    virtual void importStudents() = 0;
    virtual void deleteCheckeds() = 0;
    virtual void exportCsv() = 0;
    virtual void addUser() = 0;
    virtual void itemChecked(int index, bool checked) = 0;
};

#endif // STUDENTSVIEWGENERATED_H
```

- The class generated is derived from QmlComponentWrapper. QmlComponentWrapper is a simple base class that only loads the qml from the resources. The specific qml to load, in this case studentsView.qml,  is defined in the createComponent method that is overrided in this generated class.

- For the 5 signals, importStudents, deleteCheckeds, exportCsv, addUser and itemChecked, a protected abstract method slot are generated. Note how itemChecked has generated with the correct type parameters.
```
  protected slots:
      virtual void importStudents() = 0;
      virtual void deleteCheckeds() = 0;
      virtual void exportCsv() = 0;
      virtual void addUser() = 0;
      virtual void itemChecked(int index, bool checked) = 0;
```

- For the context property c_students, a protected field StudentsViewModel m_studentsViewModel is generated. Like this context property in the qml file is used like a model property, the generator creates a new class where implements a derived QAbstractListModel class. 
- For the object that contains the property objectName in the qml file, aImportStudents, a protected QObject *actionImportStudents() const are generated. From this method is it possible to access it to change qml properties. For exemple, if you want to change the enabled property: actionImportStudents()->setProperty("enabled", true);

### studentsViewGenerated.cpp

```
#include "studentsViewGenerated.h"

StudentsViewGenerated::StudentsViewGenerated(QObject *parent): QmlComponentWrapper(parent)
{
}

QObject *StudentsViewGenerated::createComponent()
{
    QQmlContext *context = qmlContext();
    context->setContextProperty("c_students", &m_studentsViewModel);
    QObject *item = createComponentFromQrc("/sample/StudentsView.qml");
    connect(item, SIGNAL(importStudents()), this, SLOT(importStudents()));
    connect(item, SIGNAL(deleteCheckeds()), this, SLOT(deleteCheckeds()));
    connect(item, SIGNAL(exportCsv()), this, SLOT(exportCsv()));
    connect(item, SIGNAL(addUser()), this, SLOT(addUser()));
    connect(item, SIGNAL(itemChecked(int, bool)), this, SLOT(itemChecked(int, bool)));
    return item;
}

QObject *StudentsViewGenerated::actionImportStudents() const
{
    return component()->findChild<QObject *>("actionImportStudents");
}
```
You can see how createComponent creates de context property for the model, loads the specific qml and connects all the qml signals with the slots methods.

QObject *actionImportStudents() searchs the object with objectName actionImportStudents and returns it.

### studentsView.h

```
#ifndef STUDENTSVIEW_H
#define STUDENTSVIEW_H
#include "studentsViewGenerated.h"

class StudentsView : public StudentsViewGenerated
{
    Q_OBJECT
public:
    explicit StudentsView(QObject *parent = 0);
    virtual ~StudentsView() {}
protected slots:
    void importStudents() override;
    void deleteCheckeds() override;
    void exportCsv() override;
    void addUser() override;
    void itemChecked(int index, bool checked) override;
};

#endif // STUDENTSVIEW_H
```

- The class is inheriting the StudentsViewGenerated class.
- All virtual abstract methods are overrided.
- You can modify the h file to fit your needs. For exemple, you can declare new fields, signals, methods, ...

### studentsView.cpp

```
#include "studentsView.h"

StudentsView::StudentsView(QObject *parent): StudentsViewGenerated(parent)
{
}

void StudentsView::importStudents()
{
}

void StudentsView::deleteCheckeds()
{
}

void StudentsView::exportCsv()
{
}

void StudentsView::addUser()
{
}

void StudentsView::itemChecked(int index, bool checked)
{
}
```
- An empty implementation was generated.

- You can implement your code in the methods. Note that you can use the protected methods implemented in the StudentsViewGenerated class to access actionImportStudents object.

###studentsViewModelGenerated.h
```
#ifndef STUDENTSVIEWMODELGENERATED_H
#define STUDENTSVIEWMODELGENERATED_H
#include <QAbstractListModel>

class StudentsViewModelGenerated : public QAbstractListModel
{
public:
    explicit StudentsViewModelGenerated(QObject *parent = 0);
    virtual ~StudentsViewModelGenerated() {}
    QHash<int, QByteArray> roleNames() const override;
protected:
    enum StudentsRoles {
        nameRole = Qt::UserRole + 1,
        photoSrcRole,
        studentIdRole,
        emailRole
    };
};

#endif // STUDENTSVIEWMODELGENERATED_H
```

- The class is inherited from QAbstractListModel and overrides roleNames().
- Creates a protected enum with the roles found on the qml file. You can see model.name, model.photoSrc, model.studentId and model.email on the qml file.

### studentsViewModelGenerated.cpp

```
#include "studentsViewModelGenerated.h"

StudentsViewModelGenerated::StudentsViewModelGenerated(QObject *parent): QAbstractListModel(parent)
{
}

QHash<int, QByteArray>StudentsViewModelGenerated::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[StudentsRoles::nameRole] = "name";
    roles[StudentsRoles::photoSrcRole] = "photoSrc";
    roles[StudentsRoles::studentIdRole] = "studentId";
    roles[StudentsRoles::emailRole] = "email";
    return roles;
}
```

- The roleNames method are implemented, returning the qml roles.

### studentsViewModel.h

```
#ifndef STUDENTSVIEWMODEL_H
#define STUDENTSVIEWMODEL_H
#include "studentsViewModelGenerated.h"

class StudentsViewModel : public StudentsViewModelGenerated
{
public:
    explicit StudentsViewModel(QObject *parent = 0);
    virtual ~StudentsViewModel() {}
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
};
#endif // STUDENTSVIEWMODEL_H
```

- The class is inherited from its generated StudenstViewModelGenerated.
- The abstract methods inherited from QAbstractListModel, rowCount and data, are overrided.

### studentsViewModel.cpp

```
#include "studentsViewModel.h"

StudentsViewModel::StudentsViewModel(QObject *parent): StudentsViewModelGenerated(parent)
{
}

int StudentsViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return size;
}

QVariant StudentsViewModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if ((row < 0) || (row >= size)) {
        return QVariant();
    }
    switch (role) {
    case StudentsRoles::nameRole: {
        return 0;
    }
    case StudentsRoles::photoSrcRole: {
        return 0;
    }
    case StudentsRoles::studentIdRole: {
        return 0;
    }
    case StudentsRoles::emailRole: {
        return 0;
    }
    }
    return QVariant();
}
```

- The rowCount and data are filled with a default implementation. Note that this code doesn't compile, because an undefined size variable are used in both methods. This size variable must be changed with the correct value. However, this give a minimum compilation help. If you doesn't remember to change it, a compile error appears.

## What to do if I change the QML file?

Run again the generator. The generator **generates** **studentsViewGenerated.h**,  **studentsViewGenerated.cpp**, **studentsViewModelGenerated.h** and **studentsViewModelGenerated.cpp** but will **not generate** **studentsView.h**, **studentsView.cpp**, **studentsViewModel.h** or **studentsViewModel.cpp**. Like all your specific code is in studentsView.h, studentsView.cpp, studentsViewModel.h and studentsViewModel.cpp, you don't loose any code that you create, but if something changes on qml that affects the C++ relationship, the compiler gives an error.

For example, if you delete the signal exportCsv on your qml file, the studentsViewGenerated.h file will not have the virtual void exportCsv() = 0 method. Like studentsView.h has declared this method override, the compiler gives an error. Or if you changes the role name from name to firstName, the enum generated on studentsViewModelGenerated.h will be diferent than the roles that you use on the data method on studentsViewModel.cpp, so the compiler gives an error, because StudentsRoles::name doesn't exist.