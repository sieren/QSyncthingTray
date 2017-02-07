
#include <qst/syncconnection.h>
#include <qst/syncdeviceviewmodel.h>
#include <QObject>
#include <QModelIndex>

namespace qst
{
namespace model
{
using namespace connector;
SyncDeviceModel::SyncDeviceModel(SyncConnection &connection, QObject *parent) :
   mConnection(connection)
  ,mDevices(connection.devices())
{
  connect(&mConnection, &SyncConnection::onDevicesChanged,
    this, &SyncDeviceModel::onDevicesChanged);
  connect(&mConnection, &SyncConnection::onConfigChanged,
    this, &SyncDeviceModel::onConfigChanged);
}


const SyncDevice *SyncDeviceModel::deviceInfo(const QModelIndex &index) const
{
  auto idx = static_cast<size_t>(index.row());
  auto retVal = idx < mDevices.size() ? &mDevices.at(idx) : nullptr;
  return (index.parent().isValid() ? deviceInfo(index.parent()) : retVal);
}


void SyncDeviceModel::onConfigChanged()
{
  
}


void SyncDeviceModel::onDevicesChanged()
{
  
}


void SyncDeviceModel::onDeviceStatusChanged(const SyncDevice&, int index)
{
  
}
} // model namespace
} // qst namespace
