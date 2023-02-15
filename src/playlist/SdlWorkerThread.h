#ifndef SDLWORKERTHREAD_H
#define SDLWORKERTHREAD_H

#include <QObject>
#include <QVector>
#include <QMap>

#include <QMutex>
#include <QSemaphore>


#include <mdk/Player.h>
#include <SDL.h>
#include <SDL_video.h>


class SdlWorkerThread : public QObject
{
   Q_OBJECT
public:
   explicit SdlWorkerThread( QSemaphore & waitready,
                             QObject *parent = nullptr);

   ~SdlWorkerThread();

   int addPlayer();

   void load(int id, const QString & full_path);
   void play( int id);
   void pause( int id);
   void stop( int id);
   void togglePlayPause( int id);
   void rewind( int id);
   void setLoopPlayback( int id, bool enable);
   int64_t currentPosition( int id);

   void setPosition( int id, int64_t positionMs);
   void seekRelative( int id, int64_t deltaMs);
   void seekFrame(int id, int frameNumber);

   void setVolume( int id, int volume);
   int volume( int id);
   void setMuted( int id, bool mute);

   void setAudioOnly( int id, bool audioOnly);
   /** \return true if player has become 'audio-only' after this call*/
   bool toggleAudioOnly( int id);

public slots:
   void process();

signals:
   void mediaStatusChanged( int id, mdk::MediaStatus newStatus);
   void playerStateChanged( int id, mdk::State newState);
   void durationChanged( int id, int64_t duration_ms);
   void videoAvailable( int id, bool available);

private:
   /* methods executed in thread context */
   void addPlayer_internal( int32_t win_index);

   void play_internal( int id);
   void pause_internal( int id);
   void stop_internal( int id);
   void rewind_internal( int id);

   void updateWindowVisibility(int id);

private:
   struct PlayerSet {
      mdk::Player * player;
      SDL_Window  * window;
      Uint32 winId;
      bool audioOnly;
   };

   QVector<PlayerSet *> m_players;
   QMap<Uint32, PlayerSet *> m_playerLookup;

   /* updated on request, before Player is actually created */
   int m_reservedPlayersCount;

   SDL_GLContext m_context;
   /* released when SDL has been initialized */
   QSemaphore & m_waitReady;
   /* wait for a player-set to be created */
   QSemaphore m_waitPlayerSetCreated;
   QMutex m_locker;

};

#endif // SDLWORKERTHREAD_H
