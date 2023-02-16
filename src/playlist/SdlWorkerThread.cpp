#include "SdlWorkerThread.h"
#include "testableAssert.h"

#include <QMutexLocker>
#include <QDebug>

namespace {

/* support up to 16 players */
const int MAX_NUMBER_Of_PLAYERS = 16;


const Uint32 SDL_EVENT_UPDATE_FIRST = SDL_USEREVENT + 1;
const Uint32 SDL_EVENT_UPDATE_LAST = SDL_USEREVENT + MAX_NUMBER_Of_PLAYERS;

const Uint32 sdl_event_create_window = SDL_USEREVENT+20;

const Uint32 sdl_event_play = SDL_USEREVENT+22;
const Uint32 sdl_event_pause = SDL_USEREVENT+23;
const Uint32 sdl_event_stop = SDL_USEREVENT+24;
const Uint32 sdl_event_rewind = SDL_USEREVENT+25;


}  // namespace


SdlWorkerThread::SdlWorkerThread(QSemaphore& waitready, QObject *parent) :
   QObject(parent),
   m_reservedPlayersCount(0),
   m_context(nullptr),
   m_waitReady(waitready),
   m_numberOfDisplays(0xFFFF)
{
}

SdlWorkerThread::~SdlWorkerThread()
{
   qDeleteAll(m_players.begin(), m_players.end());
   m_players.clear();
}

int SdlWorkerThread::addPlayer()
{
   /* return the ID of a player that will soon be created */
   int playerId = m_reservedPlayersCount;

   /* create event for thread context */
   SDL_Event e;
   e.type = sdl_event_create_window;
   e.user.code = (Sint32)playerId;
   SDL_PushEvent( &e);

   m_reservedPlayersCount++;

   /* wait for player-set to be created, so that it can be used as
    * soon as this function returns */
   m_waitPlayerSetCreated.acquire();

   return playerId;
}

void SdlWorkerThread::load(int id, const QString& full_path)
{
   T_ASSERT( id < m_players.length());

   m_players[id]->player->setMedia( full_path.toLatin1().constData());

   m_players[id]->player->prepare( 0, [this, id](int64_t /*time*/, bool*) {
      /* read media length and video availability and propagate these info */
      const mdk::MediaInfo & info = m_players[id]->player->mediaInfo();

      emit durationChanged( id, info.duration);
      emit videoAvailable( id, info.video.size() > 0);

      /* MDK bug: 'mediaInfo' can not be called at any time. Sometimes results are wrong. */
      m_players[id]->hasVideo = (info.video.size() > 0);
      return true;
   });
}

void SdlWorkerThread::play(int id)
{
   /* create event for thread context */
   SDL_Event e;
   e.type = sdl_event_play;
   e.user.code = (Sint32)id;
   SDL_PushEvent( &e);
}

void SdlWorkerThread::pause(int id)
{
   /* create event for thread context */
   SDL_Event e;
   e.type = sdl_event_pause;
   e.user.code = (Sint32)id;
   SDL_PushEvent( &e);
}

void SdlWorkerThread::stop(int id)
{
   /* create event for thread context */
   SDL_Event e;
   e.type = sdl_event_stop;
   e.user.code = (Sint32)id;
   SDL_PushEvent( &e);
}

void SdlWorkerThread::togglePlayPause(int id)
{
   T_ASSERT( id < m_players.length());

   if (m_players.at(id)->player->state() == mdk::State::Playing)
   {
      pause( id);
   }
   else
   {
      play( id);
   }
}

void SdlWorkerThread::rewind(int id)
{
   /* create event for thread context */
   SDL_Event e;
   e.type = sdl_event_rewind;
   e.user.code = (Sint32)id;
   SDL_PushEvent( &e);
}

void SdlWorkerThread::setLoopPlayback(int id, bool enable)
{
   T_ASSERT( id < m_players.length());

   if (enable)
   {
      /* so far only loopback of full track is supported */
      m_players.at(id)->player->setRange(0);
      m_players.at(id)->player->setLoop( INT32_MAX);
   }
   else
   {
      m_players.at(id)->player->setLoop( 0);
   }
}

int64_t SdlWorkerThread::currentPosition(int id)
{
   T_ASSERT( id < m_players.length());

   return m_players.at(id)->player->position();
}

