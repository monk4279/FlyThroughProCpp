#ifndef FLYTHROUGH_DIALOG_H
#define FLYTHROUGH_DIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <qgisinterface.h>
#include <qgsmaplayercombobox.h>

class FlyThroughDialog : public QDialog {
  Q_OBJECT

public:
  explicit FlyThroughDialog(QgisInterface *iface, QWidget *parent = nullptr);
  ~FlyThroughDialog();

private slots:
  void onGenerateClicked();
  void onPreviewClicked();

private:
  QgisInterface *mIface = nullptr;
  void setupUi();

  // UI Elements
  QgsMapLayerComboBox *mDemLayerCombo = nullptr;
  QgsMapLayerComboBox *mPathLayerCombo = nullptr;
  QComboBox *maltitudeModeCombo = nullptr;
  QDoubleSpinBox *mCameraHeightSpin = nullptr;
  QDoubleSpinBox *mCameraPitchSpin = nullptr;
  QDoubleSpinBox *mSpeedSpin = nullptr;
  QDoubleSpinBox *mLookAheadSpin = nullptr;
  QCheckBox *mBankingCheck = nullptr;
  QCheckBox *mExportVideoCheck = nullptr;
};

#endif // FLYTHROUGH_DIALOG_H
