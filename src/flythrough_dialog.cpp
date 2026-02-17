#include "flythrough_dialog.h"
#include "flythrough_core.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QtMath>
#include <qgisinterface.h>
#include <qgsmaplayercombobox.h>
#include <qgsmaplayerproxymodel.h>
#include <qgsproject.h>
#include <qgsrasterlayer.h>
#include <qgsvectorlayer.h>

// 3D Headers (Enabled for ALL platforms now)
#include <qgs3dmapcanvas.h>
#include <qgs3dmapsettings.h>
#include <qgscameracontroller.h>

FlyThroughDialog::FlyThroughDialog(QgisInterface *iface, QWidget *parent)
    : QDialog(parent), mIface(iface) {
  setupUi();
}

FlyThroughDialog::~FlyThroughDialog() {}

void FlyThroughDialog::setupUi() {
  setWindowTitle("FlyThrough Pro C++");
  resize(400, 500);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // --- Inputs Group ---
  QGroupBox *inputGroup = new QGroupBox("Input Layers", this);
  QFormLayout *inputLayout = new QFormLayout(inputGroup);

  mDemLayerCombo = new QgsMapLayerComboBox(this);
  mDemLayerCombo->setFilters(Qgis::LayerFilter::RasterLayer);
  inputLayout->addRow("DEM Layer:", mDemLayerCombo);

  mPathLayerCombo = new QgsMapLayerComboBox(this);
  mPathLayerCombo->setFilters(Qgis::LayerFilter::LineLayer |
                              Qgis::LayerFilter::PointLayer);
  inputLayout->addRow("Path Layer:", mPathLayerCombo);

  mainLayout->addWidget(inputGroup);

  // --- Camera Settings ---
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
  QMessageBox::information(
      this, "Preview",
      "Preview mode coming soon. Use 'Generate' to export video.");
}

void FlyThroughDialog::onGenerateClicked() {
  QgsVectorLayer *pathLayer =
      dynamic_cast<QgsVectorLayer *>(mPathLayerCombo->currentLayer());
  QgsRasterLayer *demLayer =
      dynamic_cast<QgsRasterLayer *>(mDemLayerCombo->currentLayer());

  if (!pathLayer || !demLayer) {
    QMessageBox::warning(this, "Error",
                         "Please select valid Path and DEM layers.");
    return;
  }

  // Find 3D Canvas
  Qgs3DMapCanvas *canvas3D = nullptr;
  QWidget *mainWindow = mIface->mainWindow();
  QList<Qgs3DMapCanvas *> canvases =
      mainWindow->findChildren<Qgs3DMapCanvas *>();

  if (!canvases.isEmpty()) {
    canvas3D = canvases.first();
  } else {
    QMessageBox::warning(this, "Error",
                         "No 3D Map View found. Please open a 3D View (View > "
                         "3D Map Views) and try again.");
    return;
  }

  /*
   * VIDEO EXPORT DISABLED TEMPORARILY
   * We skip FFmpeg setup to ensure the basic animation works on Windows.
   */

  FlyThroughCore core;
  // Retrieve from UI
  double speedKmh = mSpeedSpin->value();
  double altitude = mCameraHeightSpin->value();
  double pitch =
      -(mCameraPitchSpin->value()); // QGIS uses negative for looking down

  if (!core.generateKeyframes(pathLayer, demLayer, speedKmh, altitude, pitch,
                              mBankingCheck->isChecked())) {
    QMessageBox::warning(this, "Error",
                         "Failed to generate flight path. Check geometry.");
    return;
  }

  QProgressDialog progress("Animating ...", "Cancel", 0, 100, this);
  progress.setWindowModality(Qt::WindowModal);

  double duration = core.totalDuration();
  int fps = 30;
  int totalFrames = static_cast<int>(duration * fps);

  if (totalFrames <= 0)
    totalFrames = 1;

  QgsCameraController *camera = canvas3D->cameraController();

  for (int i = 0; i < totalFrames; ++i) {
    if (progress.wasCanceled())
      break;
    progress.setValue((i * 100) / totalFrames);

    double t = static_cast<double>(i) / fps;
    CameraKeyframe kf = core.interpolate(t);

    // Update Camera
    QgsVector3D pos = kf.position;

    // Convert yaw/pitch to lookAt
    double yawRad = qDegreesToRadians(kf.yaw);
    double pitchRad = qDegreesToRadians(kf.pitch);

    // Look vector
    double dx = std::sin(yawRad) * std::cos(pitchRad);
    double dy = std::cos(yawRad) * std::cos(pitchRad);
    double dz = std::sin(pitchRad);

    QgsVector3D target = pos + QgsVector3D(dx, dy, dz) * 100.0;

    camera->setLookAt(target);
    camera->setPos(pos);

    // Force repaint
    canvas3D->repaint();
    QCoreApplication::processEvents(); // Allow UI to update
  }

  progress.setValue(100);
  QMessageBox::information(this, "Success",
                           "Flythrough Complete! (Video Export Skipped)");
}
