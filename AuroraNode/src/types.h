/** This doesn't need to be here, Broker doesn't know what AuroraNode is */
enum class AURORA_COMMANDS {
  CMD_PLAY,
  CMD_OFF,
  CMD_PAUSE,
  CMD_FX,
  CMD_FX_NEXT,
  CMD_FX_PREV,
  CMD_FX_SPEED,
  CMD_SET_BR,
  CMD_SET_HSV,
  CMD_SET_PX,
  COMMANDS_COUNT
};

enum class MESSAGE_FORMAT {
  MSG_HUMAN_READABLE,
  MSG_BINARY
}; 