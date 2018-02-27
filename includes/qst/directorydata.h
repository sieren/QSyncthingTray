#ifndef _DIRECTORYDATA_H
#define _DIRECTORYDATA_H

#include <QFileInfo>
#include <QJsonObject>
#include <QMetaType>
#include <QString>

namespace qst
{
namespace data
{
  
  enum class SyncthingDirectoryStatus { Unknown, Idle, Unshared, Scanning, Synchronizing, OutOfSync };
  
  QString statusString(SyncthingDirectoryStatus status);
  
  struct SyncthingItemError {
    SyncthingItemError(const QString &message = QString(), const QString &path = QString())
    : message(message)
    , path(path)
    {
    }
    
    bool operator==(const SyncthingItemError &other) const
    {
      return message == other.message && path == other.path;
    }
    
    QString message;
    QString path;
  };
  
  struct SyncthingItemDownloadProgress {
    SyncthingItemDownloadProgress(
      const QString &containingDirPath = QString(), const QString &relativeItemPath = QString(), const QJsonObject &values = QJsonObject());
    QString relativePath;
    QFileInfo fileInfo;
    int blocksCurrentlyDownloading = 0;
    int blocksAlreadyDownloaded = 0;
    int totalNumberOfBlocks = 0;
    unsigned int downloadPercentage = 0;
    int blocksCopiedFromOrigin = 0;
    int blocksCopiedFromElsewhere = 0;
    int blocksReused = 0;
    int bytesAlreadyHandled;
    int totalNumberOfBytes = 0;
    QString label;
 //   ChronoUtilities::DateTime lastUpdate;
    static constexpr unsigned int syncthingBlockSize = 128 * 1024;
  };
  
  struct SyncthingCompletion {
 //   ChronoUtilities::DateTime lastUpdate;
    double percentage = 0;
    quint64 globalBytes = 0;
    quint64 neededBytes = 0;
    quint64 neededItems = 0;
    quint64 neededDeletes = 0;
  };
  
  struct SyncthingStatistics {
    quint64 bytes = 0;
    quint64 deletes = 0;
    quint64 dirs = 0;
    quint64 files = 0;
    quint64 symlinks = 0;
    
    constexpr bool isNull() const;
  };
  
  constexpr bool SyncthingStatistics::isNull() const
  {
    return bytes == 0 && deletes == 0 && dirs == 0 && files == 0 && symlinks == 0;
  }
  
  struct SyncthingDirectory {
    SyncthingDirectory(const QString &id = QString(), const QString &label = QString(), const QString &path = QString());
//    bool assignStatus(const QString &statusStr, ChronoUtilities::DateTime time);
//    bool assignStatus(SyncthingDirectoryStatus newStatus, ChronoUtilities::DateTime time);
    const QString &displayName() const;
    QString statusString() const;
    QStringRef pathWithoutTrailingSlash() const;
    bool isLocallyUpToDate() const;
    bool areRemotesUpToDate() const;
    
    QString id;
    QString label;
    QString path;
    QStringList deviceIds;
    QStringList deviceNames;
    bool readOnly = false;
    bool ignorePermissions = false;
    bool ignorePatterns = false;
    bool autoNormalize = false;
    int rescanInterval = 0;
    int minDiskFreePercentage = 0;
    SyncthingDirectoryStatus status = SyncthingDirectoryStatus::Idle;
  //  ChronoUtilities::DateTime lastStatusUpdate;
    int completionPercentage = 0;
    int scanningPercentage = 0;
    double scanningRate = 0;
    std::map<QString, SyncthingCompletion> completionByDevice;
    QString globalError;
    std::vector<SyncthingItemError> itemErrors;
    std::vector<SyncthingItemError> previousItemErrors;
    SyncthingStatistics globalStats, localStats, neededStats;
 //   ChronoUtilities::DateTime lastStatisticsUpdate;
 //   ChronoUtilities::DateTime lastScanTime;
//    ChronoUtilities::DateTime lastFileTime;
    QString lastFileName;
    std::vector<SyncthingItemDownloadProgress> downloadingItems;
    int blocksAlreadyDownloaded = 0;
    int blocksToBeDownloaded = 0;
    QString downloadLabel;
    unsigned int downloadPercentage = 0;
    bool paused = false;
    bool lastFileDeleted = false;
    
  private:
  //  bool checkWhetherStatusUpdateRelevant(ChronoUtilities::DateTime time);
    bool finalizeStatusUpdate(SyncthingDirectoryStatus newStatus);
  };
  
  inline SyncthingDirectory::SyncthingDirectory(const QString &id, const QString &label, const QString &path)
  : id(id)
  , label(label)
  , path(path)
  {
  }
  
  inline const QString &SyncthingDirectory::displayName() const
  {
    return label.isEmpty() ? id : label;
  }
  
  inline bool SyncthingDirectory::isLocallyUpToDate() const
  {
    return neededStats.isNull();
  }
  
//  inline bool SyncthingDirectory::assignStatus(SyncthingDirectoryStatus newStatus, ChronoUtilities::DateTime time)
//  {
//    return checkWhetherStatusUpdateRelevant(time) && finalizeStatusUpdate(newStatus);
//  }
  
} // namespace qst
} // namespace data

Q_DECLARE_METATYPE(qst::data::SyncthingItemError)
Q_DECLARE_METATYPE(qst::data::SyncthingItemDownloadProgress)
Q_DECLARE_METATYPE(qst::data::SyncthingDirectory)

#endif // _DIRECTORYDATA_H
