
#include <config.h>

#include "notewidget.h"
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <QGraphicsLinearLayout>
#include <QDateTime>
#include <QGraphicsWidget>
#include <asyncdatadownloader.h>
#include <asyncimagecreator.h>
#include <button.h>
#include <imageview.h>
#include <texteditor.h>
#include <label.h>
#include <themepackloader.h>
#include <webservice.h>
#include <datastore.h>
#include <memorysyncengine.h>
#include <disksyncengine.h>
#include <syncobject.h>
#include <extensionmanager.h>
#include <imagebutton.h>

#include <plexyconfig.h>
#include <toolbar.h>

class NoteWidget::PrivateNoteWidget
{
public:
  PrivateNoteWidget() {}
  ~PrivateNoteWidget() {}

  void initDataStore();

  QString getContentText(const QString &data) const;

  UIKit::Style *mStyle;
  QString mNoteTitle;
  QString mStatusMessage;
  QPixmap mPixmap;
  QString mID;
  QPixmap mAvatar;
  UIKit::Button *mButton;
  UIKit::TextEditor *mTextEdit;
  UIKit::Label *mLable;
  UIKit::ImageButton *mCloseButton;
  QGraphicsWidget *mLayoutBase;
  QGraphicsWidget *mSubLayoutBase;
  QGraphicsLinearLayout *mMainVerticleLayout;
  QGraphicsLinearLayout *mSubLayout;
  QImage mBackgroundPixmap;
  // datastore
  QuetzalKit::DataStore *mDataStore;       // main data store.
  QuetzalKit::SyncObject *mNoteListObject; // contains the list of Notes.
  QuetzalKit::SyncObject *mCurrentNoteObject;
  QuetzalKit::SyncObject *getNoteObject();
  UIKit::ToolBar *mToolBar;
  UIKit::Space *m_viewport;
};

void NoteWidget::createToolBar()
{
  d->mToolBar = new UIKit::ToolBar(d->mSubLayoutBase);
  d->mToolBar->add_action("contact", "pd_add_contact_icon", false);
  d->mToolBar->add_action("list", "pd_list_icon", false);
  d->mToolBar->add_action("date", "pd_calendar_icon", false);
  d->mToolBar->add_action("red", "pd_icon_red_color_action", false);
  d->mToolBar->add_action("yellow", "pd_icon_yellow_color_action", false);
  d->mToolBar->add_action("green", "pd_icon_green_color_action", false);
  d->mToolBar->add_action("blue", "pd_icon_blue_color_action", false);
  d->mToolBar->add_action("black", "pd_icon_black_color_action", false);
  d->mToolBar->add_action("delete", "pd_trash_icon", false);
}

void NoteWidget::setViewport(UIKit::Space *space)
{
  d->m_viewport = space;
}

NoteWidget::NoteWidget(QGraphicsObject *parent)
  : UIKit::Widget(parent), d(new PrivateNoteWidget)
{
  d->mLayoutBase = new QGraphicsWidget(this);
  d->mSubLayoutBase = new QGraphicsWidget(d->mLayoutBase);

  d->mMainVerticleLayout = new QGraphicsLinearLayout(d->mLayoutBase);
  d->mMainVerticleLayout->setOrientation(Qt::Horizontal);
  d->mMainVerticleLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);

  d->mSubLayout = new QGraphicsLinearLayout(d->mSubLayoutBase);
  d->mSubLayout->setOrientation(Qt::Vertical);
  d->mSubLayout->setContentsMargins(5.0, 5.0, 5.0, 5.0);

  d->mSubLayout->setSpacing(0);

  createToolBar();

  connect(d->mToolBar, SIGNAL(action(QString)), this,
          SLOT(onToolBarAction(QString)));

  d->mTextEdit = new UIKit::TextEditor(d->mSubLayoutBase);
  d->mTextEdit->style(
    "border: 0; background: rgba(255,255,255,255); color: #4E4945");

  d->mSubLayout->addItem(d->mTextEdit);
  d->mSubLayout->addItem(d->mToolBar);

  d->mTextEdit->set_placeholder_text("Title :");
  d->mMainVerticleLayout->addItem(d->mSubLayoutBase);

  d->mCloseButton = new UIKit::ImageButton(this);
  d->mCloseButton->set_pixmap(
    UIKit::Theme::instance()->drawable("pd_trash_icon.png", "mdpi"));
  d->mCloseButton->set_size(QSize(16, 16));
  d->mCloseButton->hide();
  d->mCloseButton->set_background_color(Qt::white);

  connect(d->mTextEdit, SIGNAL(documentTitleAvailable(QString)), this,
          SLOT(onDocuemntTitleAvailable(QString)));
  connect(d->mTextEdit, SIGNAL(textUpdated(QString)), this,
          SLOT(onTextUpdated(QString)));
  connect(d->mCloseButton, SIGNAL(clicked()), this,
          SLOT(deleteImageAttachment()));

  this->setAcceptDrops(true);
  initDataStore();
}

