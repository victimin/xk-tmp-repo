#ifndef __SUB_HTTP_CLIENT_H__
#define __SUB_HTTP_CLIENT_H__


#ifdef __cplusplus
extern "C"
{
#endif
///////////////////////////////////////////////////////////////////

#define ENABLE_SUB_HTTP 1

int addJsonMultiRadarData(XK_HTTPHandle_t *HTTPHandle, char* msgData);

///////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif


#endif
