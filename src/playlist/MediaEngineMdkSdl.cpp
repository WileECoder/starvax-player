#include "MediaEngineMdkSdl.h"

#include <QFileInfo>

#include <QFileInfo>

#include "Fader.h"
#include "testableAssert.h"
#include "StatusDisplay.h"
#include "SdlEngine.h"
#include "FullScreenMediaWidget_IF.h"

// shortcut for data encoded as UTF8
//#define qtu( i ) ((i).toUtf8().constData())


namespace {
NullMediaSource NullObject;
QStringList subtitleExtention = QStringList() << "srt" << "ass" << "ssa" << "sub" << "lrc";
QStringList imageExtention = QStringList() << "png" << "bmp" << "jpg" << "jpeg" << "gif" << "tif" << "tiff";

const qint32 DEFAULT_TICK_ms = 100;
const qint32 DEFAULT_STEP_ms = 1000;

// TBD: can we detect the file format not using file extention?
bool checkForImageFormat( const QString & extention)
{
   return imageExtention.contains( extention, Qt::CaseInsensitive);
}

}

/**
 * setup all media objects connections and make internal connections
 */
MediaEngineMdkSdl::MediaEngineMdkSdl( SdlEngine & sdlEngine,
                                      Fader & fader,
                                      FullScreenMediaWidget_IF & displayWidget,
                                      StatusDisplay & logger,
                                      QObject *parent) :
   IF_MediaEngineInterface(parent),
   m_sdlEngine(sdlEngine),
   m_fader(fader),
   m_displayWidget( displayWidget),
   m_logger( logger),
   m_canvas(nullptr),
   m_tickMs(DEFAULT_TICK_ms),
   m_stepMs(DEFAULT_STEP_ms),
   m_fadeInFlag(false),
   m_imageFileFlag( false)
{
   m_playerId = m_sdlEngine.addPlayer();

   connect( & sdlEngine, & SdlEngine::mediaStatusChanged, this, & MediaEngineMdkSdl::onGlobalMediaStateChanged);
   connect( & sdlEngine, & SdlEngine::playerStateChanged, this, & MediaEngineMdkSdl::onGlobalPlayerStateChanged);
   connect( & sdlEngine, & SdlEngine::durationChanged, this, & MediaEngineMdkSdl::onGlobalDurationChanged);
   connect( & sdlEngine, & SdlEngine::videoAvailable, this, & MediaEngineMdkSdl::onGlobalVideoAvailable);
   connect( & m_tickTimer, & QTimer::timeout, this, & MediaEngineMdkSdl::onTimerTick);

   connect( & m_fader, & Fader::changeVolume, this, & MediaEngineMdkSdl::setVolume);
}

MediaEngineMdkSdl::~MediaEngineMdkSdl()
{

}

void MediaEngineMdkSdl::setWidgetForRender( QWidget * canvas)
{
   /* TODO useless ??? */
}


void MediaEngineMdkSdl::checkPlatform()
{
#if defined(Q_OS_WIN)
#elif defined(Q_OS_UNIX)
#endif
}


void MediaEngineMdkSdl::setCurrentSource( const AbstractMediaSource *source, bool dontStopFlag)
{
   m_sdlEngine.load( m_playerId, source->fileName());

   m_imageFileFlag = checkForImageFormat( QFileInfo(source->fileName()).suffix());
}


void MediaEngineMdkSdl::play()
{
   if (m_imageFileFlag)
   {
      m_displayWidget.showPicture();
   }
   else
   {
      if (m_fadeInFlag)
      {
         m_fader.fadeInTo( volume());
         m_fadeInFlag = false;
      }

      m_sdlEngine.play( m_playerId);
   }
}

void MediaEngineMdkSdl::pause()
{
   m_sdlEngine.pause( m_playerId);
}

void MediaEngineMdkSdl::togglePlayPause()
{
   m_sdlEngine.togglePlayPause( m_playerId);
}

