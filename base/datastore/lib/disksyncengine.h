#ifndef DISKSYNCENGINE_H
#define DISKSYNCENGINE_H

#include <syncengineinterface.h>
#include <QuetzalDataKit_export.h>

namespace cherry {

class QuetzalDataKit_EXPORT disk_engine : public sync_engine_interface {
  Q_OBJECT
public:
  explicit disk_engine(QObject *a_parent_ptr = 0);
  virtual ~disk_engine();

  virtual void setEngineName(const QString &name);

  void set_app_name(const std::string &a_app_name);

  void insert_request(const sync_object &a_obj);
  void update_request(const sync_object &a_obj);
  void delete_request(const std::string &a_object_name,
                      const std::string &a_key, const std::string &a_value);

  virtual QString data(const QString &fileName);

  virtual void find(const std::string &a_object_name,
                    const std::string &a_attrib, const std::string &a_value);
  virtual void sync(const QString &datqstoreName, const QString &data);

  virtual bool hasLock();

  void saveDataToDisk(const QString &fileName, const QString &data);

  QString db_home_path();
  QString db_app_path();
private
Q_SLOTS:
  void onBytesWritten(qint64 bytes);
  void onDirectoryChanged(const QString &name);

private:
  class Privatedisk_engine;
  Privatedisk_engine *const d;
};
}
#endif // DISKSYNCENGINE_H
