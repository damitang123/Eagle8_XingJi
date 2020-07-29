#ifndef _GPS_H_
#define  _GPS_H_

extern float g_fLongitude, g_fLatitude;
extern unsigned char g_bGPS_Location_Valid;

void Gps_Init(void);
void Gps_Data_Parse(void);
unsigned char GPS_Is_Location_Valid(void);
unsigned char Get_GPS_Location_Value(float *pLongitude, float *pLatitude);
unsigned char Get_GPS_Location_String(char *strLongitude, char *strLatitude);

#endif

