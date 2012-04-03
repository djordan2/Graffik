#include "speedcontrolproxy.h"

#include <QDebug>

SpeedControlProxy::SpeedControlProxy(OMNetwork* c_net, LiveDeviceModel* c_ldm, QObject *parent) :
    QObject(parent)
{
    m_net = c_net;
    m_ldm = c_ldm;
    m_curDev = 0;

    QObject::connect(m_net, SIGNAL(deviceAdded(OMdeviceInfo*)), this, SLOT(_deviceAdded(OMdeviceInfo*)), Qt::QueuedConnection);
    QObject::connect(m_ldm, SIGNAL(deviceSelected(unsigned short))), this, SLOT(_deviceChange(unsigned short), Qt::QueuedConnection);

}


void SpeedControlProxy::_deviceAdded(OMdeviceInfo *p_dev) {
    qDebug() << "SCP: Received new device" << p_dev->name;
    m_devList.insert(p_dev->device->address(), p_dev);
}

void SpeedControlProxy::_deviceChange(unsigned short p_addr) {
    qDebug() << "SCP: Change Device" << p_addr;
    m_curDev = m_devList.value(p_addr);
}
