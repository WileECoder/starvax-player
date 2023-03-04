#include "MediaEngineMdk.h"

#include <QFileInfo>
#include <QPixmap>
#include <QMap>
#include <QAudioOutput>
#include <QVideoWidget>

#include "Fader.h"
#include "testableAssert.h"
#include "StatusDisplay.h"
#include "FullScreenMediaWidget_IF.h"
#include "supported_files.h"

// TODO remove all 3
#include <qdebug.h>
#include <QTime>
#include <QElapsedTimer>


#include <string>

//#define  DEBUG_VIDEO

#define PLAYER_STATE_TIMEOUT_ms   4000
#define SAFETY_DELAY_ms  1000

namespace {
NullMediaSource NullObject;


const qint32 DEFAULT_TICK_ms = 100;
const int DEFAULT_STEP_ms = 1000;

// TBD: can we detect the file format not using file extention?
bool checkForImageFormat( const QString & extention)
{
   return PICTURE_SUPPORTED_FORMATS.contains(extention.toLower());
}

}

/**
 * setup all media objects connections and make internal connections
 */
MediaEngineMdk::MediaEngineMdk( Fader & fader,
                                FullScreenMediaWidget_IF &displayWidget,
                                StatusDisplay & logger,
                                QObject *parent) :
   IF_MediaEngineInterface( parent),
   m_fader( fader),
   m_logger( logger),
   m_displayWidget( displayWidget),
   m_videoTrackAvailable(false),
   m_audioOnlyRequest( false),
   m_tickMs( DEFAULT_TICK_ms),
   m_stepSizeMs( DEFAULT_STEP_ms),
   m_fadeInFlag( false),
   m_imageFileFlag( false),
   m_pixmap( nullptr),
   m_requestedState( MediaObject::StoppedState)
{
   qDebug() << "err: " << m_player.error();
   QVideoWidget * video_widget = new QVideoWidget( nullptr);  // TODO: restore attach widget
   video_widget->setGeometry( 10,10, 800, 500);
   m_player.setVideoOutput( video_widget);
   video_widget->show();

   QAudioOutput * audioOut = new QAudioOutput( this);
   m_player.setAudioOutput( audioOut);
   audioOut->setVolume( 0.7);
}

MediaEngineMdk::~MediaEngineMdk()
{

}


void MediaEngineMdk::checkPlatform()
{
#if defined(Q_OS_WIN)
#elif defined(Q_OS_UNIX)
#endif
}


void MediaEngineMdk::deletePixmap()
{
   m_displayWidget.setPixmap( QPixmap());

   if (m_pixmap)
   {
      delete m_pixmap;
      m_pixmap = nullptr;
   }
}

void MediaEngineMdk::setCurrentSource( const AbstractMediaSource *source, bool dontStopFlag)
{
   if (source != nullptr)
   {
      const QString & filePath = source->fileName();
      qDebug() << "setting media: " << filePath;

      if (filePath != m_currentMediaPath)
      {
         if (dontStopFlag == false)  /* default case */
         {
            stop();
         }

         // TODO ...
         m_player.setSource( QUrl::fromLocalFile(filePath));
      }
   }
}


void MediaEngineMdk::play()
{
   // TODO ...
   m_player.play();

   evaluateDisplayShow();
}

void MediaEngineMdk::pause()
{
   qDebug() << "pause requested";
   m_requestedState = MediaObject::PausedState;

   if (m_imageFileFlag)
   {
      emit pictureShowChanged(false);
   }
   else
   {
      // TODO ...
      m_player.pause();
   }

   evaluateDisplayShow();
}

void MediaEngineMdk::togglePlayPause()
{
   if (m_imageFileFlag)
   {
      bool nowVisible = m_displayWidget.togglePictureVisibility();
      emit pictureShowChanged(nowVisible);
   }
   else
   {
      // TODO ...
   }
}


void MediaEngineMdk::stop()
{
   qDebug() << "stop requested";
   m_requestedState = MediaObject::StoppedState;
   int_stop();
}


void MediaEngineMdk::int_stop()
{
   // TODO ...
   m_player.stop();

   evaluateDisplayShow();

   /* used by UI controls, not by screen */
   emit pictureShowChanged(false);
   emit tick(0);
}

