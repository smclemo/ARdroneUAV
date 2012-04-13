#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

#include <Navdata/navdata.h>
#include <Navdata/NavDataContainer.hpp>
#include <stdio.h>

#define NAVDATADUMP 0

int newFrameAvailable = 0;
int lastframedata = 0;

int32_t ALTITUDE; //Altitude in mm (global variable)
float32_t PSI; //Current direction of heading (-180deg to 180deg)

/* Initialization local variables before event loop  */
inline C_RESULT demo_navdata_client_init( void* data )
{
#if NAVDATADUMP
  ardrone_navdata_file_init( NULL );
#endif
  initializeGlobalNavData();
  return C_OK;
}

/* Receving navdata during the event loop */
inline C_RESULT demo_navdata_client_process( const navdata_unpacked_t* const navdata )
{
#if NAVDATADUMP
  ardrone_navdata_file_process( navdata );
#endif
  if(lastframedata != navdata->navdata_demo.num_frames) newFrameAvailable++;
  lastframedata = navdata->navdata_demo.num_frames;

  ALTITUDE = navdata->navdata_altitude.altitude_raw;
  PSI = navdata->navdata_demo.psi / 1000.0;

  saveNavData(navdata); //save data to global struct

  return C_OK;
}

/* Relinquish the local resources after the event loop exit */
inline C_RESULT demo_navdata_client_release( void )
{
#if NAVDATADUMP
  ardrone_navdata_file_release();
#endif
  return C_OK;
}

/* Registering to navdata client */
BEGIN_NAVDATA_HANDLER_TABLE
  NAVDATA_HANDLER_TABLE_ENTRY(demo_navdata_client_init, demo_navdata_client_process, demo_navdata_client_release, NULL)
END_NAVDATA_HANDLER_TABLE

