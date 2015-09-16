
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <alloca.h>
#include <unistd.h>
#include <stdlib.h>

#include <uci.h>


void  triptailspace( char * str )
{
    int  ofs;

    /**/
    ofs = strlen( str );
    ofs -= 1;

    /**/
    while( ofs >= 0 )
    {
        if ( (str[ofs] == '\n') || (str[ofs] == '\r') )
        {
            str[ofs] = '\0';
        }
        else if ( (str[ofs] == 0x20) || (str[ofs] == 0x09) )
        {
            str[ofs] = '\0';
        }
        else
        {
            break;
        }

        /**/
        ofs -= 1;
    }

    return;
    
}


int  ugth_uci_getaddr( char * addr )
{
    struct uci_context * uci;
    struct uci_package * upkg;
    struct uci_element * uele;
    struct uci_section * usec;
    const char * ptr;
    int iret;
    

    /**/
    uci = uci_alloc_context();
    iret = uci_load( uci, "jyconfig", &upkg );
    if ( 0 != iret )
    {
        uci_free_context( uci );
		return 1;
    }
    
    /**/
    iret = 9;
    uci_foreach_element( &upkg->sections, uele )
    {
        usec = uci_to_section( uele );
        if( 0 != strcmp( usec->type, "webserver") )
        {
            continue;
        }

        /**/
        ptr = uci_lookup_option_string( uci, usec, "ip" );
        if ( NULL == ptr )
        {
            iret = 2;
            break;
        }

        if ( strlen(ptr) > 16 )
        {
            iret = 3;
            break;
        }

        /**/
        strcpy( addr, ptr );

        /**/
	    ptr = uci_lookup_option_string( uci, usec, "port" );
        if ( NULL == ptr )
        {
            iret = 6;
            break;
        }
        
        /**/
        strcat( addr, ":" );
        strcat( addr, ptr );
        printf( "url: %s\n", addr );
        iret = 0;
        break;
    }

    uci_free_context( uci );
    return iret;
    
}



int  ugth_download( char * fname, char * mname )
{
    int  iret;
    char  addr[100];
    char * cmdn;

    /**/
    iret = ugth_uci_getaddr( addr );
    if ( 0 != iret )
    {
        return 1;
    }
    
    /**/
    cmdn = (char *)alloca( 200 + strlen(fname) );
    if ( NULL == cmdn )
    {
        return 2;
    }

    /**/
    chdir( "/tmp" );
    
    /**/
    sprintf( cmdn, "rm -f  ./%s", fname );
    system( cmdn );
    
    sprintf( cmdn, "rm -f  ./%s", mname );
    system( cmdn );

    /**/
    sprintf( cmdn, "wget -q http://%s/%s", addr, fname );
    system( cmdn );
    sprintf( cmdn, "wget -q http://%s/%s", addr, mname );
    system( cmdn );
    return 0;
    
}


int  ugth_checksum( char * fname, char * mname )
{
    int  tlen;
    char * cmdn;
    char * ptr;    
    FILE * fp;
    
    /**/
    tlen = 200 + strlen(fname);
    cmdn = (char *)alloca( tlen );
    if ( NULL == cmdn )
    {
        return 1;
    }
    
    /**/
    sprintf( cmdn, "md5sum -c ./%s", mname );
    fp = popen( cmdn, "r" );
    if ( NULL == fp )
    {
        return 2;
    }

    /* we26n.bin: OK */
    ptr = cmdn;    
    memset( ptr, 0, tlen );
    fgets( ptr, tlen, fp );
    fclose( fp );
    
    /**/
    printf( "gets: %s\n", ptr );
    tlen = strlen( fname );
    if ( 0 != strncmp( ptr, fname, tlen) )
    {
        return 3;
    }

    /**/
    ptr += tlen;
    if ( 0 != strcmp( ptr, ": OK\n" ) )
    {
        return 3;
    }

    return 0;
    
}


int  ugth_upgrade( char * fname )
{
    char * cmdn;

    /**/
    cmdn = (char *)alloca( 200 + strlen(fname) );
    if ( NULL == cmdn )
    {
        return 1;
    }
    
    chdir( "/tmp" );
    printf( "begin upgrade\n" );

    /**/
    sprintf( cmdn, "sysupgrade ./%s", fname );
    system( cmdn );
    
    return 0;
    
}


void * ugth_thread( void * arg )
{
    int  iret;
    char * fname;
    char * mname;

    /**/
    fname = (char *)arg;
    mname = malloc( strlen(fname) + 10 );
    strcpy( mname, fname );
    strcat( mname, ".md5.txt" );

    /**/
    do {
        iret = ugth_download( fname, mname );
        if ( 0 != iret )
        {
            printf( "ugth_download, ret, %d\n", iret );
            break;
        }

        iret = ugth_checksum( fname, mname );
        if ( 0 != iret )
        {
            printf( "ugth_checksum, ret, %d\n", iret );
            break;
        }
        
        iret = ugth_upgrade( fname );
        if ( 0 != iret )
        {
            printf( "ugth_upgrade, ret, %d\n", iret );
            break;
        }
        
    } while( 0 );
    
    /**/
    free( mname );
    free( fname );
    return NULL;
    
}


/* http://stackoverflow.com/questions/2304221/what-character-sequence-should-i-not-allow-in-a-filename */
int  ugth_namechk( char * bname )
{
    int  count;
    int  tlen;
    char * patern = "abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789" "_.-";
    
    /**/
    tlen = strlen( bname );
    count = strspn( bname, patern );
    if ( tlen != count )
    {
        return 1;
    }

    /**/
    if ( bname[0] == '.' )
    {
        return 2;
    }

    /**/
    if ( bname[0] == '-' )
    {
        return 3;
    }
    
    return 0;
    
}


int  ugth_start( char * bname )
{
    int  iret;
    pthread_t  pid;
    char * ptr;

    /* check */
    iret = ugth_namechk( bname );
    if ( 0 != iret )
    {
        printf( "name check fail, %d\n", iret );
        printf( "%s\n", bname );
        return 1;
    }
    
    /**/
    ptr = strdup( bname );
    if ( NULL == ptr )
    {
        return 2;
    }

    /**/
    iret = pthread_create( &pid, NULL, ugth_thread, (void *)ptr );
    if ( 0 != iret )
    {
        return 3;
    }

    /**/
    return 0;
    
}


