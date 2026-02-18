#include "flythrough_dialog.h"
#include "flythrough_core.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <qgis/qgs3dmapcanvas.h>
#include <qgsmaplayercombobox.h>
#include <qgsmaplayerproxymodel.h>
#include <qgsproject.h>

FlyThroughDialog::FlyThroughDialog(QgisInterface *iface, QWidget *parent)
    : QDialog(parent), mIface(iface) {
  setupUi();
}

FlyThroughDialog::~FlyThroughDialog() {}

void FlyThroughDialog::setupUi() {
  setWindowTitle("FlyThrough Pro C++");
  resize(400, 500);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // --- Input Layers Group ---
  QGroupBox *inputGroup = new QGroupBox("Input Data", this);
  QFormLayout *inputLayout = new QFormLayout(inputGroup);

  mPathLayerCombo = new QgsMapLayerComboBox(this);
  mPathLayerCombo->setFilters(QgsMapLayerProxyModel::LineLayer);
  inputLayout->addRow("Path Layer (Line):", mPathLayerCombo);

  mDemLayerCombo = new QgsMapLayerComboBox(this);
  mDemLayerCombo->setFilters(QgsMapLayerProxyModel::RasterLayer);
  inputLayout->addRow("DEM Layer (Raster):", mDemLayerCombo);

  mainLayout->addWidget(inputGroup);

  // --- Camera Settings Group ---
  QGroupBox *cameraGroup = new QGroupBox("Camera Settings", this);
  QFormLayout *camLayout = new QFormLayout(cameraGroup);

  maltitudeModeCombo = new QComboBox(this);
  maltitudeModeCombo->addItems({"Above Safe Path", "Fixed Altitude (AMSL)"});
  camLayout->addRow("Altitude Mode:", maltitudeModeCombo);

  mCameraHeightSpin = new QDoubleSpinBox(this);
  mCameraHeightSpin->setRange(0, 10000);
  mCameraHeightSpin->setValue(200);
  mCameraHeightSpin->setSuffix(" m");
  camLayout->addRow("Height / Clearance:", mCameraHeightSpin);

  mCameraPitchSpin = new QDoubleSpinBox(this);
  mCameraPitchSpin->setRange(-90, 90);
  mCameraPitchSpin->setValue(65);
  mCameraPitchSpin->setSuffix(" deg");
  camLayout->addRow("Pitch (Look Down):", mCameraPitchSpin);

  mLookAheadSpin = new QDoubleSpinBox(this);
  mLookAheadSpin->setRange(0, 2000);
  mLookAheadSpin->setValue(100);
  mLookAheadSpin->setSuffix(" m");
  camLayout->addRow("Look Ahead Dist:", mLookAheadSpin);

  mainLayout->addWidget(cameraGroup);

  // --- Animation Settings ---
  QGroupBox *animGroup = new QGroupBox("Animation", this);
  QFormLayout *animLayout = new QFormLayout(animGroup);

  mSpeedSpin = new QDoubleSpinBox(this);
  mSpeedSpin->setRange(1, 10000);
  mSpeedSpin->setValue(100);
  mSpeedSpin->setSuffix(" km/h");
  animLayout->addRow("Speed:", mSpeedSpin);

  mBankingCheck = new QCheckBox("Enable Banking", this);
  mBankingCheck->setChecked(true);
  animLayout->addRow(mBankingCheck);

  mExportVideoCheck = new QCheckBox("Export Video (requires FFmpeg)", this);
  animLayout->addRow(mExportVideoCheck);

  mainLayout->addWidget(animGroup);

  // --- Buttons ---
  QHBoxLayout *btnLayout = new QHBoxLayout();

  QPushButton *previewBtn = new QPushButton("Preview", this);
  connect(previewBtn, &QPushButton::clicked, this,
          &FlyThroughDialog::onPreviewClicked);
  btnLayout->addWidget(previewBtn);

  QPushButton *generateBtn = new QPushButton("Generate", this);
  generateBtn->setDefault(true);
  connect(generateBtn, &QPushButton::clicked, this,
          &FlyThroughDialog::onGenerateClicked);
  btnLayout->addWidget(generateBtn);

  QPushButton *closeBtn = new QPushButton("Close", this);
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
  btnLayout->addWidget(closeBtn);

  mainLayout->addLayout(btnLayout);
}

void FlyThroughDialog::onPreviewClicked() {
  // TODO: Connect to Core Logic
}

void FlyThroughDialog::onGenerateClicked() {
  // TODO: Connect to Core Logic
}
