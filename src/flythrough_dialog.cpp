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
#include <qgisinterface.h>
#include <qgs3dmapcanvas.h>
#include <qgs3dmapsettings.h>
#include <qgscameracontroller.h>
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
  QMessageBox::information(
      this, "Preview",
      "Preview mode coming soon. Use 'Generate' to export video.");
}

void FlyThroughDialog::onGenerateClicked() {
  QgsVectorLayer *pathLayer =
      dynamic_cast<QgsVectorLayer *>(mPathLayerComboBox->currentLayer());
  QgsRasterLayer *demLayer =
      dynamic_cast<QgsRasterLayer *>(mDemLayerComboBox->currentLayer());

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

  QString outputFilename = QFileDialog::getSaveFileName(
      this, "Save Video", QDir::homePath(), "MP4 Video (*.mp4)");
  if (outputFilename.isEmpty())
    return;

  FlyThroughCore core;
  // TODO: Retrieve from UI
  double speedKmh = 100.0;
  double altitude = 100.0;
  double pitch = -20.0;

  if (!core.generateKeyframes(pathLayer, demLayer, speedKmh, altitude, pitch,
                              false)) {
    QMessageBox::warning(this, "Error",
                         "Failed to generate flight path. Check geometry.");
    return;
  }

  QProcess ffmpeg;
  QString program = "ffmpeg";
  QStringList arguments;
  arguments << "-y" << "-f" << "image2pipe" << "-vcodec" << "png" << "-r"
            << "30" << "-i" << "-"
            << "-c:v" << "libx264" << "-pix_fmt" << "yuv420p" << outputFilename;

  ffmpeg.start(program, arguments);
  if (!ffmpeg.waitForStarted()) {
    QMessageBox::critical(
        this, "Error",
        "Could not start FFmpeg. Is it installed and in your PATH?");
    return;
  }

  QProgressDialog progress("Rendering ...", "Cancel", 0, 100, this);
  progress.setWindowModality(Qt::WindowModal);

  double duration = core.totalDuration();
  int fps = 30;
  int totalFrames = static_cast<int>(duration * fps);

  QgsCameraController *camera = canvas3D->cameraController();

  for (int i = 0; i < totalFrames; ++i) {
    if (progress.wasCanceled())
      break;
    progress.setValue((i * 100) / totalFrames);

    double t = static_cast<double>(i) / fps;
    CameraKeyframe kf = core.interpolate(t);

    // Update Camera (Approximation for now)
    QgsVector3D pos = kf.position;

    // Look forward based on Yaw
    double yawRad = qDegreesToRadians(kf.yaw);
    double pitchRad = qDegreesToRadians(kf.pitch);

    double dx = std::sin(yawRad) * std::cos(pitchRad);
    double dy = std::cos(yawRad) * std::cos(pitchRad);
    double dz = std::sin(pitchRad);

    QgsVector3D forward(dx, dy, dz);
    QgsVector3D target = pos + forward * 100.0; // Look 100m ahead

    camera->setLookAt(target);
    // camera->setCenter(pos); // If available

    // Force repaint
    canvas3D->repaint();
    QCoreApplication::processEvents();

    QImage img = canvas3D->capture();
    img.save(&ffmpeg, "PNG");
  }

  ffmpeg.closeWriteChannel();
  ffmpeg.waitForFinished();

  progress.setValue(100);
  QMessageBox::information(this, "Success", "Video Export Complete!");
}
