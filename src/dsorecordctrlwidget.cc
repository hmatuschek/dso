#include "dsorecordctrlwidget.hh"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QDate>


DSORecordCtrlWidget::DSORecordCtrlWidget(DSO *dso, QWidget *parent)
  : QWidget(parent), _dso(dso)
{
  _recEnable    = new QCheckBox();
  QObject::connect(_recEnable, SIGNAL(clicked(bool)), this, SLOT(onRecEnabled(bool)));
  _recEnable->setChecked(false);
  QPushButton *recDirSelect = new QPushButton("...");
  QObject::connect(recDirSelect, SIGNAL(clicked()), this, SLOT(onSelectRecDir()));
  _recDirectory = new QLineEdit();
  QObject::connect(_recDirectory, SIGNAL(textEdited(QString)), this, SLOT(onRecDirSelected(QString)));
  _recFile      = new QLabel(tr("<none>"));

  QFormLayout *rec_layout = new QFormLayout();
  rec_layout->addRow(tr("Enable"), _recEnable);
  QHBoxLayout *rec_dir_layout = new QHBoxLayout();
  rec_dir_layout->setContentsMargins(0,0,0,0);
  rec_dir_layout->addWidget(_recDirectory, 1);
  rec_dir_layout->addWidget(recDirSelect, 0);
  rec_layout->addRow(tr("Directory"), rec_dir_layout);
  rec_layout->addRow(tr("File"), _recFile);
  this->setLayout(rec_layout);
}




void
DSORecordCtrlWidget::onRecEnabled(bool enabled) {
  QString path = _recDirectory->text();
  if (enabled) {
    if ((0 == path.size()) || !QDir(_recDirectory->text()).exists()) {
      _recEnable->setChecked(false);
      _dso->stopRecord();
    } else {
      // Determine file name
      QString filepattern = QString("DSO-REC-%1-%2.csv").arg(QDate::currentDate().toString(Qt::ISODate));
      int count = 1;
      while ( QFile::exists(path + "/" + filepattern.arg(count)) ) { count++; }

      // Start recording.
      _dso->record(path + "/" + filepattern.arg(count));
      _recFile->setText(filepattern.arg(count));
    }
  } else {
    _dso->stopRecord();
  }
}

void
DSORecordCtrlWidget::onSelectRecDir() {
  QString path = QFileDialog::getExistingDirectory();
  if (0 == path.size()) { return; }
  _recDirectory->setText(path);
}

void
DSORecordCtrlWidget::onRecDirSelected(QString path) {
  if (_dso->isRecording()) {
    _dso->stopRecord();
    _recEnable->setChecked(false);
  }

  QDir dir(path);
  if (!dir.exists()) { return; }
}

