// main.cpp - entry point of the Student Record Management System
#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Use the same light look on every PC, even if Windows is in dark mode.
    // (In dark mode Qt used white text, which was invisible on white boxes.)
    app.setStyle(QStyleFactory::create("Fusion"));
    QPalette pal;
    pal.setColor(QPalette::Window,          QColor("#eef1f6"));
    pal.setColor(QPalette::WindowText,      QColor("#1f2a44"));
    pal.setColor(QPalette::Base,            QColor("#ffffff"));
    pal.setColor(QPalette::AlternateBase,   QColor("#f4f6fb"));
    pal.setColor(QPalette::Text,            QColor("#1f2a44"));
    pal.setColor(QPalette::PlaceholderText, QColor("#8a94a8"));
    pal.setColor(QPalette::Button,          QColor("#3b6fd4"));
    pal.setColor(QPalette::ButtonText,      QColor("#ffffff"));
    pal.setColor(QPalette::ToolTipBase,     QColor("#ffffff"));
    pal.setColor(QPalette::ToolTipText,     QColor("#1f2a44"));
    pal.setColor(QPalette::Highlight,       QColor("#3b6fd4"));
    pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    app.setPalette(pal);

    MainWindow w;
    w.show();
    return app.exec();
}