NoteWidget::~NoteWidget() { delete d; }

void NoteWidget::setTitle(const QString &name)
{
  d->mNoteTitle = name;
  update();

  d->mCurrentNoteObject->setObjectAttribute("title", name);
  d->mDataStore->updateNode(d->mCurrentNoteObject);

  // requestNoteSideImageFromWebService(d->mNoteTitle);
}

void NoteWidget::initDataStore()
{
  d->mDataStore = new QuetzalKit::DataStore("desktopnotes", this);
  QuetzalKit::DiskSyncEngine *engine =
    new QuetzalKit::DiskSyncEngine(d->mDataStore);

  d->mDataStore->setSyncEngine(engine);

  d->mNoteListObject = d->mDataStore->begin("NoteList");
  d->mCurrentNoteObject = d->mNoteListObject->createNewObject("Note");
  d->mCurrentNoteObject->setObjectAttribute("title", "");

  d->mDataStore->insert(d->mCurrentNoteObject);
}

void NoteWidget::setNoteWidgetContent(const QString &status)
{
  d->mStatusMessage = status;
  update();
}

void NoteWidget::setID(const QString &id) { d->mID = id; }

QString NoteWidget::title() const { return d->mNoteTitle; }

QString NoteWidget::id() { return d->mID; }

QString NoteWidget::noteContent() const { return d->mStatusMessage; }

void NoteWidget::setPixmap(const QPixmap &pixmap)
{
  this->prepareGeometryChange();
  this->setGeometry(QRectF(0.0, 0.0, this->boundingRect().width(), 600));

  if (d->mPixmap.isNull()) {
    d->mTextEdit->setPos(d->mTextEdit->pos().x(),
                         d->mTextEdit->pos().y() + 300);
    d->mToolBar->setPos(d->mToolBar->pos().x(), d->mToolBar->pos().y() + 300);
    qDebug() << Q_FUNC_INFO << pixmap.isNull();
  }
  d->mPixmap = pixmap;
  d->mCloseButton->show();
  update();
}

void NoteWidget::saveNoteToStore()
{
  QuetzalKit::DataStore *store = new QuetzalKit::DataStore("deskopnotes", this);

  QuetzalKit::DiskSyncEngine *diskEngine = new QuetzalKit::DiskSyncEngine(this);
  store->setSyncEngine(diskEngine);

  QuetzalKit::SyncObject *object = store->rootObject();

  bool newObject = false;
  if (object->name().isEmpty()) {
    object->setName("notelist");
    object->setKey(10);
    newObject = true;
    // object->setObjectAttribute("src", "http://www.google.com");
  }

  // child object
  QuetzalKit::SyncObject noteObject;

  noteObject.setName("note");
  noteObject.setObjectAttribute("content", d->mTextEdit->text());
  // meta object

  QuetzalKit::SyncObject metaData;

  metaData.setName("metadata");
  metaData.setKey(100);
  metaData.setObjectAttribute("compression", "PNG");
  metaData.setObjectAttribute("location", "Colomobo, Sri Lanka");

  noteObject.addChildObject(&metaData);
  if (!newObject) {
    object->addChildObject(&noteObject);
  }

  store->addObject(object);
}

void NoteWidget::resize(const QSizeF &size)
{
  setGeometry(QRectF(0, 0, size.width(), size.height()));

  d->mLayoutBase->setGeometry(geometry());
  //d->mLayoutBase->setPos(0.0, 64.0);

  d->mSubLayoutBase->setGeometry(boundingRect());
  d->mTextEdit->setMaximumSize(geometry().size());
  d->mMainVerticleLayout->setGeometry(
    QRectF(0.0, 0.0, size.width(), size.height()));
  /*
  d->mSubLayout->setGeometry(QRectF(0, 0,
                                    boundingRect().width(),
                                    boundingRect().height()));
                                    */

  d->mCloseButton->setPos(boundingRect().width() - 20, 72.0);

  d->mMainVerticleLayout->updateGeometry();

  d->mMainVerticleLayout->invalidate();

  d->mSubLayout->invalidate();
  d->mSubLayout->updateGeometry();
  d->mSubLayout->activate();
}

