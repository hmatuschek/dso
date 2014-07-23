#include "sdr.hh"

#include <QMainWindow>
#include <QLocale>
#include <QTranslator>

#include "dso.hh"


/** Tiny digital storage oscilloscope application. */
int main(int argc, char *argv[]) {

  DSO dso(argc, argv);

  QString locale = QLocale::system().name();
  QTranslator translator;
  translator.load(QString(":/i18n/%1.qm").arg(locale));
  dso.installTranslator(&translator);

  QMainWindow win;
  win.setCentralWidget(dso.view());
  win.show();

  dso.exec();

  return 0;
}
