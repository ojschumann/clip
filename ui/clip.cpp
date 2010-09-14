#include "clip.h"
#include "ui_clip.h"
#include "crystaldisplay.h"
#include <QMdiSubWindow>

Clip::Clip(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Clip)
{
    ui->setupUi(this);
}

Clip::~Clip()
{
    delete ui;
}


void Clip::on_actionNew_Crystal_triggered(bool) {
    QMdiSubWindow* w = ui->mdiArea->addSubWindow(new CrystalDisplay(this));
    w->show();
}
