#include "dsoaboutwidget.hh"
#include <QLocale>


DSOAboutWidget::DSOAboutWidget(QWidget *parent) :
  QTextBrowser(parent)
{
  QString locale = QLocale().name();
  setSource(QString("qrc:/doc/about_%1.html").arg(locale));
  // If there is no translation -> resort to english text
  if (! this->source().isValid()) {
    setSource(QString("qrc:/doc/about_en_EN.html"));
  }
}
