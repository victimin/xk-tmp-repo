## Release notes

### 2.06.03   20-12-09    Jerry
### 2.06.03   note
* added AP mode logic


### 2.06.02   20-12-02    Jerry
### 2.06.02   note
* added registration API in the production logic


### 2.06.01   20-11-25    Jerry
### 2.06.01   note
* added a parameter(device ID) for zone-inout combo in the data what sending to each Zone sensor


### 2.06.00   20-11-15    Jerry
### 2.06.00   note
* added shm logic to operate gateway with Gateway Manager


### 2.05.04   20-11-02    Jerry
### 2.05.04   note
* added combination(inout/zone combo) logic


### 2.05.03   20-10-14    Jerry
### 2.05.03   note
* modified config.json format
* added Time object in MQTT message


### 2.05.02   20-09-24    Jerry
### 2.05.02   note
* modified condition of sending Heart rate(about "6610")


### 2.05.01   20-09-18    Jerry
### 2.05.01   note
* modified MQTT logic


### 2.05.00   20-09-18    Jerry
### 2.05.00   note
* Added MQTT logic


### 2.04.11   20-07-03    Jerry
### 2.04.11   note
* merged new fall logics


### 2.04.10   20-07-03    Jerry
### 2.04.10   note
* added OSR, Skimmer app info
* added exit bed, fidgeting printing logic
* merged new fall logics


### 2.04.09   20-06-20    Jerry
### 2.04.09   note
* added production in bulk logic


### 2.04.08   20-06-15    Jerry
### 2.04.08   note
* added 2 parameter to send to GUI in fall plugin
* added a logic to select v, m mode in fall plugin


### 2.04.07   20-06-12    Danny
### 2.04.07   note
* updated minor in fall plugin


### 2.04.06   20-06-08    Jerry, Danny
### 2.04.06   note
* fixed fall plugin point bug


### 2.04.05   20-05-25    Jerry
### 2.04.05   note
* added a logic to delete system log periodically


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