void MediaEngineMdkSdl::stop()
{
   if (m_imageFileFlag)
   {
      m_displayWidget.showPicture();
   }
   else
   {
      m_sdlEngine.stop( m_playerId);
   }
}

/** bring track back at beginning */
void MediaEngineMdkSdl::rewind()
{
   m_sdlEngine.rewind( m_playerId);
}

void MediaEngineMdkSdl::stepForward()
{
   m_sdlEngine.seekRelative( m_playerId, m_stepMs);
}

void MediaEngineMdkSdl::stepBackward()
{
   m_sdlEngine.seekRelative( m_playerId, -m_stepMs);
}


void MediaEngineMdkSdl::singleFrameForward()
{
   m_sdlEngine.seekFrame( m_playerId, 1);
}

void MediaEngineMdkSdl::singleFrameBackward()
{
   m_sdlEngine.seekFrame( m_playerId, -1);
}

void MediaEngineMdkSdl::onUserPositionRequested( qint64 positionMs)
{
   m_sdlEngine.setPosition( m_playerId, positionMs);

   /* fix GUI in case position is seeked before playback */
   emit tick( positionMs);
}

void MediaEngineMdkSdl::setMuted(bool isMuted)
{
   m_sdlEngine.setMuted( m_playerId, isMuted);
   emit muteStatus( isMuted);
}

void MediaEngineMdkSdl::setAudioOnly( bool audioOnly)
{
   m_sdlEngine.setAudioOnly( m_playerId, audioOnly);
   emit audioOnlyChanged( audioOnly);
}

void MediaEngineMdkSdl::toggleAudioOnly()
{
   bool toAudioOnly = m_sdlEngine.toggleAudioOnly( m_playerId);
   emit audioOnlyChanged( toAudioOnly);
}

void MediaEngineMdkSdl::enableFadeIn(bool enabled)
{
   m_fadeInFlag = enabled;
}

void MediaEngineMdkSdl::setLoopPlayback(bool enabled)
{
   if (enabled)
   {
      m_sdlEngine.setLoopPlayback( m_playerId, enabled);
   }
}

void MediaEngineMdkSdl::setVolume(int vol)
{
   m_sdlEngine.setVolume( m_playerId, vol);

   // valid also if 'vol' is zero
   emit muteStatus( false);

   // update GUIs
   emit volumeChanged( vol);
}

int MediaEngineMdkSdl::volume()
{
   return m_sdlEngine.volume( m_playerId);
}



void MediaEngineMdkSdl::setTickInterval( qint32 tick_ms )
{
   m_tickMs = tick_ms;
}


void MediaEngineMdkSdl::enableSubtitles()
{

}


void MediaEngineMdkSdl::disableSubtitles()
{

}


void MediaEngineMdkSdl::onGlobalMediaStateChanged(int playerId, MediaObject::AvMediaState newState)
{
   if (playerId == m_playerId)
   {
      emit AvMediaStateChanged( newState);
   }
}

void MediaEngineMdkSdl::onGlobalPlayerStateChanged(int playerId, MediaObject::AvPlayerState newState)
{
   if (playerId == m_playerId)
   {
      emit AvPlayerStateChanged( newState);

      if (newState == MediaObject::PlayingState)
      {
         m_tickTimer.start( m_tickMs);
      }
      else
      {
         m_tickTimer.stop();
      }
   }
}

void MediaEngineMdkSdl::onGlobalDurationChanged(int playerId, int64_t duration_ms)
{
   if (playerId == m_playerId)
   {
      emit totalTimeChanged( duration_ms);
   }
}

void MediaEngineMdkSdl::onGlobalVideoAvailable(int playerId, bool available)
{
   if (playerId == m_playerId)
   {
      m_videoTrackAvailable = available;
   }
}

void MediaEngineMdkSdl::onTimerTick()
{
   int64_t pos = m_sdlEngine.currentPosition( m_playerId);
   emit tick(pos);
}



