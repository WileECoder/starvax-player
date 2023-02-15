#ifndef GETTRACKLISTCOMMAND_H
#define GETTRACKLISTCOMMAND_H

#include "Command_IF.h"
class MediaListModel;


namespace Server {
class CommandReply_IF;

/**
 * @brief The GetActiveTrackCommand returns all the
 *   tracks in current show for a given line (A or B)
 */
class GetTrackListCommand : public Command_IF
{
public:
   GetTrackListCommand( MediaListModel & mediaModel_A,
                        MediaListModel & mediaModel_B,
                        CommandReply_IF & replySink);
   ~GetTrackListCommand() override {}

   // Command_IF interface
public:
   quint8 operativeCode() const override{
      return GET_TRACK_LIST;
   }

   /**
    * @brief retrive media list
    * @param parameters should be one string: "A" or "B"
    * @return true if parameter is correct
    */
   bool execute(const QStringList & parameters) override;

private:
   MediaListModel & m_mediaModel_A;  // line A
   MediaListModel & m_mediaModel_B;  // line B
   CommandReply_IF & m_replySink;

private:
   char selectModelTag(const QStringList & parameters);
   MediaListModel * selectModel( char modelTag);
};

}  // namespace Server

#endif // GETTRACKLISTCOMMAND_H
