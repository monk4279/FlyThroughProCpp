#include "flythrough_dialog.h"
#include "flythrough_core.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <qgs3dmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsmaplayercombobox.h>
#include <qgsmaplayerproxymodel.h>
#include <qgsproject.h>
#include <qgsrasterlayer.h>
#include <qgsvectorlayer.h>

FlyThroughDialog::FlyThroughDialog(QgisInterface *iface, QWidget *parent)
    : QDialog(parent), mIface(iface) {
  setupUi();
}

FlyThroughDialog::~FlyThroughDialog() {}

void FlyThroughDialog::setupUi() {
  setWindowTitle("FlyThrough Pro - 3D Animation");
  resize(600, 700);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // --- Basic Settings Group ---
  QGroupBox *basicGroup = new QGroupBox("Basic Settings", this);
  QFormLayout *basicLayout = new QFormLayout();

  mDemLayerCombo = new QgsMapLayerComboBox(this);
  mDemLayerCombo->setFilters(QgsMapLayerProxyModel::Filter::RasterLayer);
  basicLayout->addRow("DEM Layer:", mDemLayerCombo);

  mPathLayerCombo = new QgsMapLayerComboBox(this);
  mPathLayerCombo->setFilters(QgsMapLayerProxyModel::Filter::VectorLayer);
  basicLayout->addRow("Path Layer:", mPathLayerCombo);

  mOverlayLayerCombo = new QgsMapLayerComboBox(this);
  mOverlayLayerCombo->setFilters(QgsMapLayerProxyModel::Filter::RasterLayer |
                                 QgsMapLayerProxyModel::Filter::VectorLayer);
  mOverlayLayerCombo->setAllowEmptyLayer(true);
  basicLayout->addRow("Overlay (optional):", mOverlayLayerCombo);

  mAltitudeModeCombo = new QComboBox(this);
  mAltitudeModeCombo->addItem("Above Safe Path");
  mAltitudeModeCombo->addItem("Fixed Altitude (AMSL)");
  basicLayout->addRow("Altitude Mode:", mAltitudeModeCombo);

  mCameraHeightSpin = new QDoubleSpinBox(this);
  mCameraHeightSpin->setRange(1.0, 10000.0);
  mCameraHeightSpin->setValue(200.0);
  mCameraHeightSpin->setSuffix(" m");
  basicLayout->addRow("Camera Height:", mCameraHeightSpin);

  mCameraPitchSpin = new QDoubleSpinBox(this);
  mCameraPitchSpin->setRange(-90.0, 90.0);
  mCameraPitchSpin->setValue(65.0);
  mCameraPitchSpin->setSuffix("°");
  basicLayout->addRow("Camera Pitch (Down):", mCameraPitchSpin);

  mFovSpin = new QDoubleSpinBox(this);
  mFovSpin->setRange(10.0, 120.0);
  mFovSpin->setValue(45.0);
  mFovSpin->setSuffix("°");
  basicLayout->addRow("Field of View:", mFovSpin);

  mVerticalExagSpin = new QDoubleSpinBox(this);
  mVerticalExagSpin->setRange(0.1, 10.0);
  mVerticalExagSpin->setValue(1.0);
  mVerticalExagSpin->setSingleStep(0.1);
  basicLayout->addRow("Vertical Exaggeration:", mVerticalExagSpin);

  basicGroup->setLayout(basicLayout);
  mainLayout->addWidget(basicGroup);

  // --- Animation Settings Group ---
  QGroupBox *animGroup = new QGroupBox("Animation Settings", this);
  QFormLayout *animLayout = new QFormLayout();

  mSpeedSpin = new QDoubleSpinBox(this);
  mSpeedSpin->setRange(1.0, 1000.0);
  mSpeedSpin->setValue(50.0);
  mSpeedSpin->setSuffix(" m/s");
  animLayout->addRow("Speed:", mSpeedSpin);

  mSmoothingSpin = new QSpinBox(this);
  mSmoothingSpin->setRange(0, 10);
  mSmoothingSpin->setValue(0);
  animLayout->addRow("Path Smoothing:", mSmoothingSpin);

  mBankingCheck = new QCheckBox("Enable Camera Banking", this);
  mBankingCheck->setChecked(true);
  animLayout->addRow(mBankingCheck);

  mBankingFactorSpin = new QDoubleSpinBox(this);
  mBankingFactorSpin->setRange(0.0, 2.0);
  mBankingFactorSpin->setValue(0.5);
  mBankingFactorSpin->setSingleStep(0.1);
  animLayout->addRow("Banking Factor:", mBankingFactorSpin);

  mLookaheadSpin = new QDoubleSpinBox(this);
  mLookaheadSpin->setRange(0.0, 5000.0);
  mLookaheadSpin->setValue(1000.0);
  mLookaheadSpin->setSuffix(" m");
  animLayout->addRow("Look-ahead Distance:", mLookaheadSpin);

  mFpsSpin = new QSpinBox(this);
  mFpsSpin->setRange(24, 120);
  mFpsSpin->setValue(30);
  animLayout->addRow("FPS:", mFpsSpin);

  animGroup->setLayout(animLayout);
  mainLayout->addWidget(animGroup);

  // --- Rendering Group ---
  QGroupBox *renderGroup = new QGroupBox("Rendering", this);
  QFormLayout *renderLayout = new QFormLayout();

  mTerrainShadingCheck = new QCheckBox("Terrain Shading", this);
  mTerrainShadingCheck->setChecked(true);
  renderLayout->addRow(mTerrainShadingCheck);

  mExportVideoCheck = new QCheckBox("Export Video (DISABLED)", this);
  mExportVideoCheck->setEnabled(false);
  mExportVideoCheck->setToolTip("Video export not implemented in this version");
  renderLayout->addRow(mExportVideoCheck);

  renderGroup->setLayout(renderLayout);
  mainLayout->addWidget(renderGroup);

  // --- Buttons ---
  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  QPushButton *generateBtn = new QPushButton("Generate Flythrough", this);
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
  onGenerateClicked(); // Same as generate for now
}

void FlyThroughDialog::onGenerateClicked() {
  // Validate inputs
  if (!mDemLayerCombo->currentLayer() || !mPathLayerCombo->currentLayer()) {
    QMessageBox::warning(this, "Missing Layers",
                         "Please select both DEM and path layers.");
    return;
  }

  // Build parameters
  FlythroughParams params;
  params.pathLayer =
      qobject_cast<QgsVectorLayer *>(mPathLayerCombo->currentLayer());
  params.demLayer =
      qobject_cast<QgsRasterLayer *>(mDemLayerCombo->currentLayer());
  params.overlayLayer = mOverlayLayerCombo->currentLayer();
  params.altitudeMode = mAltitudeModeCombo->currentText();
  params.cameraHeight = mCameraHeightSpin->value();
  params.cameraPitch = mCameraPitchSpin->value();
  params.fieldOfView = mFovSpin->value();
  params.verticalExaggeration = mVerticalExagSpin->value();
  params.speed = mSpeedSpin->value();
  params.smoothing = mSmoothingSpin->value();
  params.enableBanking = mBankingCheck->isChecked();
  params.bankingFactor = mBankingFactorSpin->value();
  params.terrainShading = mTerrainShadingCheck->isChecked();
  params.lookaheadDistance = mLookaheadSpin->value();
  params.fps = mFpsSpin->value();

  // Create and run core logic
  FlyThroughCore *core = new FlyThroughCore(mIface);
  if (core->generateFlythrough(params)) {
    // Success - close dialog
    accept();
  }
  // Core manages its own lifecycle (deletes when animation stops)
}
