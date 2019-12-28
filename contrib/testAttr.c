#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/time.h>
#ifdef __APPLE__
#include <sys/attr.h>
#include <sys/paths.h>
#endif /* __APPLE__ */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "applefile.h"

extern struct attrlist	setalist;

int
main(int argc, char **argv)
{
  char                        finfo[ FINFOLEN ];
  int			      i;
  getattrlist( argv[1], &setalist, finfo, FINFOLEN,
	       FSOPT_NOFOLLOW );
  for (i=0; i<FINFOLEN; i++) printf("%.2X ", finfo[i]);
  printf("\n");
}