void NoteWidget::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
  UIKit::Widget::paint(painter, option, widget);
  painter->save();
  painter->setRenderHint(QPainter::SmoothPixmapTransform);
  painter->drawPixmap(
    QRectF(0, 0, option->exposedRect.width(), 300.0), d->mPixmap,
    QRectF((d->mPixmap.width() - option->exposedRect.width()) / 2, 0.0,
           option->exposedRect.width(), 300));
  painter->restore();
}

void NoteWidget::dropEvent(QGraphicsSceneDragDropEvent *event)
{
  qDebug() << Q_FUNC_INFO << event->mimeData();
}

void NoteWidget::requestNoteSideImageFromWebService(const QString &key)
{
  QuetzalSocialKit::WebService *service =
    new QuetzalSocialKit::WebService(this);

  service->create("com.flickr.json.api");

  QVariantMap args;
  args["api_key"] = K_SOCIAL_KIT_FLICKR_API_KEY;
  args["text"] = key;
  args["per_page"] = QString::number(1);
  args["safe_search"] = "1";
  args["tags"] = "wallpaper,wallpapers,banners";
  args["tag_mode"] = "all";
  args["page"] = QString::number(1);

  service->queryService("flickr.photos.search", args);

  connect(service, SIGNAL(finished(QuetzalSocialKit::WebService *)), this,
          SLOT(onServiceCompleteJson(QuetzalSocialKit::WebService *)));
}

void NoteWidget::requestPhotoSizes(const QString &photoID)
{
  QuetzalSocialKit::WebService *service =
    new QuetzalSocialKit::WebService(this);

  service->create("com.flickr.json.api");

  QVariantMap args;
  args["api_key"] = K_SOCIAL_KIT_FLICKR_API_KEY;
  args["photo_id"] = photoID;

  service->queryService("flickr.photos.getSizes", args);

  connect(service, SIGNAL(finished(QuetzalSocialKit::WebService *)), this,
          SLOT(onSizeServiceCompleteJson(QuetzalSocialKit::WebService *)));
}

void NoteWidget::onClicked() { Q_EMIT clicked(this); }

void NoteWidget::onTextUpdated(const QString &text)
{
  QString save;
  if (d->mNoteTitle.isEmpty()) {
    save = text;
  } else {
    save = d->getContentText(text);
  }

  d->mCurrentNoteObject->setTextData(save);
  d->mDataStore->updateNode(d->mCurrentNoteObject);
}

void NoteWidget::onDocuemntTitleAvailable(const QString &title)
{
  this->setTitle(title);
}

void NoteWidget::onToolBarAction(const QString &action)
{
  qDebug() << Q_FUNC_INFO << action;
  if (action == tr("date")) {

    if (d->m_viewport) {
      d->m_viewport->create_activity("datepickeractivity", "Date/Time", QPointF(),
                                     QRectF(0, 0, 600, 440), QVariantMap());
    }

  } else if (action == tr("list")) {
    d->mTextEdit->begin_list();
  } else if (action == tr("link")) {
    d->mTextEdit->convert_to_link();
  } else if (action == tr("red")) {
    d->mTextEdit->style("border: 0; background: #D55521; color: #ffffff");
    d->mCurrentNoteObject->setObjectAttribute("color", "#D55521");
    d->mDataStore->updateNode(d->mCurrentNoteObject);
  } else if (action == tr("yellow")) {
    d->mTextEdit->style("border: 0; background: #E6DA42; color: #000000");
    d->mCurrentNoteObject->setObjectAttribute("color", "#E6DA42");
    d->mDataStore->updateNode(d->mCurrentNoteObject);
  } else if (action == tr("green")) {
    d->mTextEdit->style("border: 0; background: #29CDA8; color: #ffffff");
    d->mCurrentNoteObject->setObjectAttribute("color", "#29CDA8");
    d->mDataStore->updateNode(d->mCurrentNoteObject);
  } else if (action == tr("blue")) {
    d->mTextEdit->style("border: 0; background: #0AACF0; color: #ffffff");
    d->mCurrentNoteObject->setObjectAttribute("color", "#0AACF0");
    d->mDataStore->updateNode(d->mCurrentNoteObject);
  } else if (action == tr("black")) {
    d->mTextEdit->style("border: 0; background: #4A4A4A; color: #ffffff");
    d->mCurrentNoteObject->setObjectAttribute("color", "#4A4A4A");
    d->mDataStore->updateNode(d->mCurrentNoteObject);
  } else if (action == tr("delete")) {
    d->mDataStore->deleteObject(d->mCurrentNoteObject);
    this->hide();
  }
}

