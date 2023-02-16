#ifndef MEDIAENGINE_MDKSDL_H
#define MEDIAENGINE_MDKSDL_H

#include "IF_MediaEngineInterface.h"
#include "AbstractMediaSource.h"

#include "SdlEngine.h"

#include <QTimer>

class Fader;
class FullScreenMediaWidget_IF;
class ApplicationSettings;
class StatusDisplay;


/** this class will aggregate specific objects for plugin used
 *  in implementation. As it is not easily testable, implementation
 *  should be simple and with as few logic as possible.
 */
class MediaEngineMdkSdl : public IF_MediaEngineInterface
{
   Q_OBJECT
public:
   explicit MediaEngineMdkSdl(SdlEngine & sdlEngine, Fader & fader,
                              FullScreenMediaWidget_IF &displayWidget, StatusDisplay & logger,
                              QObject *parent = nullptr);
   ~MediaEngineMdkSdl() override;

   /** used to bind a "screen" widget to media engine implementation */
   void setWidgetForRender(QWidget * canvas) override;

public slots:
   void checkPlatform() override;
   void setCurrentSource( const AbstractMediaSource *source, bool dontStopFlag = false ) override;
   void play() override;
   void pause() override;
   void togglePlayPause() override;
   void stop() override;
   void rewind() override;
   void setStepSizeMs( int stepMs) override  {
      m_stepMs = stepMs;
   }
   void stepForward() override;
   void stepBackward() override;
   void singleFrameForward() override;
   void singleFrameBackward() override;
   void setVolume(int vol) override;
   int volume() override;
   void setTickInterval(qint32 tick_ms ) override;
   void onUserPositionRequested( qint64 positionMs) override;
   void setMuted(bool isMuted) override;
   void setAudioOnly(bool audioOnly) override;
   void toggleAudioOnly() override;
   void enableFadeIn( bool enabled) override;
   void setLoopPlayback( bool enabled) override;
   void enableSubtitles() override;
   void disableSubtitles() override;

private slots:
   /* any instance of a player has changed its state */
   void onGlobalMediaStateChanged( int playerId, MediaObject::AvMediaState newState);
   void onGlobalPlayerStateChanged( int playerId, MediaObject::AvPlayerState newState);
   void onGlobalDurationChanged( int playerId, int64_t duration_ms);
   void onGlobalVideoAvailable( int playerId, bool available);

   void onTimerTick();


private:
   SdlEngine & m_sdlEngine;
   Fader & m_fader;
   FullScreenMediaWidget_IF & m_displayWidget;
   StatusDisplay & m_logger;
   QTimer m_tickTimer;
   QWidget * m_canvas;  /* only used for still picture */

   SdlEngine::PlayerId m_playerId;
   bool m_videoTrackAvailable;  /* current media has a video track */
   qint32 m_tickMs;
   qint32 m_stepMs;  /* use for jump forward or backward */
   bool m_fadeInFlag;
   bool m_imageFileFlag;
};


#endif // MEDIAENGINE_MDKSDL_H
