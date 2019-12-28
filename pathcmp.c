/*
 * Copyright (c) 2003 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */

#include "config.h"

#ifdef __APPLE__
#define USE_ASCII	1
#endif /* __APPLE__ */

#include <sys/param.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>

#include "pathcmp.h"

    int
pathcasecmp( const char *p1, const char *p2,
    int case_sensitive )
{
    int		rc;
    unsigned	char	path1[MAXPATHLEN], path2[MAXPATHLEN];

    strcpy(path1, decode(p1));
    strcpy(path2, decode(p2));

    do {
	if ( case_sensitive ) {
	    rc = ( (unsigned char)*path1 - (unsigned char)*path2 );
	} else {
	    rc = ( tolower( *path1 ) - tolower( *path2 ));
	}

	if ( rc != 0 ) {
	    if (( *p2 != '\0' ) && ( *p1 == '/' )) {
		return( -1 );
	    } else if (( *path1 != '\0' ) && ( *path2 == '/' )) {
		return( 1 );
	    } else {
		return( rc );
	    }
	}
	path2++;
    } while ( *path1++ != '\0' );

    return( 0 );
}

/* Just like strcmp(), but pays attention to the meaning of '/'.  */
    int 
pathcmp( const char *p1, const char *p2 )
{
    return( pathcasecmp( p1, p2, 1 ));
}

    int
ischildcase( const char *child, const char *parent, int
    case_sensitive )
{
    int		rc;
    size_t	parentlen;


    if ( parent == NULL ) {
	return( 1 );
    }

    parentlen = strlen( parent );

    if ( parentlen > strlen( child )) {
	return( 0 );
    }
    if (( 1 == parentlen ) && ( '/' == *parent )) {
	return( '/' == *child );
    }

    if ( case_sensitive ) {
	rc = strncmp( parent, child, parentlen );
    } else {
	rc = strncasecmp( parent, child, parentlen );
    }
    if (( rc == 0 ) && (( '/' == child[ parentlen ] ) ||
	    ( '\0' == child[ parentlen ] ))) {
	return( 1 );
    }
    return( 0 );
}

    int
ischild( const char *child, const char *parent )
{
    return( ischildcase( child, parent, 1 ));
}