void NoteWidget::onServiceCompleteJson(QuetzalSocialKit::WebService *service)
{
  QList<QVariantMap> photoList = service->methodData("photo");

  Q_FOREACH(const QVariantMap & map, photoList) {
    requestPhotoSizes(map["id"].toString());
  }

  service->deleteLater();
  ;
}

void NoteWidget::onSizeServiceCompleteJson(
  QuetzalSocialKit::WebService *service)
{
  Q_FOREACH(const QVariantMap & map, service->methodData("size")) {
    if (map["label"].toString() == "Large" ||
        map["label"].toString() == "Large 1600" ||
        map["label"].toString() == "Original") {
      qDebug() << Q_FUNC_INFO << map["label"].toString() << "->"
               << map["source"].toString();
      QuetzalSocialKit::AsyncDataDownloader *downloader =
        new QuetzalSocialKit::AsyncDataDownloader(this);

      QVariantMap metaData;
      metaData["method"] = service->methodName();
      metaData["id"] =
        service->inputArgumentForMethod(service->methodName())["photo_id"];
      metaData["data"] = service->inputArgumentForMethod(service->methodName());

      downloader->setMetaData(metaData);
      downloader->setUrl(map["source"].toString());
      connect(downloader, SIGNAL(ready()), this, SLOT(onImageReady()));
    }
  }

  service->deleteLater();
}

void NoteWidget::onDownloadCompleteJson(QuetzalSocialKit::WebService *service) {}

void NoteWidget::onImageReady()
{
  QuetzalSocialKit::AsyncDataDownloader *downloader =
    qobject_cast<QuetzalSocialKit::AsyncDataDownloader *>(sender());

  if (downloader) {
    QuetzalSocialKit::AsyncImageCreator *imageSave =
      new QuetzalSocialKit::AsyncImageCreator(this);

    connect(imageSave, SIGNAL(ready()), this, SLOT(onImageSaveReadyJson()));

    imageSave->setMetaData(downloader->metaData());
    imageSave->setData(downloader->data(), UIKit::Config::cache_dir(), true);
    imageSave->setCrop(QRectF(0.0, 0.0, 300.0, 300.0));
    imageSave->start();

    downloader->deleteLater();
  }
}

void NoteWidget::onImageSaveReadyJson()
{
  qDebug() << Q_FUNC_INFO;
  QuetzalSocialKit::AsyncImageCreator *c =
    qobject_cast<QuetzalSocialKit::AsyncImageCreator *>(sender());

  if (c) {
    d->mBackgroundPixmap = c->image();

    if (d->mLayoutBase) {
      d->mLayoutBase->setPos(0.0, 300.0);
    }

    update();
    c->quit();
    c->deleteLater();
  }
}

void NoteWidget::onImageReadyJson(const QString &fileName)
{
  qDebug() << Q_FUNC_INFO << fileName;
}

void NoteWidget::deleteImageAttachment()
{
  d->mPixmap = QPixmap();

  this->prepareGeometryChange();
  this->setGeometry(QRectF(0.0, 0.0, this->boundingRect().width(), 300));

  if (d->mPixmap.isNull()) {
    d->mTextEdit->setPos(d->mTextEdit->pos().x(),
                         d->mTextEdit->pos().y() - 300);
    d->mToolBar->setPos(d->mToolBar->pos().x(), d->mToolBar->pos().y() - 300);
  }

  d->mCloseButton->hide();

  update();
}

QString NoteWidget::PrivateNoteWidget::getContentText(
  const QString &data) const
{
  QStringList dataList = data.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

  QString rv;
  for (int i = 1; i < dataList.count(); i++) {
    rv += dataList.at(i);
  }

  return rv;
}

QuetzalKit::SyncObject *NoteWidget::PrivateNoteWidget::getNoteObject()
{ return 0;}
