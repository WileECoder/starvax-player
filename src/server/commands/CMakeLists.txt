
set(STARVAX_SOURCES
  ${STARVAX_SOURCES}
   src/server/commands/ControlOwnCommand.cpp
   src/server/commands/ControlOwnCommand.h
   src/server/commands/DisconnectCommand.cpp
   src/server/commands/DisconnectCommand.h
   src/server/commands/GetLightsetListCommand.cpp
   src/server/commands/GetLightsetListCommand.h
   src/server/commands/GetOwnListCommand.cpp
   src/server/commands/GetOwnListCommand.h
   src/server/commands/GetShowPathCommand.cpp
   src/server/commands/GetShowPathCommand.h
   src/server/commands/GetTrackListCommand.cpp
   src/server/commands/GetTrackListCommand.h
   src/server/commands/PauseTrackCommand.cpp
   src/server/commands/PauseTrackCommand.h
   src/server/commands/PlayTrackCommand.cpp
   src/server/commands/PlayTrackCommand.h
   src/server/commands/RewindMediaCommand.cpp
   src/server/commands/RewindMediaCommand.h
   src/server/commands/SelectLightTransitionCommand.cpp
   src/server/commands/SelectLightTransitionCommand.h
   src/server/commands/SetActiveTrackCommand.cpp
   src/server/commands/SetActiveTrackCommand.h
   src/server/commands/SetCredentialsCommand.cpp
   src/server/commands/SetCredentialsCommand.h
   src/server/commands/StopMediaCommand.cpp
   src/server/commands/StopMediaCommand.h
  PARENT_SCOPE
)

set ( ALL_SRC_DIRS
   ${ALL_SRC_DIRS}
   ${CMAKE_CURRENT_SOURCE_DIR}
   PARENT_SCOPE
)