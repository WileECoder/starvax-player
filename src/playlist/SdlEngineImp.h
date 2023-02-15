#ifndef SDLENGINEIMP_H
#define SDLENGINEIMP_H

#include "SdlEngine.h"

#include <mdk/global.h>

class SdlWorkerThread;
class QThread;

class SdlEngineImp : public SdlEngine
{
   Q_OBJECT
public:
   SdlEngineImp( QObject * parent = nullptr);

   /// start the SDL event loop
   /// @note To be called only once in only one instance.
   void launch();

   // SdlEngine interface
public:
   PlayerId addPlayer() override;

   void load(PlayerId id, const QString& full_path) override;
   void play(PlayerId id) override;
   void pause(PlayerId id) override;
   void stop(PlayerId id) override;
   void togglePlayPause( PlayerId id) override;
   void rewind(PlayerId id) override;
   void setLoopPlayback( PlayerId id, bool enable) override;

   int64_t currentPosition( PlayerId id) override;
   void setPosition( PlayerId id, int64_t positionMs) override;
   void seekRelative( PlayerId id, int64_t deltaMs) override;
   void seekFrame( PlayerId id, int frameNumber) override;

   void setVolume( PlayerId id, int vol) override;
   int volume( PlayerId id) override;
   void setMuted( PlayerId id, bool mute) override;

   void setAudioOnly( PlayerId id, bool audioOnly) override;
   /** \return true if player has become 'audio-only' after this call*/
   bool toggleAudioOnly( PlayerId id) override;

private slots:
   void onMediaStatusChanged( int id, mdk::MediaStatus newStatus);
   void onPlayerStateChanged( int id, mdk::State newState);


private:
   SdlWorkerThread * m_worker;
   QThread * m_thread;

   bool m_launched;
};

#endif // SDLENGINEIMP_H
