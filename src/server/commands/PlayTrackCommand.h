#ifndef PLAYTRACKCOMMAND_H
#define PLAYTRACKCOMMAND_H

#include "Command_IF.h"
class IF_MediaEngineInterface;

// TODO think about the great code duplication in all
// commands related to playlist that have 'line' as parameter

// TODO replace all "track" with "media"

namespace Server {

class CommandReply_IF;

/**
 * @brief The PlayTrackCommand plays the track that is currently
 *   active. It is not enough to be selected. Nothing happens
 *   if no track has focus.
 */
class PlayTrackCommand : public Command_IF
{
public:
   PlayTrackCommand( IF_MediaEngineInterface & mediaEngine_A,
                     IF_MediaEngineInterface & mediaEngine_B,
                     CommandReply_IF & replySink);

   ~PlayTrackCommand() override{}

   // Command_IF interface
public:
   quint8 operativeCode() const override{
      return PLAY_MEDIA;
   }

   /**
    * @brief plays current track.
    * @param parameters is one string that can be "A", "B" or "A_B"
    * @return
    */
   bool execute(const QStringList & parameters) override;

private:
   IF_MediaEngineInterface & m_mediaEngine_A;
   IF_MediaEngineInterface & m_mediaEngine_B;
   CommandReply_IF & m_replySink;
};

}  // namespace Server

#endif // PLAYTRACKCOMMAND_H
