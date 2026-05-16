#pragma once

#include <QIcon>

class KIconLoader
{
public:
    enum StdSizes {
        SizeSmall = 16,
        SizeSmallMedium = 22,
        SizeMedium = 32,
        SizeLarge = 48,
    };
};

class StatusNotifierItemSource
{
public:
    void overlayIcon(QIcon *icon, QIcon *overlay);
};
