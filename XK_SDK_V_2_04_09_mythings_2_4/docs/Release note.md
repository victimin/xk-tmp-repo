## Release notes

### 2.04.05.mythings.2.4   20-12-17    Jerry
### 2.04.05.mythings.2.4   note
* added production API logic


### 2.04.05.mythings.2.3   20-12-11    Jerry
### 2.04.05.mythings.2.3   note
* added production API logic


### 2.04.05.mythings.2.2   20-12-01    Jerry
### 2.04.05.mythings.2.2   note
* added presence-vital parsing logic for behrtech message


### 2.04.05.mythings.2.1   20-11-26    Jerry
### 2.04.05.mythings.2.1   note
* added Zone-Inout combination logics


### 2.04.05.mythings.2.0   20-11-19    Jerry
### 2.04.05.mythings.2.0   note
* added H/W reset logic


### 2.04.05.mythings.1.4   20-06-02    Jerry
### 2.04.05.mythings.1.4   note
* added radar Heartbeat LED logic


### 2.04.05.mythings.1.3   20-05-25    Jerry
### 2.04.05.mythings.1.3   note
* added 2 more error type


### 2.04.05.mythings.1.1   20-05-08    Jerry
### 2.04.05.mythings.1.1   note
* added CMD & Separator for each sensor
* added error type


### 2.04.05.mythings   20-05-08    Jerry
### 2.04.05.mythings   note
* Made for Mythings Behrtech.


### 2.04.04   20-04-24    Jerry
### 2.04.04   note
* drastically reduced cpu load rate


### 2.04.03   20-04-22    Jerry
### 2.04.03   note
* Fixed a bug that could not be transmitted after a certain time
* Allow "ok" in response
* Fixed minor bugs 


### 2.04.02   20-03-30    Jerry
### 2.04.02   note
* bug fixed(serial number pointer bug)
* CheckAppInfo(funtion) added(serial number pointer bug)


### 2.04.01   20-03-30    Jerry
### 2.04.01   note
* new version numbering
* fixed several small bugs
* added SIC(Send if changed) logic (It can be on to set 'send-if-changed' as '1' in XK_CommonDefineSet.h)
  (the interval can be set 'SIC-interval' as '1' in XK_CommonDefineSet.h) 


### 2.3.1   20-03-25    Jerry
### 2.3.1   note
* fixed several small bugs
* added new uart_app(2.1): printing all ports


### 2.3.0   20-03-15    Jerry
### 2.3.0   note
* http source logic cleaned up
* Added logic to restart xk-sdk when there is no response from the server
* Changed all fall detection data to a decimal
* fixed several small bugs


### 2.2.0   20-02-21    Jerry
### 2.2.0   note
* Modified sending message format(meaningful object name for internal API)


### 2.1.9   20-02-13    Jerry
### 2.1.9   note
* Modified parsing logic for config.json to operate exception handling


### 2.1.7   20-02-06    Jerry
### 2.1.7   note
* Added kill pre-process logic by -f option
* Added 'accountId' to send HTTP message in header (It can be set in config.json)
* Added channel to send HTTP/HTTPS message to multi server (It can be set 'ONOFF_SUB_EPOINT' as '1' in XK_CommonDefineSet.h)


### 2.1.6   20-02-05    Danny
### 2.1.6   note
* Added the Button Logic
* Added the On/Off shorcut in ~/Desktop folder


### 2.1.5   20-01-16    Danny
### 2.1.5   note
* Modified the logic to change WMFD app number 


### 2.1.4   20-01-15    Jerry
### 2.1.4   note
* Created release note 