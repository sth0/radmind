#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <utime.h>
#include <errno.h>

#include "snet.h"
#include "argcargv.h"

    static char
t_convert( int type )
{
    switch( type ) {
    case S_IFREG:
	return ( 'f' );
    case S_IFDIR:
	return ( 'd' );
    case S_IFLNK:
	return ( 'l' );
    case S_IFCHR:
	return ( 'c' );
    case S_IFBLK:
	return ( 'b' );
#ifdef SOLARIS
    case S_IFDOOR:
	return ( 'D' );
#endif
    case S_IFIFO:
	return ( 'p' );
    case S_IFSOCK:
	return ( 's' );
    default:
	return ( 0 );
    }
}

    static int
ischild( const char *parent, const char *child)
{
    if ( parent == NULL ) {
	printf( "  in root?" );
	return 1;
    } else {
	printf( "  %s a child of %s?", child, parent );
	return !strncmp(parent, child, strlen( parent ) );
    }
}

    void
apply( FILE *f, char *parent )
{
    char		tline[ 2 * MAXPATHLEN ];
    int			tac, targvind, present;
    char		**targv, dir[ MAXPATHLEN ];
    char		type;
    extern char		*optarg;
    struct stat		st;
    mode_t	   	mode;
    struct utimbuf	times;
    uid_t		uid;
    gid_t		gid;

    if ( parent != NULL ) {
	printf( "\nWorking in %s\n", parent );
    } else {
	printf( "\nWorking in root\n" );
    }

    while ( fgets( tline, MAXPATHLEN, f ) != NULL ) {

	printf( "%s", tline );

	tac = argcargv( tline, &targv );

	/* check buffer overflow */

	if ( tac == 1 ) {
	    printf( "Command file: %s\n", targv[ 0 ] );
	} else {

	    /* Get argument offset */

	    if ( ( *targv[ 0 ] == '+' ) || ( *targv[ 0 ] == '-' ) ) {
		targvind = 1;
	    } else {
		targvind = 0;
	    }

	    /* Do type check on local file */

	    if ( lstat( targv[ 1 + targvind ], &st ) ==  0 ) {
	        type = t_convert ( S_IFMT & st.st_mode );
		present = 1;
	    } else if ( errno != ENOENT ) { 
		perror( targv[ 1 + targvind ] );
		goto done;
	    } else {
		present = 0;
	    }

	    if ( type != *targv[ 0 + targvind ] && present ) {
		printf( "Must remove %c %s\n", type, targv[ 1 + targvind ] );

		if ( type == 'd' ) {
		    printf( "Calling apply to handle dir\n" );

		    /* Recurse */

		    strcpy( &dir, targv[ 1 + targvind ] );

		    apply( f, targv[ 1 + targvind ] );

		    /* Directory is empty */

		    if ( rmdir( dir ) != 0 ) {
			perror( dir );
			goto done;
		    }

		    present = 0;

		} else {
		    printf( "Unlinking %s for update\n", targv[ 1 + targvind ] );
		    if ( unlink( targv[ 1 + targvind ] ) != 0 ) {
			perror( targv[ 1 + targvind ] );
			goto done;
		    }

		    prsent = 0;
		}
	    }

	    /* Do transcript line */

	    switch( *targv[ 0 ] ) {

	    case '+':

		/* DOWNLOAD */

		printf( "Have to download something\n" );
		break;

	    case '-':
		
		/* DELTE */

		if ( type == 'd' ) {
		    printf( "Calling apply to handle dir\n" );

		    /* Recurse */
		    
		    strcpy( &dir, targv[ 2 ] );

		    apply( f, targv[ 2 ] );

		    /* Directory is empty */

		    if ( rmdir( dir ) != 0 ) {
			perror( dir );
			goto done;
		    }
		} else {
		    printf( "Unlinking %s\n", targv[ 2 ] ); 
		    if ( unlink( targv[ 2 ] ) != 0 ) {
			perror( targv[ 2 ] );
			goto done;
		    }
		}
		break;

	    default:

		/* UPDATE */

		printf( "Updating %s...", targv[ 1 + targvind ] );

		switch ( *targv[ 0 ] ) {
		case 'f':

		    mode = strtol( targv[ 2 ], (char **)NULL, 8 );
		    uid = atoi( targv[ 3 ] );
		    gid = atoi( targv[ 4 ] );
		    times.modtime = atoi( targv[ 5 ] );

		    /* check mode */
		    if( mode != st.st_mode ) {
			printf( "  mode -> %o\n", mode );
			if ( chmod( targv[ 1 ], mode ) != 0 ) {
			    perror( "targv[ 1 ]" );
			    goto done;
			}
		    }

		    /* check uid & gid */
		    if( uid != st.st_uid  || gid != st.st_gid ) {
			if ( uid != st.st_uid ) printf( "  uid -> %i\n", (int)uid );
			if ( gid != st.st_gid ) printf( "  gid -> %i\n", (int)gid );
			if ( lchown( targv[ 1 ], uid, gid ) != 0 ) {
			    perror( targv[ 1 ] );
			    goto done;
			}
		    }

		    /* check modification time */
		    if( times.modtime != st.st_mtime ) {
			printf( "%s time -> %i\n", targv[ 1 ], (int)times.modtime );
			times.actime = st.st_atime;
			if ( utime( targv[ 1 ], &times ) != 0 ) {
			    perror( targv[ 1 ] );
			    goto done;
			}
		    }
		    break;

		case 'd':

		    mode = strtol( targv[ 2 ], (char **)NULL, 8 );
		    uid = atoi( targv[ 3 ] );
		    gid = atoi( targv[ 4 ] );

		    if ( !present ) {
			if ( mkdir( targv[ 1 ], mode ) != 0 ) {
			    perror( targv[ 1 ] );
			    goto done;
			}

			if ( lchown( targv[ 1 ], uid, gid ) != 0 ) {
			    perror( "lchown" );
			    goto done;
			}
		    } else {

			/* check mode */
			if( mode != st.st_mode ) {
			    printf( "  mode -> %o\n", mode );
			    if ( chmod( targv[ 1 ], mode ) != 0 ) {
				perror( "chmod" );
				goto done;
			    }
			}

			/* check uid & gid */
			if( uid != st.st_uid  || gid != st.st_gid ) {
			    if ( uid != st.st_uid ) printf( "  uid -> %i\n", (int)uid );
			    if ( gid != st.st_gid ) printf( "  gid -> %i\n", (int)gid );
			    if ( lchown( targv[ 1 ], uid, gid ) != 0 ) {
				perror( "lchown" );
				goto done;
			    }
			}
		    }
		    printf( "  DONE\n" );
		    break;

		case 'h':

		    /* would we ever get into this if?*/

		    if ( present ) {
			if ( unlink( targv[ 1 ] ) != 0 ) {
			    perror( "unlink" );
			    goto done;
			}
		    }

		    if ( link( targv[ 2 ], targv[ 1 ] ) != 0 ) {
			perror( "link" );
			goto done;
		    }
		    break;

		case 'l':

		    /* would we ever get into this if?*/

		    if ( unlink( targv[ 1 ] ) != 0 ) {
			perror( "unlink" );
			goto done;
		    }
		    
		    if ( symlink( targv[ 2 ], targv[ 1 ] ) != 0 ) {
			perror( "symlink" );
			goto done;
		    }
		    break;

		case 'D':
		    if ( type != 'D' ) {
			printf( "Door update to non-door\n" ); 
		    }
		    break;
		case 's':
		    if ( type != 's' ) {
			printf( "socket update to non-socket\n" ); 
		    }
		    break;
		case 'p':
		    if ( type != 'p' ) {
			printf( "Named pipe update to non-named pipe\n" ); 
		    }
		    break;
		case 'b':
		    if ( type != 'b' ) {
			printf( "Blk special update to non-blk special\n" ); 
		    }
		    break;
		case 'c':
		    if ( type != 'c' ) {
			printf( "Char special update to non-char special\n" ); 
		    }
		    break;
		default :
		    printf( "Unkown type %s to update\n", targv[ 1 ] );
		    break;
		}
		break;  // End of defualt switch
	    }

	    /* check for child */

	    printf( "Checking child\n" );

	    if ( ! ischild( parent, targv[ 1 + targvind ] ) ) {
		printf( "  NO!\n" );
		return;
	    }
	    printf( "  YES!\n" );
	}

    }

    return;

done:

    fclose( f );

    exit( 1 );
}

    int
main( int argc, char **argv )
{
    FILE		*f; 

    if (( f = fopen( argv[ 1 ], "r" )) == NULL ) { 
	perror( argv[ 1 ] );
	exit( 1 );
    }

    apply( f, NULL );

    exit( 0 );
}
