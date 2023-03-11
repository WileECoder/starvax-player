#include "MediaEngineMdk.h"

#include <QFileInfo>
#include <QPixmap>
#include <QMap>

#include "Fader.h"
#include "testableAssert.h"
#include "StatusDisplay.h"
#include "FullScreenMediaWidget_IF.h"
#include "supported_files.h"

// TODO remove all 3
#include <qdebug.h>
#include <QTime>
#include <QElapsedTimer>
#include <QThread>

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
   qRegisterMetaType<mdk::MediaStatus>("mdk::MediaStatus");
   qRegisterMetaType<mdk::State>("mdk::State");
   qRegisterMetaType<int64_t>("int64_t");
   qRegisterMetaType<MediaObject::AvPlayerState>("MediaObject::AvPlayerState");

   connect( & m_tickTimer, & QTimer::timeout, this, & MediaEngineMdk::onTimerTick);
   connect( & m_fader, & Fader::changeVolume, this, & MediaEngineMdk::setVolume);
   connect( this, & MediaEngineMdk::int_mediaStatusChanged, this, & MediaEngineMdk::onMediaStatusChanged, Qt::QueuedConnection);
   connect( this, & MediaEngineMdk::int_playerStateChanged, this, & MediaEngineMdk::onPlayerStateChanged, Qt::QueuedConnection);
   connect( this, & MediaEngineMdk::int_videoAvailableChanged, this, & MediaEngineMdk::onVideoAvailable, Qt::QueuedConnection);
   connect( this, & MediaEngineMdk::int_mediaError, this, & MediaEngineMdk::onMediaError, Qt::QueuedConnection);

   m_player.setDecoders(mdk::MediaType::Video, {
                        #if (_WIN32+0)
                           "MFT:d3d=11",
                           "CUDA",
                           "D3D11",
                           "DXVA",
                        #elif (__linux__+0)
                           "VAAPI",
                           "VDPAU",
                           "CUDA",
                           "MMAL",
                        #endif
                           "FFmpeg"});

   m_player.onStateChanged([this](mdk::State s){
      qDebug() << "player: " << (int8_t)s;
      emit int_playerStateChanged( s);
   });

   m_player.onMediaStatusChanged([this](mdk::MediaStatus s){
      qDebug() << "media: " << Qt::hex << (int)s;
      emit int_mediaStatusChanged( s);
      return true;
   });


   m_displayWidget.attachPlayer( m_player);
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

         m_imageFileFlag = checkForImageFormat( QFileInfo(filePath).suffix());
         deletePixmap();

         if (m_imageFileFlag)
         {
            m_pixmap = new QPixmap( filePath);
            m_displayWidget.setPixmap( *m_pixmap);
         }
         else
         {
            QElapsedTimer timer;
            timer.start();
            emit AvMediaStateChanged( MediaObject::LoadingState);

            m_player.setMedia( filePath.toUtf8().constData());
            emit tick(0);

            /* buffer data immediately */
            m_player.prepare( 0, [this, timer](int64_t /*time*/, bool*) {
               /* read media length and video availability and propagate these info */
               const mdk::MediaInfo & info = m_player.mediaInfo();

               qDebug() << "loaded in " << timer.elapsed() << " ms.";

               if (m_player.mediaStatus() & (mdk::MediaStatus::Invalid))
               {
                  emit int_mediaError();
                  return false;
               }
               else
               {
                  onDurationChanged( info.duration);

                  /* MDK bug: 'mediaInfo' can not be called at any time. Sometimes results are wrong. */
                  m_videoTrackAvailable = (info.video.size() > 0);
                  emit int_videoAvailableChanged( m_videoTrackAvailable);
                  return true;
               }
            });
         }

         QThread::msleep( SAFETY_DELAY_ms);

//         QElapsedTimer timer;
//         timer.start();
//         bool ok = m_player.waitFor( mdk::State::Paused, PLAYER_STATE_TIMEOUT_ms);
//         qDebug() << "load complete: " << ok << " in " << timer.elapsed();

         m_currentMediaPath = filePath;
         emit currentMediaChanged( *source);
      }
   }
   else
   {
      /* no active source */
      emit currentMediaChanged( NullObject);

      m_imageFileFlag = false;
   }
}


