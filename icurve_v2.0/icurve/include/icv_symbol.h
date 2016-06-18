#pragma once

#include <qwt_symbol.h>

class IcvSymbol:public QwtSymbol
{
public:
    enum Style
    {
        Arrow,
        Star
    };

    IcvSymbol(Style = Arrow);
    ~IcvSymbol(void);
};
