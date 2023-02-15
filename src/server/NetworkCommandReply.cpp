#include "NetworkCommandReply.h"
#include <QIODevice>
#include <QDataStream>



Server::NetworkCommandReply::NetworkCommandReply() :
   m_device( nullptr)
{
}


void Server::NetworkCommandReply::sendReplay( quint8 replyCode, quint8 requestCode,
                                              const QString & message)
{
   sendReplay( replyCode, requestCode, QStringList(message));
}

// TODO replace 'message' with 'item'
void Server::NetworkCommandReply::sendReplay( quint8 replyCode, quint8 requestCode,
                                              const QStringList & messageList)
{
   // TODO this module has 2 different reasons to change:
   // formatting and network access
   if (m_device != nullptr)
   {
      QByteArray content;

      content += static_cast<char>(replyCode);
      content += static_cast<char>(requestCode);

      /* number of messages */
      content += static_cast<char>(messageList.size());

      foreach (QString message, messageList)
      {
         content += message.toLatin1();
         /* add terminator */
         content += '\0';
      }

      // termination
      content += static_cast<unsigned char>(0xFF);
      content += static_cast<unsigned char>(0xFF);

      m_device->write( content);
   }
}

