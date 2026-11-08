#pragma once
#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QString>

// Vector-painted line icons (no resource files). Names:
// barcode scan camera upload download copy check info trash shield user history print hash braces
// server sliders pause bolt warn image target ruler backspace folder bug gauge arrow
QPixmap iconPixmap(const QString &name, const QColor &color, int size = 20);
QIcon   makeIcon(const QString &name, const QColor &color, int size = 20);
QLabel *iconLabel(const QString &name, const QColor &color, int size = 20);
