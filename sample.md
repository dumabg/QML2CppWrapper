# NewStudentView.qml sample

```
import QtQuick 2.9
import QtQuick.Window 2.3
Window {
    signal selectPhoto
    signal okClicked
    signal cancelClicked
    property alias name: name.text
    readonly property alias studentId: studentId.text
    readonly property alias email: email.text 
    width: 300
    height: 320
    visible: true
    flags: Qt.Dialog | Qt.MSWindowsFixedSizeDialogHint | Qt.WindowTitleHint
    modality: Qt.ApplicationModal
    Column {
        id: content
        width: parent.width - 20
        x: 10
        y: 10
        spacing: 20
        TextInput2 {
            id: name
            width: parent.width
            font.capitalization: Font.Capitalize
            caption: qsTr("Name")
        }
        TextInput2 {
            id: studentId
            width: 100
            caption: qsTr("Id")
            inputMethodHints: Qt.ImhDigitsOnly
            inputMask: "99/9999999"
        }
        TextInput2 {
            id: email
            width: parent.width
            font.capitalization: Font.AllLowercase
            caption: qsTr("Email")
            inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhLowercaseOnly
        }
        Column {
            spacing: 3
            Text2 {
                text: qsTr("Photo")
                wrapMode: Text.WordWrap
            }
            Rectangle {
                width: 45
                height: width
                border.color: "black"
                border.width: 1
                color: "transparent"
                radius: 3
                smooth: true
                Image {
                    id: img         
                    width: 45
                    height: 45
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: selectPhoto()
                }
            }
        }
        OkCancelButtons {
            id: buttons
            okEnabled: (name.text.trim().length > 0) && (studentId.text.trim().length > 0)
                                     && ((email.text.indexOf('.', email.text.indexOf('@'))) > -1)
        }
    }
    Component.onCompleted: {
        buttons.onOkClicked.connect(okClicked)
        buttons.onCancelClicked.connect(cancelClicked)
    }
}
```
## What is generated and why?

For this qml four files are generated: **newStudentView.h**, **newStudentView.cpp**, **newStudentViewGenerated.h** and **newStudentViewGenerated.cpp**.

### newStudentViewGenerated.h

```
#ifndef NEWSTUDENTVIEWGENERATED_H
#define NEWSTUDENTVIEWGENERATED_H
#include <base/qmlComponentWrapper.h>
#include <QString>
#include <QQuickItem>

class NewStudentViewGenerated : public QmlComponentWrapper
{
    Q_OBJECT
public:
    explicit NewStudentViewGenerated(QObject *parent = 0);
    virtual ~NewStudentViewGenerated() {}
protected:
    virtual QObject *createComponent() override;
    QString name() const;
    void setName(const QString &value);
    QString studentId() const;
    QString email() const;
protected slots:
    virtual void selectPhoto() = 0;
    virtual void okClicked() = 0;
    virtual void cancelClicked() = 0;
};

#endif // NEWSTUDENTVIEWGENERATED_H
```
- The class generated is derived from QmlComponentWrapper. QmlComponentWrapper is a simple base class that only loads the qml from the resources. The specific qml to load, in this case NewStudentView.qml,  is defined in the createComponent method that is overrided in this generated class.
- For the 3 signals, selectPhoto, okClicked and cancelClicked, a protected slot abstract method are generated: 

```
protected slots:
    virtual void selectPhoto() = 0;
    virtual void okClicked() = 0;
    virtual void cancelClicked() = 0;
```

- For the 3 properties, name, studentId and email, a correspondent getter and setter are created:
```
    QString name() const;
    void setName(const QString &value);
    QString studentId() const;
    QString email() const;
```
No set methods generated for studentId and email because the property are readonly.

### newStudentViewGenerated.cpp

```
#include "newStudentViewGenerated.h"

NewStudentViewGenerated::NewStudentViewGenerated(QObject *parent): QmlComponentWrapper(parent)
{
}

QObject *NewStudentViewGenerated::createComponent()
{
    QObject *item = createComponentFromQrc("/sample/NewStudentView.qml");
    connect(item, SIGNAL(selectPhoto()), this, SLOT(selectPhoto()));
    connect(item, SIGNAL(okClicked()), this, SLOT(okClicked()));
    connect(item, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
    return item;
}

QString NewStudentViewGenerated::name() const
{
    return component()->property("name").toString();
}

void NewStudentViewGenerated::setName(const QString &value)
{
    component()->setProperty("name", value);
}

QString NewStudentViewGenerated::studentId() const
{
    return component()->property("studentId").toString();
}

QString NewStudentViewGenerated::email() const
{
    return component()->property("email").toString();
}
```
You can see how createComponent loads the specific qml and connects all the qml signals with the slots methods.

### newStudentView.h

```
#ifndef NEWSTUDENTVIEW_H
#define NEWSTUDENTVIEW_H
#include "newStudentViewGenerated.h"

class NewStudentView : public NewStudentViewGenerated
{
    Q_OBJECT
public:
    explicit NewStudentView(QObject *parent = 0);
    virtual ~NewStudentView() {}
protected slots:
    void selectPhoto() override;
    void okClicked() override;
    void cancelClicked() override;
};

#endif // NEWSTUDENTVIEW_H
```

* The class is inheriting the NewStudentViewGenerated class.
* All virtual abstract methods are overrided.
* You can modify the h file to fit your needs. For exemple, you can declare new fields, signals, methods, ...

### newStudentView.cpp

```
#include "newStudentView.h"

NewStudentView::NewStudentView(QObject *parent): NewStudentViewGenerated(parent)
{
}

void NewStudentView::selectPhoto()
{
}

void NewStudentView::okClicked()
{
}
void NewStudentView::cancelClicked()
{
}
```
- An empty implementation was generated.
- You can implement your code in the methods. Note that you can use the protected methods implemented in the NewStudentViewGenerated class to access the qml properties: name, studentId and email.

## What to do if I change the QML file?

Run again the generator. The generator **generates** **newStudentViewGenerated.h** and **newStudentViewGenerated.cpp**, but will **not generate** neither **newStudentView.h** nor **newStudentView.cpp**. Like all your specific code is in newStudentView.h and newStudentView.cpp, you don't loose any code that you create, but if something changes on qml that affects the C++ relationship, the compiler gives an error.

For example, if you delete the signal selectPhoto on your qml file, the newStudentViewGenerated.h file will not have the virtual void selectPhoto() = 0 method. Like newStudentView.h has declared this method override, the compiler gives an error. Or if you changes the studentId type to long and you have used the studentId() method, the compiler gives an error, because you are using studentId() like a QString and not like a long.