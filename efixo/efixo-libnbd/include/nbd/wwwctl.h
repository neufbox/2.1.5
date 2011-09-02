#ifndef _LIBNBD_WWWCTL_H_
#define _LIBNBD_WWWCTL_H_

#include "plugins/wwwctl.h"

#include <string.h>

int nbd_wwwctl_start( void );
int nbd_wwwctl_stop( void );
int nbd_wwwctl_restart( void );
int nbd_wwwctl_reload( void );
int nbd_wwwctl_status( char **buf_xml, size_t * buf_xml_size );

#endif
