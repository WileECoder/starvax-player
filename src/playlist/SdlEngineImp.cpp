#include "SdlEngineImp.h"
#include "SdlWorkerThread.h"
#include "testableAssert.h"

#include <QThread>
#include <QSemaphore>

SdlEngineImp::SdlEngineImp(QObject* parent) :
   SdlEngine(parent),
   m_worker( nullptr),
   m_thread( new QThread()),
   m_launched(false)
{
   qRegisterMetaType<mdk::MediaStatus>("mdk::MediaStatus");
   qRegisterMetaType<mdk::State>("mdk::State");
   qRegisterMetaType<int64_t>("int64_t");
}

void SdlEngineImp::launch()
{
   if (m_launched == false)
   {
      QSemaphore wait(0);

      m_launched = true;

      m_worker = new SdlWorkerThread( wait);
      m_worker->moveToThread( m_thread);
      connect( m_thread, &QThread::started, m_worker, &SdlWorkerThread::process);
      connect( m_thread, &QThread::finished, m_thread, &SdlWorkerThread::deleteLater);

      connect( m_worker, & SdlWorkerThread::mediaStatusChanged, this, & SdlEngineImp::onMediaStatusChanged);
      connect( m_worker, & SdlWorkerThread::playerStateChanged, this, & SdlEngineImp::onPlayerStateChanged);
      connect( m_worker, & SdlWorkerThread::durationChanged, this, & SdlEngine::durationChanged);
      connect( m_worker, & SdlWorkerThread::videoAvailable, this, & SdlEngine::videoAvailable);

      m_thread->start();

      /* wait for thread to be started and SDL initialized */
      wait.acquire();
   }
}

SdlEngine::PlayerId SdlEngineImp::addPlayer()
{
   T_ASSERT(m_worker);
   return m_worker->addPlayer();
}


void SdlEngineImp::load(SdlEngine::PlayerId id, const QString& full_path)
{
   m_worker->load( id, full_path);
}

void SdlEngineImp::play(SdlEngine::PlayerId id)
{
   m_worker->play( id);
}

void SdlEngineImp::pause(SdlEngine::PlayerId id)
{
   m_worker->pause( id);
}

void SdlEngineImp::stop(SdlEngine::PlayerId id)
{
   m_worker->stop( id);
}

void SdlEngineImp::togglePlayPause(PlayerId id)
{
   m_worker->togglePlayPause( id);
}

void SdlEngineImp::rewind(SdlEngine::PlayerId id)
{
   m_worker->rewind( id);
}

void SdlEngineImp::setLoopPlayback(PlayerId id, bool enable)
{
   m_worker->setLoopPlayback( id, enable);
}

int64_t SdlEngineImp::currentPosition(PlayerId id)
{
   return m_worker->currentPosition( id);
}

void SdlEngineImp::setPosition(PlayerId id, int64_t positionMs)
{
   m_worker->setPosition( id, positionMs);
}

void SdlEngineImp::seekRelative(PlayerId id, int64_t deltaMs)
{
   m_worker->seekRelative( id, deltaMs);
}

void SdlEngineImp::seekFrame(PlayerId id, int frameNumber)
{
   m_worker->seekFrame( id, frameNumber);
}

void SdlEngineImp::setVolume(PlayerId id, int vol)
{
   m_worker->setVolume( id, vol);
}

int SdlEngineImp::volume(PlayerId id)
{
   return m_worker->volume( id);
}

void SdlEngineImp::setMuted(PlayerId id, bool mute)
{
   m_worker->setMuted( id, mute);
}

void SdlEngineImp::setAudioOnly(PlayerId id, bool audioOnly)
{
   m_worker->setAudioOnly( id, audioOnly);
}

bool SdlEngineImp::toggleAudioOnly(PlayerId id)
{
   return m_worker->toggleAudioOnly( id);
}


void SdlEngineImp::onMediaStatusChanged(int id, mdk::MediaStatus newStatus)
{
   if (newStatus & (mdk::Loading))
   {
      emit mediaStatusChanged( id, MediaObject::LoadingState);
   }
   else if (newStatus & (mdk::Buffering))
   {
      emit mediaStatusChanged( id, MediaObject::BufferingState);
   }
   else if (newStatus & (mdk::Loaded | mdk::Buffered))
   {
      emit mediaStatusChanged( id, MediaObject::LoadedState);
   }
   else if (newStatus & (mdk::Invalid))
   {
      emit mediaStatusChanged( id, MediaObject::ErrorState);
   }
}


void SdlEngineImp::onPlayerStateChanged(int id, mdk::State newState)
{
   if (newState == mdk::State::Stopped)  // == mdk::State::NotRunning
   {
      emit playerStateChanged( id, MediaObject::StoppedState);
   }
   else if (newState == mdk::State::Playing)  // == mdk::State::NotRunning
   {
      emit playerStateChanged( id, MediaObject::PlayingState);
   }
   else  // mdk::State::Paused
   {
      emit playerStateChanged( id, MediaObject::PausedState);
   }
}

