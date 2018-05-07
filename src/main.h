#ifndef MAIN_H
#define MAIN_H
#include "mainGenerated.h"

class main : public mainGenerated
{
    Q_OBJECT
public:
    explicit main(QObject *parent = 0);
    virtual ~main() {}
protected slots:
};

#endif // MAIN_H