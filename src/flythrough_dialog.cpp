#include "flythrough_dialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

FlyThroughDialog::FlyThroughDialog(QgisInterface *iface, QWidget *parent)
    : QDialog(parent), mIface(iface) {
  setupUi();
}

FlyThroughDialog::~FlyThroughDialog() {}

void FlyThroughDialog::setupUi() {
  setWindowTitle("FlyThrough Pro C++");

  QVBoxLayout *layout = new QVBoxLayout(this);

  QLabel *label =
      new QLabel("FlyThrough Pro C++ Plugin\n(Under Construction)", this);
  label->setAlignment(Qt::AlignCenter);
  layout->addWidget(label);

  QPushButton *btn = new QPushButton("Close", this);
  connect(btn, &QPushButton::clicked, this, &QDialog::accept);
  layout->addWidget(btn);
}
