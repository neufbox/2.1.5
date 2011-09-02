
/* $Id: ctlan.h 8219 2008-09-16 08:17:00Z mgo $ */

#ifndef _LIBNBD_DDNSCTL_H_
#define _LIBNBD_DDNSCTL_H_

#include "plugins/ddnsctl.h"

#include <string.h>

int nbd_ddnsctl_start( void );
int nbd_ddnsctl_stop( void );
int nbd_ddnsctl_restart( void );
int nbd_ddnsctl_status( char **buf_xml, size_t * buf_xml_size );

#endif
