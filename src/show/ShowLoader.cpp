#include <QFileInfo>
#include <QTextStream>

#include "ShowLoader.h"
#include "IF_ShowGuiInterface.h"
#include "ShowFileFactory.h"
#include "showfileparser.h"
#include "IF_ScriptEngineInterface.h"
#include "MediaListModel.h"
#include "LightPresetModel.h"
#include "SequenceEditorGui.h"
#include "ApplicationSettings.h"
#include "testableAssert.h"


ShowLoader::ShowLoader(IF_ShowGuiInterface & fileSelect, ShowFileFactory *parserFactory,
                        MediaListModel *mediaModelLineA, MediaListModel * mediaModelLineB,
                        LightPresetModel *lighModel,
                        SequenceEditorGui *sequencerGui,
                        IF_ScriptEngineInterface *scriptEngine,
                        ApplicationSettings & applicationSettings, QObject *parent) :
   QObject(parent),
   m_guiInterface( fileSelect),
   m_parserFactory( parserFactory),
   m_mediaModelLineA( mediaModelLineA),
   m_mediaModelLineB( mediaModelLineB),
   m_lightModel( lighModel),
   m_sequencerGui( sequencerGui),
   m_scriptEngine( scriptEngine),
   m_applicationSettings( applicationSettings)
{
   T_ASSERT( parserFactory != nullptr);
   T_ASSERT( mediaModelLineA != nullptr);
   T_ASSERT( lighModel != nullptr);
   T_ASSERT( scriptEngine != nullptr);
}

ShowLoader::~ShowLoader()
{
   delete m_parserFactory;
}

void ShowLoader::LoadShow(const QString &filePath)
{
   QString openFileName = selectTargetFile(filePath);

   if (openFileName != QString())
   {
      T_ASSERT_REP (QFileInfo::exists(openFileName),
                    QString( tr("file %1 does not exist").arg(openFileName)));

      loadExistingFile(openFileName);
   }
}

QString ShowLoader::selectTargetFile(const QString &filePath) const
{
   QString openFileName;

   /* if passed name is empty, prompt the user for one */
   if (filePath == QString())
   {
      openFileName = m_guiInterface.selectShowToOpen();
   }
   else
   {
      openFileName = filePath;
   }

   return openFileName;
}

/**
 * load file \p fileName, that is supposed to exist
 */
void ShowLoader::loadExistingFile(const QString &fileName)
{
   QFile showFile(fileName);
   showFile.open( QIODevice::ReadOnly );

   T_ASSERT_REP( showFile.isOpen(),
                 tr("Unable to open: %1").arg(fileName));

   QTextStream stream( &showFile);
   QDir fileDir = QFileInfo(fileName).absoluteDir();
   ShowFileParser * parser = m_parserFactory->buildShowParser( stream, fileDir);

   parser->parse();

   if (confirmShowFileLoad(parser))
   {
      loadShowFile( &stream, fileName, parser);

      /* set file-changed flag if any error occourred */
      if (parser->errorLog().size() > 0)
      {
         emit loadShowCorrupted();
      }
   }

   delete parser;
}

/**
 * In case of parse errors, ask user to confirm to load file
 * anyway; return true in case parser has no errors.
 */
bool ShowLoader::confirmShowFileLoad(const ShowFileParser *parser) const
{
   return m_guiInterface.confirmShowFileLoad( parser->errorLog().join("\n"));
}

void ShowLoader::loadShowFile(QTextStream *strShow, const QString &filename, ShowFileParser *parser)
{
   resetShowData();
   loadShowScript( strShow );
   loadShowData( filename, *parser);

   emit showNameChanged( filename );

   m_applicationSettings.addRecentShowFile( filename);
}

/**
  * load the script so that it will be displayed
  * in script viewer
  */
void ShowLoader::loadShowScript( QTextStream *strShow )
{
   /*
    * The whole file can be passed as html, because show data,
    * (music, lights, ...), are formatted as html comments
    */
   strShow->seek(0);
   emit scriptContent( strShow->readAll() );
}

/** distribute show data to the components that need it
 */
void ShowLoader::loadShowData(const QString &filename, const ShowFileParser &parser )
{
   emit newSearchPath( QFileInfo(filename).absolutePath() );

   foreach (QString mediaPath, parser.trackListLineA())
   {
      m_mediaModelLineA->addMediaFile( mediaPath);
   }

   foreach (QString mediaPath, parser.trackListLineB())
   {
      m_mediaModelLineB->addMediaFile( mediaPath);
   }

   foreach (LightPresetData *lightset, parser.lightsetList() )
   {
      m_lightModel->updatePreset( *lightset );
   }

   m_sequencerGui->setScriptContent( parser.sequencerScript());
}

/**
  * clean up current show
  */
void ShowLoader::resetShowData()
{
   m_scriptEngine->setScriptContent("");
   m_mediaModelLineA->removeRows( 0, m_mediaModelLineA->rowCount());
   m_mediaModelLineB->removeRows( 0, m_mediaModelLineB->rowCount());
   m_lightModel->removeRows( 0, m_lightModel->rowCount());
   m_sequencerGui->setScriptContent( QString());
}
