#ifndef __I3G4250D_DRIVER_H
#define __I3G4250D_DRIVER_H

#ifdef __cplusplus
 extern "C" {
#endif

	 void Gyroscope_Init(void);
	 void Gyroscope_lopp(void);
	 void Gyroscope_SetDataReady(void);
	 float Gyroscope_GetAngleX(void);
	 float Gyroscope_GetAngleY(void);
	 float Gyroscope_GetAngleZ(void);

	 #ifdef __cplusplus
}
#endif

#endif
