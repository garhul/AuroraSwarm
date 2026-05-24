
### Serial commands
Serial commands are case insensitive



node ls
node rm -a, -m mac_address
node name -m name
node update -m -v?
node 

up -i index | -m mac address
mv -i index | -m mac address new name
send -i index | -m mac_address | -a address MSG
set relay
clear relay 
list

### Message Protocol

1B, 1B, ? 
MSG, Payload size, payload

Max peers = 19 (non encrypted + broadcast addr)

Broker actions:
- Pair request handling
- Allow / Disallow auto-pairing
- Add a node
- Remove a node
- Send message to node
- Send message to all nodes
- Provide serial terminal
  - list nodes
  - remove node
  - clear nodes
  - request node update
  - label/name node
  - send message to node
  - register message relay (origin mac - destination mac(s))
  - ?export config
  - ?load config

Peer actions
 - Request pairing
 - Report Sensor data (for weather stations / ambient sensors)
 - Report Status update (on state change) ^

