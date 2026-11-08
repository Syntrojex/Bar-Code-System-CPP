#pragma once
#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QString>

QPixmap iconPixmap(const QString &name, const QColor &color, int size = 20);
QIcon   makeIcon(const QString &name, const QColor &color, int size = 20);
QLabel *iconLabel(const QString &name, const QColor &color, int size = 20);
