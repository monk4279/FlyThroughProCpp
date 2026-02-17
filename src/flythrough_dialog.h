#ifndef FLYTHROUGH_DIALOG_H
#define FLYTHROUGH_DIALOG_H

#include <QDialog>
#include <qgisinterface.h>

class FlyThroughDialog : public QDialog {
  Q_OBJECT

public:
  explicit FlyThroughDialog(QgisInterface *iface, QWidget *parent = nullptr);
  ~FlyThroughDialog();

private:
  QgisInterface *mIface = nullptr;
  void setupUi();
};

#endif // FLYTHROUGH_DIALOG_H
