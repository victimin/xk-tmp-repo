#ifndef __BLE_HCI__
#define __BLE_HCI__

#ifdef __cplusplus
extern "C"
{
#endif

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

struct hci_request ble_hci_request(uint16_t ocf, int clen, void * status, void * cparam);

int GetHciDevice();
void SetBleScanParams(int device);
void SetBleEventsReportMask(int device);
void EnableScanning(int device, le_set_scan_enable_cp * scan_cp_dest);
void GetResults(int device);
void DisableScanning(int device, le_set_scan_enable_cp * scan_cp_dest);

#ifdef __cplusplus
}
#endif
#endif

