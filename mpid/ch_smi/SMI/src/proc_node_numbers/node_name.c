/* $Id$ */

#include "node_name.h"
#include "startup/tcpsync.h"

smi_error_t SMI_Get_node_name (char *nodename, size_t *namelen)
{
  DSECTION("SMI_Get_node_name");
  int iError;
  char szHostName[MAXHOSTNAMELEN];

  ASSERT_R((nodename != NULL), "illegal parameter for nodename", SMI_ERR_PARAM);
  ASSERT_R((namelen != NULL), "illegal parameter for namelen", SMI_ERR_PARAM);
  ASSERT_R(((*namelen) > 0), "illegal parameter for *namelen", SMI_ERR_PARAM);

  iError = _smi_tcp_init();
  ASSERT_R((iError == 0),"could not init tcp", SMI_ERR_OTHER);

  iError = gethostname(szHostName, MAXHOSTNAMELEN);
  ASSERT_R((iError == 0),"gethostname failed", SMI_ERR_OTHER);

  if ( strlen(szHostName) >= (*namelen) ) {
    DPROBLEM("insufficient space is provided for hostname");
    DSECTLEAVE
      return(SMI_ERR_OTHER);
  }

  *namelen = strlen(szHostName);
  
  strcpy(nodename, szHostName);
  _smi_tcp_finalize();

  return(SMI_SUCCESS);
}