void MediaEngineMdk::evaluateDisplayShow()
{
#ifdef DEBUG_VIDEO
   QString action;
#endif

   if (m_imageFileFlag)
   {
      m_displayWidget.hideVideo();

      if ((m_requestedState == MediaObject::PlayingState) &&
          (m_pixmap != nullptr) )
      {
#ifdef DEBUG_VIDEO
         action = "show picture";
#endif
         m_displayWidget.showPicture();
      }
      else
      {
#ifdef DEBUG_VIDEO
         action = "hide picture";
#endif
         m_displayWidget.hidePicture();
      }
   }
   else // audio-video
   {
      m_displayWidget.hidePicture();

      if ((m_audioOnlyRequest == true) ||
          (m_videoTrackAvailable == false) ||
          (m_requestedState == MediaObject::StoppedState) )
      {
#ifdef DEBUG_VIDEO
         action = "hide video";
#endif
         m_displayWidget.hideVideo();
      }
      else
      {
#ifdef DEBUG_VIDEO
         action = "show video";
#endif
         m_displayWidget.showVideo();
      }
   }

#ifdef DEBUG_VIDEO
   qDebug() << (uint64_t)this
            << ":" << action << "  "
            << "  image:"<< m_imageFileFlag
            << "  audio_only:" << m_audioOnlyRequest
            << "  video_available:" << m_videoTrackAvailable
            << "  m_requested:"  <<  m_requestedState
            << "  pixmap_null" << (m_pixmap == nullptr);
#endif
}


/** bring track back at beginning */
void MediaEngineMdk::rewind()
{
   // TODO ...
   emit tick(0);
}

void MediaEngineMdk::setStepSizeMs(int stepMs)
{
   m_stepSizeMs = stepMs;
}

void MediaEngineMdk::stepForward()
{
   // TODO ...
}

void MediaEngineMdk::stepBackward()
{
   // TODO ...
}


void MediaEngineMdk::singleFrameForward()
{
   // TODO ...
}

void MediaEngineMdk::singleFrameBackward()
{
   // TODO ...
}

void MediaEngineMdk::onUserPositionRequested( qint64 positionMs)
{
   // TODO ...

   /* fix GUI in case position is sought before playback */
   emit tick( positionMs);
}

void MediaEngineMdk::setMuted(bool isMuted)
{
   // TODO ...
   emit muteStatus( isMuted);
}

void MediaEngineMdk::setAudioOnly( bool audioOnly)
{
   m_audioOnlyRequest = audioOnly;
   emit audioOnlyChanged( audioOnly);

   evaluateDisplayShow();
}

void MediaEngineMdk::showOnTop(bool onTop)
{
   m_displayWidget.setOnTop( onTop);
   emit onTopChanged( onTop);
}


void MediaEngineMdk::enableFadeIn(bool enabled)
{
   m_fadeInFlag = enabled;
}

void MediaEngineMdk::setLoopPlayback(bool /*enabled*/)
{
   // TODO ...
}

void MediaEngineMdk::setVolume(int vol)
{
   // TODO ...

   // valid also if 'vol' is zero
   emit muteStatus( false);

   // update GUIs
   emit volumeChanged( vol);
}

int MediaEngineMdk::volume()
{
   // TODO ...
   return 0;
}


void MediaEngineMdk::setTickInterval( qint32 tick_ms )
{
   m_tickMs = tick_ms;
}


void MediaEngineMdk::enableSubtitles()
{
   // TODO
#if 0
   static const QStringList subtitleExtention = QStringList() << "srt" << "ass" << "ssa" << "sub" << "lrc";
   QString sub_path = m_currentMediaPath + ".ass";

   m_player.setMedia(sub_path.toLatin1().constData(), mdk::MediaType::Subtitle);
   m_player.setActiveTracks( mdk::MediaType::Subtitle, {0});
#endif
}


void MediaEngineMdk::disableSubtitles()
{
#if 0
   m_player.setMedia( nullptr, mdk::MediaType::Subtitle);
#endif
}


void MediaEngineMdk::onTimerTick()
{
   // TODO ...

}



void MediaEngineMdk::onDurationChanged(int64_t duration_ms)
{
   emit totalTimeChanged( duration_ms);
}

void MediaEngineMdk::onVideoAvailable(bool available)
{
   m_videoTrackAvailable = available;
   evaluateDisplayShow();
}


void MediaEngineMdk::onAudioOnlyRequest()
{
   emit audioOnlyChanged( true);
}

void MediaEngineMdk::onMediaError()
{
   qDebug() << "MEDIA ERROR";
   m_logger.showMessage(tr("Media Error"), StatusDisplay::WARNING);
   emit AvMediaStateChanged( MediaObject::ErrorState);
}



