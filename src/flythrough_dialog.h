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

  // UI Elements (matching Python dialog)
  QgsMapLayerComboBox *mDemLayerCombo = nullptr;
  QgsMapLayerComboBox *mPathLayerCombo = nullptr;
  QgsMapLayerComboBox *mOverlayLayerCombo = nullptr;
  QComboBox *mAltitudeModeCombo = nullptr;
  QDoubleSpinBox *mCameraHeightSpin = nullptr;
  QDoubleSpinBox *mCameraPitchSpin = nullptr;
  QDoubleSpinBox *mFovSpin = nullptr;
  QDoubleSpinBox *mVerticalExagSpin = nullptr;
  QDoubleSpinBox *mSpeedSpin = nullptr;
  QSpinBox *mSmoothingSpin = nullptr;
  QDoubleSpinBox *mBankingFactorSpin = nullptr;
  QDoubleSpinBox *mLookaheadSpin = nullptr;
  QSpinBox *mFpsSpin = nullptr;
  QCheckBox *mBankingCheck = nullptr;
  QCheckBox *mTerrainShadingCheck = nullptr;
  QCheckBox *mExportVideoCheck = nullptr;
};

#endif // FLYTHROUGH_DIALOG_H
