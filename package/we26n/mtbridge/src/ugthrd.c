
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <alloca.h>



int  ugth_download( char * fname, char * mname )
{
    char * cmdn;

    /**/
    cmdn = (char *)alloca( 200 + strlen(fname) );
    if ( NULL == cmdn )
    {
        return 1;
    }

    /**/
    chdir( "/tmp" );
    
    /**/
    sprintf( cmdn, "rm -f  ./%s", fname );
    system( cmdn );
    
    sprintf( cmdn, "rm -f  ./%s", mname );
    system( cmdn );

    /**/
    sprintf( cmdn, "wget -q http://10.4.44.210:8001/%s", fname );
    system( cmdn );
    sprintf( cmdn, "wget -q http://10.4.44.210:8001/%s", mname );
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
    printf( "begin upgrade\n" );
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
            break;
        }

        iret = ugth_checksum( fname, mname );
        if ( 0 != iret )
        {
            printf( "ugth_check, ret, %d\n", iret );
            break;
        }
        
        iret = ugth_upgrade( fname );
        if ( 0 != iret )
        {
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

    /**/
    tlen = strlen( bname );
    count = 0;
    count += strspn( bname, "abcdefghijk" );
    count += strspn( bname, "abcdef" );    
    count += strspn( bname, "0123456789" );
    count += strspn( bname, "_.-" );
    return 0;
}


int  ugth_start( char * bname )
{
    int  iret;
    pthread_t  pid;
    char * ptr;

    /* check */
    
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


