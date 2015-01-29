#include "dsoaboutwidget.hh"
#include <QLocale>


DSOAboutWidget::DSOAboutWidget(QWidget *parent) :
  QTextBrowser(parent)
{
  setSource(QString("qrc:/doc/about.html"));
}