void MediaEngineMdk::play()
{
   qDebug() << "play requested. status: 0x" << Qt::hex << m_player.mediaStatus();
   m_requestedState = MediaObject::PlayingState;

   if (m_imageFileFlag)
   {
      emit pictureShowChanged(true);
   }
   else
   {
      if (m_fadeInFlag)
      {
         m_fader.fadeInTo( volume());
         m_fadeInFlag = false;
      }

      m_player.set( mdk::State::Playing);
      QThread::msleep( SAFETY_DELAY_ms);
   }

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
      m_player.set( mdk::State::Paused);
      QThread::msleep( SAFETY_DELAY_ms);
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
      if (m_player.state() == mdk::State::Playing)
      {
         pause();
      }
      else
      {
         play();
      }
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
   m_player.set(mdk::State::Stopped);
   QThread::msleep( SAFETY_DELAY_ms);

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
   m_player.seek(0);
   emit tick(0);
}

void MediaEngineMdk::setStepSizeMs(int stepMs)
{
   m_stepSizeMs = stepMs;
}

void MediaEngineMdk::stepForward()
{
   m_player.seek( m_player.position() + m_stepSizeMs);
}

void MediaEngineMdk::stepBackward()
{
   m_player.seek( m_player.position() - m_stepSizeMs);
}


void MediaEngineMdk::singleFrameForward()
{
   m_player.seek( 1, mdk::SeekFlag::FromNow | mdk::SeekFlag::Frame );
   emit tick( m_player.position());
}

void MediaEngineMdk::singleFrameBackward()
{
   m_player.seek( -1, mdk::SeekFlag::FromNow | mdk::SeekFlag::Frame );
   emit tick( m_player.position());
}

void MediaEngineMdk::onUserPositionRequested( qint64 positionMs)
{
   if (m_player.state() == mdk::State::Stopped)
   {
      m_player.prepare( positionMs);
   }
   else
   {
      m_player.seek( positionMs);
   }

   /* fix GUI in case position is sought before playback */
   emit tick( positionMs);
}

void MediaEngineMdk::setMuted(bool isMuted)
{
   m_player.setMute( isMuted);
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

void MediaEngineMdk::setLoopPlayback(bool enabled)
{
   m_player.setLoop( enabled ? 0x1FFFFFFF : 0);
}

void MediaEngineMdk::setVolume(int vol)
{
   m_player.setVolume( (float)vol / 100.f);

   // valid also if 'vol' is zero
   emit muteStatus( false);

   // update GUIs
   emit volumeChanged( vol);
}

int MediaEngineMdk::volume()
{
   return (int)(m_player.volume() * 100.f);
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
   int64_t pos = m_player.position();
   emit tick(pos);
}


void MediaEngineMdk::onMediaStatusChanged(mdk::MediaStatus newStatus)
{
   if (newStatus & (mdk::Loading))
   {
      emit AvMediaStateChanged( MediaObject::LoadingState);
   }
   else if (newStatus & (mdk::Buffering))
   {
      emit AvMediaStateChanged( MediaObject::BufferingState);
   }
   else if (newStatus & (mdk::Loaded | mdk::Buffered))
   {
      emit AvMediaStateChanged( MediaObject::LoadedState);
   }
   else if (newStatus & (mdk::Invalid))
   {
      emit AvMediaStateChanged( MediaObject::ErrorState);
   }
}


void MediaEngineMdk::onPlayerStateChanged(mdk::State newState)
{
   static const QMap< mdk::State, MediaObject::AvPlayerState> PLAYER_TABLE =
   {
      { mdk::State::NotRunning, MediaObject::StoppedState},
      { mdk::State::Stopped, MediaObject::StoppedState},
      { mdk::State::Running, MediaObject::PlayingState},
      { mdk::State::Playing, MediaObject::PlayingState},
      { mdk::State::Paused, MediaObject::PausedState},
   };

   MediaObject::AvPlayerState playerState = PLAYER_TABLE.value( newState, MediaObject::StoppedState);
   emit AvPlayerStateChanged( playerState);

   switch (playerState)
   {
   case  MediaObject::PlayingState :
      m_tickTimer.start( m_tickMs);
      break;
   case MediaObject::PausedState :
      m_tickTimer.stop();
      break;
   case MediaObject::StoppedState :
      m_tickTimer.stop();

      if (m_requestedState != MediaObject::StoppedState)
      {
         /* guess: stop for end-of-file reached. */
         emit finished();
      }

      break;
   }

   evaluateDisplayShow();
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



