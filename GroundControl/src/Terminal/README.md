# Terminal Module

Overview
- The Terminal module provides a lightweight command parser and dispatcher for the ground-control serial/terminal interface. It parses incoming text commands, maps them to command types, and dispatches to registered handlers.

Command handler type
- `CommandHandler`: function pointer type: `void (*)(uint8_t argc, char* args[BUFFER_SIZE])` — handler receives an argument count and a fixed-size argument buffer.

Commands (enum `Commands`)
- `CMD_SYSTEM_RESTART` — Restart the current node/system.
- `CMD_SYSTEM_UPDATE` — Trigger a system update.
- `CMD_SYSTEM_CONFIG_SET` — Set a configuration value for the current node.
- `CMD_SYSTEM_CONFIG_GET` — Get a configuration value for the current node.
- `CMD_SYSTEM_CFG_DUMP` — Dump the current configuration.
- `CMD_SYSTEM_CFG_CLEAR` — Clear the current configuration.
- `CMD_SYSTEM_CFG_IMPORT` — Import configuration from input (e.g. rules/nodes).
- `CMD_NODE_ADD` — Add a node/node.
- `CMD_NODE_LS` — List registered nodes.
- `CMD_NODE_MV` — Rename a node/node.
- `CMD_NODE_RM` — Remove a node/node.
- `CMD_NODE_CALL` — Send a message to a node.
- `CMD_NODE_SYS_UPDATE` — Request OTA update for a node.
- `CMD_ROUTE_ADD` — Add a routing rule (not implemented).
- `CMD_ROUTE_LS` — List routing rules (not implemented).
- `CMD_ROUTE_RM` — Remove a routing rule (not implemented).
- `CMD_ROUTE_UPDATE` — Update a routing rule (not implemented).
- `CMD_HELP` — Print help text.
- `CMD_COUNT` — Internal count / placeholder for number of commands.

Terminal class (brief)
- `Terminal()` — Constructor; initializes internal state.
- `void poll()` — Polls for input, parses available data, and dispatches commands.
- `void attachHandler(Commands type, CommandHandler handler)` — Register a handler for a specific command type.

Internal/private members (for reference)
- `ESPNowWrapper* espNow` — pointer to ESP-NOW wrapper used for node messaging.
- `CommandHandler handlers[CMD_COUNT]` — handler table indexed by `Commands`.
- `char buffer[BUFFER_SIZE]` — input buffer used for building/parsing commands.
- `char delimiter` — delimiter used for separating commands (defaults to `\n`).
- `void parseCommand()` — internal parser invoked when input is available.
- `void handleCommand(uint8_t cmdType, uint8_t argc, char* argv[BUFFER_SIZE])` — internal dispatcher.
- `void printHelp(String cmdToken)` — prints help for a given token.
- `void bindHandlers()` — sets up any built-in/default handlers.

Usage example
```
// register a handler for node add
void onNodeAdd(uint8_t argc, char* args[BUFFER_SIZE]){
  // parse args and handle
}

Terminal term;
term.attachHandler(CMD_NODE_ADD, onNodeAdd);

// in loop
term.poll();
```

See the header for details and comments: [src/Terminal/terminal.h](src/Terminal/terminal.h#L1-L200)
