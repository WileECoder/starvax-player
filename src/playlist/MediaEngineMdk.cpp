#include "MediaEngineMdk.h"

#include <QFileInfo>
#include <QPixmap>
#include <QMap>

#include "Fader.h"
#include "testableAssert.h"
#include "StatusDisplay.h"
#include "FullScreenMediaWidget_IF.h"
#include "supported_files.h"

#include <qdebug.h>
#include <string>

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
   connect( this, & MediaEngineMdk::int_mediaStatusChanged, this, & MediaEngineMdk::onMediaStatusChanged);
   connect( this, & MediaEngineMdk::int_playerStateChanged, this, & MediaEngineMdk::onPlayerStateChanged);
   connect( this, & MediaEngineMdk::int_videoAvailableChanged, this, & MediaEngineMdk::onVideoAvailable);

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
      emit int_playerStateChanged( s);
   });

   m_player.onMediaStatusChanged([this](mdk::MediaStatus s){
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


void MediaEngineMdk::setCurrentSource( const AbstractMediaSource *source, bool dontStopFlag)
{
   if (dontStopFlag == false)  /* default case */
   {
      int_stop();
      disableSubtitles();
      m_videoTrackAvailable = false;
   }

   if (source != nullptr)
   {
      const QString & filePath = source->fileName();

      if (filePath != m_currentMediaPath)
      {
         m_imageFileFlag = checkForImageFormat( QFileInfo(filePath).suffix());

         if (m_imageFileFlag)
         {
            if (m_pixmap)
            {
               delete m_pixmap;
               m_pixmap = nullptr;
            }

            m_pixmap = new QPixmap( filePath);
            m_displayWidget.setPixmap( *m_pixmap);
         }
         else
         {
            m_player.setMedia( filePath.toUtf8().constData());

            /* buffer data immediately */
            m_player.prepare( 0, [this](int64_t /*time*/, bool*) {
               /* read media length and video availability and propagate these info */
               const mdk::MediaInfo & info = m_player.mediaInfo();

               onDurationChanged( info.duration);

               /* MDK bug: 'mediaInfo' can not be called at any time. Sometimes results are wrong. */
               m_videoTrackAvailable = (info.video.size() > 0);
               emit int_videoAvailableChanged( m_videoTrackAvailable);
               return true;
            });
         }

         m_currentMediaPath = filePath;
         emit currentMediaChanged( *source);
         emit tick(0);
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
   m_requestedState = MediaObject::PausedState;

   if (m_imageFileFlag)
   {
      T_ASSERT( m_pixmap);
      m_displayWidget.setPixmap( *m_pixmap);
      m_displayWidget.showPicture();
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

      if (m_videoTrackAvailable && (m_audioOnlyRequest == false))
      {
         m_displayWidget.showVideo();
      }
   }
}

void MediaEngineMdk::pause()
{
   m_requestedState = MediaObject::PausedState;

   if (m_imageFileFlag)
   {
      m_displayWidget.hidePicture();
      emit pictureShowChanged(false);
   }
   else
   {
      m_player.set( mdk::State::Paused);
   }
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
   m_requestedState = MediaObject::StoppedState;
   int_stop();
}


void MediaEngineMdk::int_stop()
{
   m_displayWidget.hideAll();
   m_player.set(mdk::State::Stopped);
   emit pictureShowChanged(false);
   emit tick(0);
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

   if ((m_audioOnlyRequest == true) || (m_videoTrackAvailable == false))
   {
      m_displayWidget.hideVideo();
   }
   else
   {
      m_displayWidget.showVideo();
   }
}

void MediaEngineMdk::showOnTop(bool onTop)
{
   m_displayWidget.setOnTop( onTop);
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

   static const QStringList subtitleExtention = QStringList() << "srt" << "ass" << "ssa" << "sub" << "lrc";
   QString sub_path = m_currentMediaPath + ".ass";

   m_player.setMedia(sub_path.toLatin1().constData(), mdk::MediaType::Subtitle);
   m_player.setActiveTracks( mdk::MediaType::Subtitle, {0});
}


void MediaEngineMdk::disableSubtitles()
{
   m_player.setMedia( nullptr, mdk::MediaType::Subtitle);
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
}


void MediaEngineMdk::onDurationChanged(int64_t duration_ms)
{
   emit totalTimeChanged( duration_ms);
}

void MediaEngineMdk::onVideoAvailable(bool available)
{
   m_videoTrackAvailable = available;

   /* It may happen that 'play' is issued just after 'setCurrentSource',
    * before media is loaded. In this case, video widget must be shown now.
    * Do not use "m_player.state()" due to the playing/pause glitch that happens
    * when loading a file */
   if (m_videoTrackAvailable &&
       (m_requestedState == MediaObject::PlayingState) &&
       (m_audioOnlyRequest == false))
   {
      m_displayWidget.showVideo();
   }
}


void MediaEngineMdk::onAudioOnlyRequest()
{
   emit audioOnlyChanged( true);
}