void SdlWorkerThread::setPosition(int id, int64_t positionMs)
{
   T_ASSERT( id < m_players.length());
   m_players.at(id)->player->seek( positionMs, mdk::SeekFlag::FromStart /*| mdk::SeekFlag::Fast*/);
}

void SdlWorkerThread::seekRelative(int id, int64_t deltaMs)
{
   T_ASSERT( id < m_players.length());
   m_players.at(id)->player->seek( deltaMs, mdk::SeekFlag::FromNow /*| mdk::SeekFlag::Fast */);
}

void SdlWorkerThread::seekFrame(int id, int frameNumber)
{
   T_ASSERT( id < m_players.length());
   m_players.at(id)->player->seek( frameNumber, mdk::SeekFlag::Frame | mdk::SeekFlag::FromNow);
}

void SdlWorkerThread::setVolume(int id, int volume)
{
   T_ASSERT( id < m_players.length());
   m_players.at(id)->player->setVolume( (float)volume/(float)100.);
}

int SdlWorkerThread::volume(int id)
{
   T_ASSERT( id < m_players.length());
   return (int)(m_players.at(id)->player->volume() * (float)100.);
}

void SdlWorkerThread::setMuted(int id, bool mute)
{
   T_ASSERT( id < m_players.length());
   m_players.at(id)->player->setMute( mute);
}

void SdlWorkerThread::updateWindowVisibility( int id)
{
   /* no video track available or audio-only requested by user */
   bool audioOnly = (m_players.at(id)->hasVideo == false) || (m_players.at(id)->audioOnly);


   if (audioOnly)
   {
      /* audio-only requsted or media not containing video track */
      SDL_HideWindow( m_players.at(id)->window);
   }
   else
   {
      SDL_ShowWindow( m_players.at(id)->window);
      SDL_RaiseWindow( m_players.at(id)->window);
   }
}

void SdlWorkerThread::checkForDisplays()
{
   int numDisplay = SDL_GetNumVideoDisplays();

   if ((numDisplay != m_numberOfDisplays) &&
       (numDisplay >= 1))
   {
      SDL_Rect bounds;
      m_numberOfDisplays = numDisplay;

      int res = SDL_GetDisplayBounds( m_numberOfDisplays - 1, & bounds);
      T_ASSERT( res == 0);

      /* put all windows in last display */
      for (PlayerSet * instance : m_players)
      {
         SDL_SetWindowPosition( instance->window, bounds.x, bounds.y);
         SDL_SetWindowSize( instance->window, bounds.w, bounds.h);
      }
   }

}

void SdlWorkerThread::setAudioOnly(int id, bool audioOnly)
{
   T_ASSERT( id < m_players.length());

   m_players.at(id)->audioOnly = audioOnly;

   if (m_players.at(id)->player->state() != mdk::State::Stopped)
   {
      /* when playing or paused, update window state immediately */
      updateWindowVisibility(id);
   }
}

bool SdlWorkerThread::toggleAudioOnly(int id)
{
   T_ASSERT( id < m_players.length());
   /* set opposite of current audio-only state */
   setAudioOnly( id, ! m_players.at(id)->audioOnly);

   return m_players.at(id)->audioOnly;
}

