
### Serial commands
Serial commands are case insensitive



device ls
device rm -a, -m mac_address
device name -m name
device update -m -v?
device 

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
- Add a device
- Remove a device
- Send message to device
- Send message to all devices
- Provide serial terminal
  - list devices
  - remove device
  - clear devices
  - request device update
  - label/name device
  - send message to device
  - register message relay (origin mac - destination mac(s))
  - ?export config
  - ?load config

Peer actions
 - Request pairing
 - Report Sensor data (for weather stations / ambient sensors)
 - Report Status update (on state change) ^

