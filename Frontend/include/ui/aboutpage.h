#pragma once
#include <QWidget>

// Screen 5 - "About Project": what BarcodeHub is, how the barcode format works,
// the tech stack, the course details and the team.
class AboutPage : public QWidget {
    Q_OBJECT
public:
    explicit AboutPage(QWidget *parent = nullptr);
};