void SdlWorkerThread::process()
{
   SDL_Event event;

   SDL_Init(SDL_INIT_VIDEO);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // (default) this enables vsync

   m_waitReady.release();

   while (true) {
      SDL_WaitEvent( &event);

      switch (event.type) {
      case SDL_QUIT:
         break;
      case SDL_WINDOWEVENT:
          if (event.window.event == SDL_WINDOWEVENT_EXPOSED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
             struct PlayerSet * player = m_playerLookup.value( event.window.windowID, nullptr);
             if (player != nullptr) {
                int w = 100, h = 100;
                SDL_GetWindowSize( player->window, &w, &h);
                player->player->setVideoSurfaceSize(w, h);
             }

          }
         break;
      case SDL_KEYDOWN:
         if (event.key.keysym.sym == SDLK_SPACE) {
            struct PlayerSet * player = m_playerLookup.value( event.window.windowID, nullptr);
            /* toggle play/pause */
            if( player->player->state() == mdk::State::Playing) {
               player->player->set(mdk::State::Paused);
            } else {
               player->player->set(mdk::State::Playing);
            }
         }
         else if (event.key.keysym.sym == SDLK_ESCAPE) {
            for (PlayerSet * player: m_players) {
               player->player->set(mdk::State::Stopped);
            }
         }

         break;

      case sdl_event_play:
         checkForDisplays();
         play_internal( event.user.code);
         break;
      case sdl_event_pause:
         pause_internal( event.user.code);
         break;
      case sdl_event_stop:
         stop_internal( event.user.code);
         break;
      case sdl_event_rewind:
         rewind_internal( event.user.code);
         break;

      case sdl_event_create_window:
         addPlayer_internal( event.user.code);
         m_waitPlayerSetCreated.release();
         break;

      default:
         if ((event.type >= SDL_EVENT_UPDATE_FIRST) &&
             (event.type <= SDL_EVENT_UPDATE_LAST))
         {
            Uint32 win_index = event.type - SDL_EVENT_UPDATE_FIRST;
            T_ASSERT( win_index < (Uint32)m_players.length());

            PlayerSet * player = m_players[(int)win_index];
            SDL_GL_MakeCurrent( player->window, m_context);
            player->player->renderVideo();
            SDL_GL_SwapWindow( player->window);
         }
         break;
      }

   }

   T_ASSERT(false);
}

void SdlWorkerThread::addPlayer_internal( int32_t win_index)
{
   PlayerSet * aPlayerSet = new PlayerSet;

   aPlayerSet->player = new mdk::Player;
   int player_index = m_players.length();

   SDL_DisplayMode dispMode;
   SDL_GetCurrentDisplayMode(SDL_GetNumVideoDisplays() - 1, & dispMode);
   aPlayerSet->player->setVideoSurfaceSize(dispMode.w, dispMode.h);

   aPlayerSet->window =
         SDL_CreateWindow( QString("MDK player #%1").arg(win_index).toLatin1().constData(),
                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, dispMode.w, dispMode.h,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE |
                           /*SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS |*/ SDL_WINDOW_KEYBOARD_GRABBED);

   T_ASSERT( aPlayerSet->window);

   aPlayerSet->winId = SDL_GetWindowID( aPlayerSet->window);

   aPlayerSet->audioOnly = false;
   aPlayerSet->hasVideo = false;

   /* only create one context */
   if (m_context == nullptr) {
      m_context = SDL_GL_CreateContext( aPlayerSet->window);
   }

   aPlayerSet->player->onStateChanged([this, player_index](mdk::State s){
      emit playerStateChanged( player_index, s);

      if (s == mdk::State::Stopped)
      {
         SDL_HideWindow( m_players.at(player_index)->window);
      }
   });

   aPlayerSet->player->onMediaStatusChanged([this, player_index](mdk::MediaStatus s){
      emit mediaStatusChanged( player_index, s);
      return true;
   });

   aPlayerSet->player->currentMediaChanged([&]{
      // qDebug() << "current media changed";
   });


   aPlayerSet->player->setRenderCallback([player_index](void*){
      SDL_Event e;
      e.type = SDL_EVENT_UPDATE_FIRST + (Uint32)player_index;
      SDL_PushEvent(&e);
   });

   aPlayerSet->player->onEvent([](const mdk::MediaEvent& /*e*/){
      return false;
   });

   m_players.append( aPlayerSet);
   m_playerLookup.insert( aPlayerSet->winId, aPlayerSet);

   /* update new window according to desktops */
   checkForDisplays();
}


void SdlWorkerThread::play_internal(int id)
{
   T_ASSERT( id < m_players.length());
   m_players[id]->player->set(mdk::State::Playing);

   updateWindowVisibility( id);
}

void SdlWorkerThread::pause_internal(int id)
{
   T_ASSERT( id < m_players.length());
   m_players[id]->player->set(mdk::State::Paused);
}

void SdlWorkerThread::stop_internal(int id)
{
   T_ASSERT( id < m_players.length());

   m_players[id]->player->set(mdk::State::Stopped);
   SDL_HideWindow( m_players[id]->window);
}

void SdlWorkerThread::rewind_internal(int id)
{
   T_ASSERT( id < m_players.length());
   m_players[id]->player->seek(0);
}


