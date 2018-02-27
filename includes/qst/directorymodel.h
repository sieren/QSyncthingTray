#ifndef _DIRECTORYMODEL_H
#define _DIRECTORYMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

#include <vector>

namespace qst
{
namespace data
{
  class SyncConnection;
  struct SyncthingDirectory;
}

namespace model
{

class SyncthingDirectoryModel : public QAbstractItemModel {
  Q_OBJECT
public:
  enum SyncthingDirectoryModelRole {
    DirectoryStatus = Qt::UserRole + 1,
    DirectoryPaused,
    DirectoryStatusString,
    DirectoryStatusColor,
    DirectoryId,
    DirectoryPath,
    DirectoryDetail,
  };
  
  explicit SyncthingDirectoryModel(data::SyncConnection &connection, QObject *parent = nullptr);
  
  public Q_SLOTS:
  QHash<int, QByteArray> roleNames() const;
  const QVector<int> &colorRoles() const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &child) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  const data::SyncthingDirectory *dirInfo(const QModelIndex &index) const;
  
  private Q_SLOTS:
  void newConfig();
  void newDirs();
  void dirStatusChanged(const data::SyncthingDirectory&, int index);
  
private:
  static QHash<int, QByteArray> initRoleNames();
  static QString dirStatusString(const data::SyncthingDirectory &dir);
  QVariant dirStatusColor(const data::SyncthingDirectory &dir) const;
  
  const std::vector<data::SyncthingDirectory> &mDirectories;
};

} // namespace model
} // namespace qst

Q_DECLARE_METATYPE(QModelIndex)

#endif // _DIRECTORYMODEL_H
