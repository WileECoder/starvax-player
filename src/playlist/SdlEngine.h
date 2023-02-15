#ifndef SDLENGINE_H
#define SDLENGINE_H

#include <QObject>
#include "IF_MediaEngineInterface.h"

class SdlEngine : public QObject
{
   Q_OBJECT
public:
   SdlEngine( QObject * parent = nullptr) :
      QObject(parent)
   {}

   virtual ~SdlEngine() override {}

   typedef int PlayerId;

   virtual PlayerId addPlayer() = 0;

   virtual void load( PlayerId id, const QString & full_path) = 0;
   virtual void play( PlayerId id) = 0;
   virtual void pause( PlayerId id) = 0;
   virtual void stop( PlayerId id) = 0;
   virtual void togglePlayPause( PlayerId id) = 0;
   virtual void rewind( PlayerId id) = 0;

   virtual void setLoopPlayback( PlayerId id, bool enable) = 0;
   virtual int64_t currentPosition( PlayerId id) = 0;
   virtual void setPosition( PlayerId id, int64_t positionMs) = 0;
   virtual void seekRelative( PlayerId id, int64_t deltaMs) = 0;
   virtual void seekFrame( PlayerId id, int frameNumber) = 0;

   /** \p vol: volume in range 0-100 */
   virtual void setVolume( PlayerId id, int vol) = 0;
   virtual int volume( PlayerId id) = 0;
   virtual void setMuted( PlayerId id, bool mute) = 0;

   virtual void setAudioOnly( PlayerId id, bool audioOnly) = 0;
   /** \return true if player has become 'audio-only' after this call*/
   virtual bool toggleAudioOnly( PlayerId id) = 0;

signals:
   void mediaStatusChanged( int playerId, MediaObject::AvMediaState newStatus);
   void playerStateChanged( int playerId, MediaObject::AvPlayerState newState);
   void durationChanged( int playerId, int64_t duration_ms);
   void videoAvailable( int playerId, bool available);
};

#endif // SDLENGINE_H
