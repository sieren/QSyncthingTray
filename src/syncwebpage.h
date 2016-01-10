#ifndef SYNCWEBPAGE_H
#define SYNCWEBPAGE_H

#include <QObject>
#include <QAuthenticator>
#include <QtWebEngineWidgets/QWebEnginePage>

using Authentication = std::pair<std::string, std::string>;

class SyncWebPage : public QWebEnginePage
{
  
public:
  SyncWebPage();
  SyncWebPage(QObject *parent) : QWebEnginePage(parent) { }
  void updateConnInfo(QUrl url, Authentication authInfo);

private slots:
  void requireAuthentication(const QUrl & requestUrl, QAuthenticator * authenticator);

protected:
  virtual bool certificateError(const QWebEngineCertificateError & certificateError) override;
  
private:
  Authentication mAuthInfo;

};

#endif // SYNCWEBPAGE_H
