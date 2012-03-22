#include "filmwindow.h"
#include "ui_filmwindow.h"

filmWindow::filmWindow(OMNetwork* c_net, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::filmWindow)
{
    ui->setupUi(this);
    _net = c_net;
    _ldModel = new LiveDeviceModel(_net, this);

    ui->devButtonList->setModel(_ldModel);
//    ui->devButtonList->setMovement(QListView::Free);
//    ui->devButtonList->setDragDropMode(QAbstractItemView::InternalMove);
}

filmWindow::~filmWindow()
{
    delete ui;
}
