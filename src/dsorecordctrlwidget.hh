#ifndef DSORECORDCTRLWIDGET_HH
#define DSORECORDCTRLWIDGET_HH

#include <QWidget>
#include "dso.hh"

#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>

class DSORecordCtrlWidget : public QWidget
{
  Q_OBJECT
public:
  explicit DSORecordCtrlWidget(DSO *dso, QWidget *parent = 0);

protected slots:
  void onRecEnabled(bool enabled);
  void onRecDirSelected(QString path);
  void onSelectRecDir();

protected:
  DSO *_dso;
  QCheckBox *_recEnable;
  QLineEdit *_recDirectory;
  QLabel    *_recFile;
};

#endif // DSORECORDCTRLWIDGET_HH
