/*--------------------------------------------
    Purpose: General purpose utilities library

    Author:  M.A. O'Neill
             Tumbling Dice Ltd
             Gosforth
             Newcastle upon Tyne
             NE3 4RT
             United Kingdom

    Version: 8.11 
    Dated:   2nd January 2025 
    E-Mail:  mao@tumblingdice.co.uk
--------------------------------------------*/


#include <me.h>
#include <ctype.h>
#include <signal.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <dirent.h>
#include <string.h>
#include <bsd/string.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <pwd.h>
#include <sched.h>
#include <stdarg.h>
#include <sys/timeb.h>
#include <xtypes.h>
#include <poll.h>
#include <syscall.h>
#include <zlib.h>
#include <cache.h>


/*-------------------------------------------------------------*/
/* If we have OpenMP support then we also have pthread support */
/* (OpenMP is built on pthreads for Linux distributions).      */
/*-------------------------------------------------------------*/

#if defined(_OPENMP) && !defined(PTHREAD_SUPPORT)
#define PTHREAD_SUPPORT
#endif /* PTHREAD_SUPPORT */


#ifdef PTHREAD_SUPPORT
#include <tad.h>
#endif /* PTHREAD_SUPPORT */

#ifdef SHADOW_SUPPORT
#include <shadow.h>
#endif /* SHADOW_SUPPORT */

#include <termios.h>
#include <netdb.h> 
#include <sys/ioctl.h>
#include <time.h>


#ifdef DLL_SUPPORT
#include <dll.h>
#endif /* DLL_SUPPORT */

#ifdef PERSISTENT_HEAP_SUPPORT
#include <pheap.h>
#endif /* PERSISTENT_HEAP_SUPPORT */

#include <sched.h>
#include <sys/vfs.h>
#include <stdlib.h>
#include <psrp.h>
#include <casino.h>


#ifndef __NOT_LIB_SOURCE__
#define __UTILIB__
#endif /* __NOT_LIB_SOURCE__ */


/*----------------------------------------------------------------*/
/* Get signal mapping appropriate to OS and hardware architecture */
/*----------------------------------------------------------------*/

#define __DEFINE__
#if defined(I386) || defined(X86_64)
#include <sig.linux.x86.h>
#endif /* I386) || X86_64 */

#ifdef ARMV6L
#include <sig.linux.arm.h>
#endif /* ARMV6L */

#ifdef ARMV7L
#include <sig.linux.arm.h>
#endif /* ARMV7L */

#ifdef AARCH64
#include <sig.linux.arm.h>
#endif /* AARCH64 */
#undef __DEFINE__

#undef   __NOT_LIB_SOURCE__
#include <utils.h>
#define  __NOT_LIB_SOURCE__

#include <netlib.h>


/*---------------------------------------------*/
/* Pipe migration protocol (PMP) verson number */
/*---------------------------------------------*/

#define FSHP_VERSION "1.00"


/*-----------------------------------------*/
/* Embedded window resizing data in buffer */
/*-----------------------------------------*/

#define WINRESIZE_PENDING    255


/*------------------------------*/
/* ISOFS_SUPER_MAGIC definition */
/*------------------------------*/

#define ISOFS_SUPER_MAGIC    0x9660
#define NFS_SUPER_MAGIC      0x6969


/*-------------------------*/
/* Handle a broken va_list */
/*-------------------------*/

#ifdef VA_BROKEN
int __builtin_va_alist;
#endif /* VA_BROKEN */


/*----------------------------------------*/
/* Variables imported by this application */
/*----------------------------------------*/


_IMPORT _BOOLEAN psrp_reactivate_client;
_IMPORT _BOOLEAN in_psrp_new_segment;
_IMPORT int32_t  psrp_seg_cnt;
_IMPORT int32_t  chlockdes;
_IMPORT int32_t  c_client;
_IMPORT pid_t    psrp_client_pid[MAX_CLIENTS];



/*---------------------------------------*/
/* Standard globals required by std_init */
/*---------------------------------------*/

_PUBLIC char      boldOn [8]  = "\e[1m",                                                // Make character bold
                  boldOff[8]  = "\e[m";                                                 // Macke character non-bold
 
_PUBLIC pid_t     appl_pid,                                                             // Process id
                  appl_ppid,                                                            // Parent process
                  appl_remote_pid,                                                      // Remote PID to relay signals to 
	          appl_softdog_pid            = (-1),                                   // Software watchdog process PID
                  appl_sid;                                                             // Session (process group) id
       
_PUBLIC uid_t     appl_uid;                                                             // Process UID
_PUBLIC gid_t     appl_gid;                                                             // Process GID

_PUBLIC  int32_t  ptr,                                                                   // Argument pointer
                  sargc,                                                                 // Argument count
                  t_args,                                                                // Total number of arguments
                  vitimer_quantum,                                                       // Time interval for (PSRP) timers
                  pupshold_cnt                  = 0,                                     // Number of PUPS/P3 signals held
	          appl_fsa_mode,                                                         // Filesystem access mode
                  appl_t_args,                                                           // Total application command tail arguments
                  appl_alloc_opt,                                                        // Allocation options (for pups_malloc etc.)
                  appl_max_files,                                                        // Maximum number of file table slots
                  appl_max_child,                                                        // Maximum number of child table slots
                  appl_max_pheaps,                                                       // Maximum number of persistent heap table slots
                  appl_max_vtimers,                                                      // Maximum number of virtual timers
                  appl_max_orifices,                                                     // Maximum number of orifice table slots
                  appl_vtag,                                                             // Version ID tag  for application
                  appl_last_child,                                                       // PID of last child forked by pups_system

                  #ifdef CRIU_SUPPORT
                  appl_criu_ssaves             = 0,                                      // Number of times state has been saved
                  appl_criu_poll_time          = DEFAULT_CRIU_POLL_TIME,                 // Poll time for (Criu) state saving
                  #endif /* CRIU_SUPPORT */


                  #ifdef _OPENMP
                  appl_omp_threads,                                                      // Number of OMP threads
                  #endif /* _OPENMP */

                  appl_tty                     = (-1),                                   // Applications controlling terminal
                  appl_timestamp,                                                        // Application compilation time stamp
                  appl_nice_lvl                = 10,                                     // Process niceness
                  appl_softdog_timeout         = DEFAULT_SOFTDOG_TIMEOUT;                // Software watchdog timeout


#ifdef PTHREAD_SUPPORT
_PUBLIC pthread_t appl_root_tid                = (pthread_t)NULL;                        // Root (initial) thread for process 
#endif /* PTHREAD_SUPPORT */

_PUBLIC   int32_t pupsighold_cnt[MAX_SIGS]     = { [0 ... MAX_SIGS-1] = 1 };             // Counter for signal states

_PUBLIC _BOOLEAN  appl_verbose                     = FALSE,                              // Set appl_verbose reporting mode
                  appl_resident                    = FALSE,                              // TRUE if application is memory resident
                  appl_enable_resident             = FALSE,                              // TRUE if application can be memory resident
                  appl_proprietary                 = FALSE,                              // TRUE if application is proprietary
                  appl_nodetach                    = FALSE,                              // Don't detach stdio in background
                  test_mode                        = FALSE,                              // Set test mode
                  init                             = TRUE,                               // Initialise tail decoder system
                  appl_pg_leader                   = FALSE,                              // Application is process group leader
                  appl_wait                        = FALSE,                              // TRUE if process stopped
                  appl_fgnd                        = TRUE,                               // TRUE if in foreground pgrp
                  appl_snames_crypted              = FALSE,                              // If TRUE encrypt shadow files
	          appl_softdog_enabled             = FALSE,                              // TRUE if software watchdog enabled
                  argd[255];                                                             // Argument decode status flags

#ifdef CRIU_SUPPORT
                  appl_criu_ssave                  = FALSE;                              // TRUE if (Criu) state saving enabled

_PUBLIC char      appl_criu_ssave_dir[SSIZE]       = "";                                 // Criu checkpoint directory for state saving
_PUBLIC char      appl_criu_dir[SSIZE]             = "/tmp";                             // Criu directory (holds migratable files and checkpoint directories)
#endif /* CRIU_SUPPORT */


#ifdef SSH_SUPPORT
_PUBLIC char      ssh_remote_port[SSIZE]           = "22";                               // Port for remote node
_PUBLIC char      ssh_remote_uname[SSIZE]          = "";                                 // Username for remote node
_PUBLIC _BOOLEAN  ssh_compression                  = FALSE;                              // Use compressed ssh network tunneling
#endif /* SSH_SUPPORT */

#ifdef MAIL_SUPPORT
_PUBLIC _BOOLEAN appl_mailable                     = FALSE;                              // TRUE if process supports mail
#endif /* MAIL_SUPPORT */

_PUBLIC _BOOLEAN  appl_secure                      = FALSE;                              // TRUE if application secure
_PUBLIC _BOOLEAN  appl_kill_pg                     = FALSE;                              // If TRUE kill process group on exit
_PUBLIC _BOOLEAN  appl_default_chname              = TRUE;                               // TRUE if PSRP channel name default
_PUBLIC _BOOLEAN  appl_have_pen                    = FALSE;                              // TRUE if binname != execution name
_PUBLIC _BOOLEAN  appl_psrp                        = FALSE;                              // TRUE if application PSRP enabled
_PUBLIC _BOOLEAN  appl_psrp_load                   = TRUE;                               // If TRUE load PSRP resources at start
_PUBLIC _BOOLEAN  appl_psrp_save                   = FALSE;                              // If TRUE save dispatch table at exit
_PUBLIC _BOOLEAN  appl_etrap                       = FALSE;                              // If TRUE trap-wait in pups_exit()
_PUBLIC _BOOLEAN  appl_ppid_exit                   = FALSE;                              // If TRUE exit if parent terminates
_PUBLIC _BOOLEAN  appl_rooted                      = FALSE;                              // If TRUE system context cannot migrate
_PUBLIC _BOOLEAN  pups_process_homeostat           = FALSE;                              // TRUE if process homeostat enabled 
_PUBLIC _BOOLEAN  ignore_pups_signals              = TRUE;                               // Ignore PUPS signals if TRUE
_PUBLIC _BOOLEAN  pups_abort_restart               = FALSE;                              // TRUE if abort handler enabled
_PUBLIC _BOOLEAN  in_pups_exit                     = FALSE;                              // TRUE if in pups_exit()
_PUBLIC _BOOLEAN  in_vt_handler                    = FALSE;                              // TRUE if in vt_handler()

_PUBLIC  sigjmp_buf pups_restart_buf;                                                    // Abort restart buffer


_PUBLIC _BOOLEAN   ftab_extend                     = TRUE;                               // TRUE if extening file table
_PUBLIC ftab_type  *ftab                           = (ftab_type *)NULL;                  // PUPS file table
_PUBLIC chtab_type *chtab                          = (chtab_type *)NULL;                 // PUPS child (process) table
_PUBLIC sigtab_type sigtab[MAX_SIGS];                                                    // Addresses of signal handlers
_PUBLIC vttab_type *vttab                          = (vttab_type *)NULL;                 // Virtual timer table


_PUBLIC char      date[SSIZE]                      = "";                                 // Date stamp
_PUBLIC char      errstr[SSIZE]                    = "";                                 // Error string
_PUBLIC char      appl_machid[SSIZE]               = "";                                 // Unique machine (host) i.d.

_PUBLIC char      version[SSIZE]                   = "",                                 // Version of the code
                  appl_owner[SSIZE]                = "",                                 // Owner of this application
                  appl_password[SSIZE]             = "",                                 // Application owners password
                  appl_crypted[SSIZE]              = "",                                 // Application encrypted owners password
                  appl_name[SSIZE]                 = "",                                 // Name of application
                  appl_remote_host[SSIZE]          = "",                                 // Name of remote host for signal relay
                  appl_fifo_dir[SSIZE]             = "",                                 // Name of default FIFO patchboard
                  appl_ch_name[SSIZE]              = "",                                 // Name of application PSRP channel
                  appl_logfile[SSIZE]              = "",                                 // Error/log file

                  #ifdef MAIL_SUPPORT
                  appl_mdir[]                      = "",                                 // MH inbox for this process
                  appl_mh_folder[]                 = "",                                 // MH folder for this process
                  appl_mime_dir[]                  = "",                                 // MIME message parts workspace
                  appl_mime_type[]                 = "",                                 // MIME message type
                  appl_replyto[]                   = "",                                 // MH reply address
                  #endif /* MAIL_SUPPORT */

                  appl_tunnel_path[SSIZE]          = "",                                 // Pathname for process tunnel
                  appl_bin_name[SSIZE]             = "",                                 // Name of process binary
                  appl_ttyname[SSIZE]              = "",                                 // Name of process controlling terminal
                  appl_pam_name[SSIZE]             = "",                                 // Name of PSRP authentication module
                  author[SSIZE]                    = "",                                 // Author name
                  revdate[SSIZE]                   = "",                                 // Revision date
                  arg_f_name[SSIZE]                = "",                                 // Name of comamnd tail argument file
	          appl_home[SSIZE]                 = "",                                 // Effective home directory
                  appl_cwd[SSIZE]                  = "",                                 // Current working directory
                  appl_cmd_str[SSIZE]              = "",                                 // Application command string
                  appl_host[SSIZE]                 = "",                                 // Application host processor node
                  appl_state[SSIZE]                = "",                                 // Application state information
                  appl_err[SSIZE]                  = "",                                 // Application error information
                  appl_argfifo[SSIZE]              = "",                                 // Argument FIFO (for memory resident application)
                  *args[ARGS]                      = { [0 ... ARGS-1] = (char *)NULL };  // Secondary argument vector

_PUBLIC  int32_t  (*appl_mail_handler)(char *);                                          // Application specific mail handler


/*----------------------------------------*/
/* Private variables used by this routine */
/*----------------------------------------*/
/*------------------------------*/
/* Homeostatis status for stdio */
/*------------------------------*/

_PRIVATE  int32_t in_state   = NONE;
_PRIVATE  int32_t pin_state  = NONE;
_PRIVATE  int32_t out_state  = NONE;
_PRIVATE  int32_t pout_state = NONE;
_PRIVATE  int32_t err_state  = NONE;
_PRIVATE  int32_t perr_state = NONE;


/*----------------------------------------------------------*/
/* Error handler parameters (preset to PUPS default values) */
/*----------------------------------------------------------*/

_PRIVATE  int32_t err_action   = PRINT_ERROR_STRING | EXIT_ON_ERROR;           // Default error action
_PRIVATE  int32_t err_code     = (-2);                                         // PUPS/P3 error code
_PRIVATE  FILE *err_stream  = (FILE *)NULL;                                    // Error/status log stream
_PRIVATE  FILE *fgnd_stdin  = (FILE *)NULL;                                    // Foreground stream (standard input)
_PRIVATE  FILE *fgnd_stdout = (FILE *)NULL;                                    // Foreground stream (standard ouput)
_PRIVATE  FILE *fgnd_stderr = (FILE *)NULL;                                    // Foreground stream (standard error)


_PRIVATE _BOOLEAN in_setvitimer     = FALSE;                                   // TRUE if in setvitimer
_PRIVATE _BOOLEAN no_vt_services    = FALSE;                                   // TRUE if VT services disabled
_PRIVATE _BOOLEAN do_closeall       = FALSE;                                   // TRUE if closeall entered
_PRIVATE _BOOLEAN started_detached  = FALSE;                                   // TRUE if application has been detached
_PRIVATE _BOOLEAN default_argfile   = FALSE;                                   // TRUE if default argument file
_PRIVATE _BOOLEAN in_close_routine  = FALSE;                                   // TRUE if in pups_close() or pups_fclose()
_PRIVATE _BOOLEAN jump_vector       = TRUE;                                    // Context OK for longjmp() if TRUE



/*---------------------------------------------------*/
/* Private variables used by virtual interval timers */
/*---------------------------------------------------*/

_PRIVATE int32_t       start_index      = 0;
_PRIVATE int32_t       active_v_timers  = 0;
_PRIVATE _BOOLEAN  vt_no_reset      = FALSE;


/*---------------------------------------*/
/* Private variables used by child table */
/*---------------------------------------*/

_PRIVATE  int32_t       n_children       = 0;


/*----------------------*/
/* Checkpoint file name */
/*----------------------*/

_PRIVATE char ckpt_file_name[SSIZE] = "";



#ifdef PTHREAD_SUPPORT
/*------------------------------*/
/* Mutexes used by this library */
/*------------------------------*/

// Thread safe pups_fork mutex 
_PUBLIC pthread_mutex_t pups_fork_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// Thread safe file table mutex
_PUBLIC pthread_mutex_t ftab_mutex      = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// Thread safe child table mutex
_PUBLIC pthread_mutex_t chtab_mutex     = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// Thread safe child table mutex
_PRIVATE pthread_mutex_t sigtab_mutex   = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// Thread safe lock table mutex
_PRIVATE pthread_mutex_t lock_mutex     = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// Thread safe vttimer mutex
_PRIVATE pthread_mutex_t malarm_mutex   = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// Thread safe date mutex
_PRIVATE pthread_mutex_t date_mutex     = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// Thread error handler mutex
_PRIVATE pthread_mutex_t errstr_mutex   = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// File copy mutex
_PRIVATE pthread_mutex_t copy_mutex     = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

#endif /* PTHREAD_SUPPORT */



/*-------------------------------------------------*/
/* Slot and usage functions - used by slot manager */
/*-------------------------------------------------*/
/*---------------*/
/* Slot function */
/*---------------*/

_PRIVATE void utilib_slot(const int32_t level)
{   (void)fprintf(stderr,"int lib utilib %s: [ANSI C]\n",UTILIB_VERSION);

    if(level > 1)
    {  (void)fprintf(stderr,"(C) 1985-2025 Tumbling Dice\n");
       (void)fprintf(stderr,"Author: M.A. O'Neill\n");
       (void)fprintf(stderr,"PUPS/P3 general purpose utilities library (gcc %s: built %s %s)\n\n",__VERSION__,__TIME__,__DATE__);
    }
    else
       (void)fprintf(stderr,"\n");
    (void)fflush(stderr);
}


/*----------------*/
/* Usage function */
/*----------------*/

_PRIVATE void utilib_usage(void)
{   (void)fprintf(stderr,"\nGeneric PUPS/P3 options\n");  
    (void)fprintf(stderr,"=======================\n\n");
    (void)fprintf(stderr,"[-mra [-cpu <cpu available:%f>] [-mem <mem required:%d> [-timeout <hours:%f>]\n",
                                      PUPS_DEFAULT_MIN_CPU,PUPS_DEFAULT_MIN_MEM,PUPS_DEFAULT_RESOURCE_WAIT);

    (void)fprintf(stderr,"[-etrap:FALSE]   [-version:FALSE] [-vtag] [-usage:FALSE] [-verbose:FALSE [-test:FALSE]]\n");
    (void)fprintf(stderr,"[-argf <argfile>] [-slots] [-slotinfo] [-secure:FALSE]\n");

#ifdef SSH_SUPPORT
    (void)fprintf(stderr,"[-on <host> [-bg:FALSE]]\n");
#endif /* SSH_SUPPORT */

#ifdef MAIL_SUPPORT
    (void)fprintf(stderr,"[-mail_dir <mail directory>]\n");
#endif /* MAIL_SUPPORT */

#ifndef SINGLE_THREAD
#ifdef _OPENMP
    (void)fprintf(stderr,"[-omp_threads]\n");
#endif /* _OPENMP */
#endif /* SINGLE_THREAD */

    if(appl_enable_resident == TRUE)
       (void)fprintf(stderr,"[-resident <command argument FIFO>]\n");

    (void)fprintf(stderr,"[-noseed:FALSE]\n");
    (void)fprintf(stderr,"[-parent <parent pid:%d | parent name>]\n",appl_ppid);
    (void)fprintf(stderr,"[-parent_exit:FALSE]\n");
    (void)fprintf(stderr,"[-closestdio]\n");
    (void)fprintf(stderr,"[-tunnel]\n");
    (void)fprintf(stderr,"[-log]\n");
    (void)fprintf(stderr,"[-cwd <working directory>]\n");
    (void)fprintf(stderr,"[-growth_protection] <retrys> [-nice <lvl: %d>]\n",appl_nice_lvl);
    (void)fprintf(stderr,"[-pen <process execution name:application name>]\n");
    (void)fprintf(stderr,"[-bin <application exec binary:pen defaulted>]\n");
    (void)fprintf(stderr,"[-in | -inalive | -pin | -pinalive <stdin redirection stream>]\n");
    (void)fprintf(stderr,"[-out | -outalive | -pout | -poutalive <stdout redirection stream>]\n");
    (void)fprintf(stderr,"[-err | -erralive | -perr | -perralive <stderr redirection stream>]\n");
    (void)fprintf(stderr,"[-patchboard <local or private PSRP patchboard:/tmp>]\n");
    (void)fprintf(stderr,"[-no_vt_services:FALSE]\n");
    (void)fprintf(stderr,"[-pg_leader] | [-session_leader]\n");
    (void)fprintf(stderr,"[-kill_pg:FALSE]\n");
    (void)fprintf(stderr,"[-pgrp <process group leader pid>]\n");
    (void)fprintf(stderr,"[-channel_name <process PSRP channel name>]\n");

    #ifdef SSH_SUPPORT
    (void)fprintf(stderr,"[-ssh_port <port>\n");
    (void)fprintf(stderr,"[-ssh_compress]\n");
    #endif /* SSH_SUPPORT */

    #ifdef CRIU_SUPPORT
    (void)fprintf(stderr,"[-ssave [-sspoll <criu state save poll time seconds:%d>] [-cd <criu directory:/tmp]]\n",appl_poll_time);
    #endif /* CRIU_SUPPORT */

    (void)fprintf(stderr,"[-softdog [-softdog <watchdog timeout seconds:%d>]\n",appl_softdog_timeout);
    (void)fprintf(stderr,"[-stdio_dead]\n");
    (void)fprintf(stderr,"[-pam <PUPS authentication module name>]\n");
    (void)fprintf(stderr,"[-shadows_crypted:FALSE]\n");

    #ifndef SUPPRESS_PSRP_USAGE
    (void)fprintf(stderr,"[-psrp_autoload:FALSEE]\n");
    (void)fprintf(stderr,"[-psrp_autosave:FALSE]\n");
    #endif /* SUPPRESS_PSRP_USAGE */

    (void)fprintf(stderr,"[-vitab <max virtual timers:32>]\n");
    (void)fprintf(stderr,"[-ftab <max files:32>]\n");
    (void)fprintf(stderr,"[-chtab <max children:32>]\n");


    #ifdef DLL_SUPPORT
    (void)fprintf(stderr,"[-ortab <max DLL orifices:32>]\n");
    #endif /* DLL_SUPPORT */


    #ifdef PERSISTENT_HEAP_SUPPORT
    (void)fprintf(stderr,"[-phtab <max persistent heaps:32>]\n");
    #endif /* PERSISTENT_HEAP_SUPPORT */

    (void)fprintf(stderr,"\n\nApplication specific options\n");
    (void)fprintf(stderr,"============================\n\n");
    (void)fflush(stderr);
}


#ifdef SLOT
#include <slotman.h>
_EXTERN void (* SLOT )() __attribute__ ((aligned(16))) = utilib_slot;
_EXTERN void (* USE  )() __attribute__ ((aligned(16))) = utilib_usage;
#endif /* SLOT */


/*------------------------*/
/* Application build date */
/*------------------------*/

_EXTERN char appl_build_time[SSIZE];
_EXTERN char appl_build_date[SSIZE];




/*-------------------*/
/* Application shell */
/*-------------------*/

_PUBLIC char shell[SSIZE] = "";


/*-------------------------------------------------*/
/* Variables which are private to this application */
/*-------------------------------------------------*/


                               /*-----------------------------------*/
_PRIVATE int32_t sbrk_retrys;  /* Maximum number of retrys for sbrk */
                               /*-----------------------------------*/


/*-------------------------------------------------------*/
/* Procedures which are private to the utilities library */
/*-------------------------------------------------------*/

#ifdef CRIU_SUPPORT
// Perioidically save state (via Criu)
_PROTOTYPE _PRIVATE int32_t ssave_homeostat(void *, char *);
#endif /* CRIU_SUPPORT */

//Extract embedded window resizing data from buffer
                                                  /*-----------------------------*/
_PRIVATE _BOOLEAN ws_extract(des_t           ,    /* File descriptor for ibuf    */
                             _BYTE          *,    /* Input buffer                */
                             size_t         *,    /* Bytes in input buffer       */
                             _BYTE          *,    /* Pre resize buffer           */
                             size_t         *,    /* Number of pre resize bytes  */
                             _BYTE          *,    /* Post resize buffer          */
                             size_t         *,    /* Number of post resize bytes */
                             struct winsize *);   /* Winsize structure           */
                                                  /*-----------------------------*/

// Abort restart handler
_PROTOTYPE _PRIVATE int32_t pups_restart_handler(int32_t);


#ifdef CRIU_SUPPORT
// Handler for state saving
_PROTOTYPE _PRIVATE int32_t ssave_handler(int32_t);
#endif /* CRIU_SUPPORT */

// Handler for SIGTTIN/SIGTTOU
_PROTOTYPE _PRIVATE int32_t fgio_handler(int32_t);

// Handler for SIGTTIN/SIGTTOU
_PROTOTYPE _PRIVATE int32_t segbusfpe_handler(int32_t);

// Handler to deal with segmentation violations
_PROTOTYPE _PRIVATE void catch_sigsegv(void);

// Process group leaders handler for SIGTERM
_PROTOTYPE _PRIVATE int32_t pg_leaders_term_handler(int32_t);

// Process group leaders handler for SIGHUP (SIGTSTP)
_PROTOTYPE _PRIVATE int32_t pg_leaders_stop_handler(int32_t);

// Process group leaders handler for SIGCONT
_PROTOTYPE _PRIVATE int32_t pg_leaders_cont_handler(int32_t);

// Handler for SIGCONT (for PUPS processes other than process group leader)
_PROTOTYPE _PRIVATE  int32_t pups_cont_handler(int32_t);


/*------------------------------------------------------*/
/* Handler for SIGQUIT - enables PUPS processes to exec */
/* PUPS exit functions on reciept of SIGQUIT            */
/*------------------------------------------------------*/

_PROTOTYPE _PRIVATE  int32_t pups_exit_handler(int32_t);

// Handler for SIGCHLD
_PROTOTYPE _PRIVATE void chld_handler(int32_t);

// Initialise PUPS virtual interval timers
_PROTOTYPE _PRIVATE void initvitimers(int32_t);

// Initialise PUPS file table
_PROTOTYPE _PRIVATE void initftab(int32_t,  int32_t);

// Initialise signal status
_PROTOTYPE _PRIVATE void initsigstatus(void);

// Hoemeostat which reconnects migrating file system objects
_PROTOTYPE _PRIVATE  int32_t reconnect(int32_t, _BOOLEAN *);

// Initialise PUPS persistent heaps
_PROTOTYPE _PRIVATE void shm_init(void);

// Initialise PUPS heap object tracking system
_PROTOTYPE _PRIVATE void tinit(void);




/*---------------------------------------------------------------*/
/* Make sure correct signal mask is used if we are multithreaded */
/*---------------------------------------------------------------*/

_PUBLIC int32_t pups_sigprocmask(int32_t how,  const sigset_t *restrict set, sigset_t *restrict oset)

{    int32_t ret;

     #ifdef PTHREAD_SUPPORT
     ret = pthread_sigmask(how,set,oset);
     #else
     ret = setprocmask(how,set,oset);
     #endif /* PTHREAD_SUPPORT */

    return(ret);
}




/*----------------------------------------------------------------------*/
/* Get time accurate to milliseconds - this funtcion has to be double   */
/* precision because of the size of the integers in the timeb structure */
/*----------------------------------------------------------------------*/

_PUBLIC double millitime(void)

{    double       ret;
     struct timeb tspec;

     (void)ftime(&tspec);
     ret = (double)tspec.time + (double)tspec.millitm/1000.0;

     return(ret);
}




/*-------------------------------------------------*/
/* Get IPV4 address and node name of Internet host */
/*-------------------------------------------------*/

_PUBLIC int32_t pups_get_ip_info(const char *dev_name, char *ip_addr, char *node_name)
{
    int32_t            fd;
    struct ifreq       ifr;
    struct sockaddr_in sin;
    unsigned char      buf[SSIZE] = "";

    if(dev_name  == (char *)NULL  ||
       ip_addr   == (char *)NULL  ||
       node_name == (char *)NULL   )
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;

    (void)snprintf(ifr.ifr_name, IFNAMSIZ, dev_name);

    if(ioctl(fd, SIOCGIFADDR, &ifr) == (-1))
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*-------------------------------------------*/
    /* Get IPV4 address (in human readable form) */
    /* Need to use inet_ntop for IPV4 and IPV6   */
    /*-------------------------------------------*/

    snprintf(ip_addr,SSIZE, (char *)inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    (void)close(fd);


    /*-----------------------------------------*/
    /* Get network node name (from IP address) */
    /*-----------------------------------------*/

    (void)memset(&sin, 0, sizeof(sin));
    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip_addr);
    sin.sin_port        = 0;

    (void)getnameinfo( (struct sockaddr *)&sin,
                       sizeof(sin),
                       buf,
                       sizeof(buf), NULL, 0, 0);

    (void)strlcpy(node_name,buf,SSIZE);

    pups_set_errno(OK);
    return(0);
}



/*--------------------------------------------------------*/
/* Round a floating point number to N significant figures */
/*--------------------------------------------------------*/

_PUBLIC FTYPE sigfig(const FTYPE arg, const uint32_t digits)

{   FTYPE factor,
          sigfig_arg;

    if(arg == 0.0)
       return(0.0);
    else if(digits == 0)
       return(arg);

    factor     = POW(10.0, digits - CEIL(LOG10(FABS(arg))));
    sigfig_arg = ROUND(arg * factor) / factor;

    return(sigfig_arg);
}



/*----------------------------------------------*/
/* Find the square of a floating point argument */
/*----------------------------------------------*/

_PUBLIC FTYPE sqr(const FTYPE arg)

{   return((FTYPE)arg*arg);
}




/*----------------------------------------*/
/* Swap a pair of short integer arguments */
/*----------------------------------------*/

_PUBLIC void sswap(int16_t *arg_1, int16_t *arg_2)

{   int16_t temp;

    temp   = *arg_1;
    *arg_1 = *arg_2;
    *arg_2 = temp;
}




/*---------------------------------*/
/* Swap a pair of intger arguments */
/*---------------------------------*/

_PUBLIC void iswap(int32_t *arg_1,  int32_t *arg_2)

{    int32_t temp;

    temp   = *arg_1;
    *arg_1 = *arg_2;
    *arg_2 = temp;
}




/*---------------------------------------*/
/* Swap a pair of int64_t eger arguments */
/*---------------------------------------*/

_PUBLIC void lswap(int64_t *arg_1, int64_t *arg_2)

{    int64_t temp;

    temp   = *arg_1;
    *arg_1 = *arg_2;
    *arg_2 = temp;
}



/*-----------------------------------------*/
/* Swap a pair of floating point arguments */
/*-----------------------------------------*/

_PUBLIC void fswap(FTYPE *arg_1, FTYPE *arg_2)

{   FTYPE temp;

    temp   = *arg_1;
    *arg_1 = *arg_2;
    *arg_2 = temp;
}



/*------------------------------------------------------------*/
/* Strip comments of the form {token} comment {token} from an */
/* ASCII text file                                            */
/*------------------------------------------------------------*/

_PUBLIC FILE *pups_strp_commnts(const char token, const FILE *c_file, char *tmp_f_name)

{  uint32_t i;

   char ch,
        *line = (char *)NULL;

   _BOOLEAN content = FALSE,
            looper  = TRUE;

   FILE     *t_file = (FILE *)NULL;

   if(c_file == (FILE *)NULL || tmp_f_name == (char *)NULL)
   {  pups_set_errno(EINVAL);
      return(FILE *)NULL;
   }

   if(access(tmp_f_name,F_OK) == (-1) && pups_creat(tmp_f_name,0600) == (-1))
   {  pups_set_errno(EINVAL);
      return((FILE *)NULL);
   }

   t_file = fopen(tmp_f_name,"w+");
   line   = pups_malloc(SSIZE);


   /*---------------------------------------------------------*/
   /* If line is prefixed with a '#' character, skip the line */ 
   /*---------------------------------------------------------*/

   do  {   (void)fgets(line,SSIZE,c_file);

           if(feof(c_file) != 0)
              looper = FALSE;
           else
           {  content = FALSE;


              /*---------------------------------------*/
              /* If line only contains carriage return */
              /* discard it.                           */
              /*---------------------------------------*/
              
              if(line[0] == '\n')
                 line[i]   = '\0';
              else 


              /*--------------------------------------------*/
              /* Check for content. Discard empty line      */
              /* containing only whitespace and/or comment. */
              /*--------------------------------------------*/

              {  for(i=0; i<strlen(line); ++i)
                 {  if(line[i] == '#')
                    {  line[i]   = '\0';
                       break;
                    }
                    else if(line[i] != "" && line != '\t')
                       content = TRUE;
                 }
              }              


              /*-----------------------------------------*/
              /* Only output lines which contain content */
              /*-----------------------------------------*/

              if(content == TRUE)
              {  (void)fputs(line,t_file);
                 (void)fflush(t_file);
              }
          }

        } while(looper == TRUE);


   /*---------------------------------------*/
   /* Free all resources no longer required */
   /*---------------------------------------*/

   pups_set_errno(OK);
   (void)pups_free(line);
   (void)fclose(c_file);


   /*-------------------------------------------------------*/
   /* Rewind stripped data file, and return a pointer to it */
   /*-------------------------------------------------------*/

   (void)rewind(t_file);
   return(t_file);
}



/*---------------------------------------------------*/
/* Return first non whitespace character in a string */
/*---------------------------------------------------*/

_PUBLIC char *strfirst(const char *s1)

{   uint32_t i,
             size;

    if(s1 == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return((char *)NULL);
    }

    size = strlen(s1);
    for(i=0; i<size; ++i)
    {  if(s1[i] != ' ' && s1[i] != '\n' && s1[i] != '\r' && s1[i] != '\0')
         return(&s1[i]);
    } 

    pups_set_errno(OK);
    return((char *)NULL);
}





/*----------------------------*/
/* Strips comments 'in place' */
/*----------------------------*/

_PUBLIC _BOOLEAN strip_comment(const FILE *stream,  int32_t *line_cnt, char *line)

{  
    if(stream == (const FILE *)NULL || line_cnt == (int32_t *)NULL || line == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    /*-------------------------------------------------*/
    /* Wind to next line which is not either a comment */
    /* or whitespace                                   */
    /*-------------------------------------------------*/

    do {

           /*-----------------------------------------------------*/
           /* Swapped position of fgets() used to be after feof() */
           /*-----------------------------------------------------*/

           (void)fgets(line,SSIZE,stream);

           if(feof(stream) != 0)
              return(FALSE);

           ++(*line_cnt);
       } while(strempty(line) == TRUE || strin(strfirst(line),"#") == TRUE);

    pups_set_errno(OK);
    return(TRUE);
}




/*------------------------------------*/
/* Convert ASCII character to integer */
/*------------------------------------*/

_PUBLIC int32_t actoi(const char ch)

{   if((int32_t)ch >= (int32_t)'0' && (int32_t)ch <= (int32_t)'9')
       return((int32_t)ch - 0x30);
    else
       return(-1);
}



/*-----------------------------*/
/* Test for short even integer */
/*-----------------------------*/

_PUBLIC _BOOLEAN seven(const int16_t arg)

{   if(arg%2 > 0)
       return(FALSE);
    else
       return(TRUE);
}




/*-----------------------*/
/* Test for even integer */
/*-----------------------*/

_PUBLIC _BOOLEAN ieven(const int32_t arg)

{   if(arg%2 > 0)
       return(FALSE);
    else
       return(TRUE);
}




/*----------------------------*/
/* Test for long even integer */
/*----------------------------*/

_PUBLIC _BOOLEAN leven(const int64_t  arg)

{   if(arg%2 > 0)
       return(FALSE);
    else
       return(TRUE);
}




/*------------------------------*/
/*  Test for short integer sign */
/*------------------------------*/

_PUBLIC  int32_t ssign(const int16_t arg)

{   if(arg > 0)
       return((int32_t)1);
    else if(arg < 0)
       return((int32_t)(-1));

    return((int32_t)0);
}




/*-----------------------*/
/* Test for integer sign */
/*-----------------------*/

_PUBLIC  int32_t isign(const int32_t arg)

{   if(arg > 0)
       return((int)1);
    else if(arg < 0)
       return((int)(-1));

    return((int)0);
}




/*----------------------------*/
/* Test for long integer sign */
/*----------------------------*/

_PUBLIC  int32_t lsign(const int64_t arg)

{   if(arg > 0)
       return((int32_t)1);
    else if(arg < 0)
       return((int32_t)(-1));

    return((int32_t)0);
}




/*--------------------------------------------*/
/* Find the maximum of pair of short integers */
/*--------------------------------------------*/

_PUBLIC int32_t smax(const int16_t a, const int16_t b)

{   if(a >= b)
       return((int32_t)a);
    else
       return((int32_t)b);
}




/*--------------------------------------*/
/* Find the maximum of pair of integers */
/*--------------------------------------*/

_PUBLIC int32_t imax(const int32_t a, const int32_t b)

{   if(a >= b)
       return(a);
    else
       return(b);
}




/*-------------------------------------------*/
/* Find the maximum of pair of int64_t egers */
/*-------------------------------------------*/

_PUBLIC  int32_t lmax(const  int64_t  a, const int64_t  b)

{   if(a >= b)
       return((int32_t)a);
    else
       return((int32_t)b);
}




/*--------------------------------------------*/
/* Return the sign of floating point argument */
/*--------------------------------------------*/

_PUBLIC FTYPE fsign(const FTYPE arg)

{   if(arg > 0.0)
       return(1.0);

    if(arg < 0.0)
       return(-1.0);

    return(0.0);
}




/*---------------------------------------------------*/
/* Round floating point value to the nearest integer */
/*---------------------------------------------------*/

_PUBLIC  int32_t iround(const FTYPE x)
{   return((int32_t)rint(x));
}




/*-----------------------------------------*/
/* Pause program (while under development) */
/*-----------------------------------------*/

_PUBLIC int32_t upause(const char *prompt)

{   char ch;

    if(prompt == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);

    (void)fprintf(stderr,"%s\n",prompt);
    (void)fflush (stdin);
    (void)scanf  ("%c",&ch);

    if(pups_get_errno() == OK)
       return(0);
    else
       return(-1);
}




/*------------------------------------------------------------------*/
/* Test to see if file is on a mounted filesystem. If it is, return */
/* details of the host exporting the filesystem                     */
/*------------------------------------------------------------------*/

_PUBLIC _BOOLEAN pups_get_fs_mountinfo(const char *f_name, char *mount_host)

{    int32_t f_index,
             status;

    FILE     *mtab_stream      = (FILE *)NULL;

    char     line[SSIZE]       = "",
             cwd[SSIZE]        = "",
             import_fs[SSIZE]  = "",
             pathname[SSIZE]   = "";


    if(f_name == (const char *)NULL || mount_host == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if(pups_is_on_nfs(f_name) == FALSE)
       return(FALSE);


    /*---------------------------------------------------*/
    /* We must have an absolute path - if not we need to */
    /* build the absolute path                           */
    /*---------------------------------------------------*/

    if(f_name[0] != '/')
    {  (void)getcwd(cwd,SSIZE);
       (void)snprintf(pathname,SSIZE,"%s/%s",cwd,f_name);
    }
    else
       (void)strlcpy(pathname,f_name,SSIZE);


    /*------------------------------------------------*/
    /* df O/P has a common format in most modern UNIX */
    /* variants. It may be quicker to use /etc/mtab   */
    /* but unfortunately this is far less standard    */
    /*------------------------------------------------*/

    mtab_stream = pups_fcopen("df",shell,"r");
    do {   (void)pups_fgets(line,SSIZE,mtab_stream);

           if(feof(mtab_stream) == 0)
           {  char strdum[SSIZE];

              (void)sscanf(line,"%s%s%s%s%s%s",mount_host,strdum,strdum,strdum,strdum,import_fs);
              if(strin(pathname,import_fs) == TRUE)
              {  

                 /*-----------------------------------------------*/
                 /* Get name of host exporting file's filesystem  */
                 /* we only need process mount information if we  */
                 /* detect a ':' character which is indicative of */
                 /* a mounted filesystem                          */
                 /*-----------------------------------------------*/

                 if((f_index = ch_pos(mount_host,':')) != (-1) && f_index < 256)
                 {  mount_host[f_index] = '\0';

                    (void)pups_fclose(mtab_stream);

                    pups_set_errno(OK);
                    return(TRUE);
                }
              }
           }
       } while(feof(mtab_stream) == 0);

    (void)strlcpy(mount_host,"none",SSIZE);
    (void)pups_fclose(mtab_stream);


    pups_set_errno(OK);
    return(FALSE);
}




/*-------------------------------------------------------------------*/
/* Find next free file table index (returns (-1) if file table full) */
/*-------------------------------------------------------------------*/

_PUBLIC int32_t pups_find_free_ftab_index(void)

{   uint32_t i;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
       if(ftab[i].fdes == (-1))
       {  pups_set_errno(OK);

          ftab[i].fname[0]    = '\0';
          ftab[i].hname[0]    = '\0';
          ftab[i].fshadow[0]  = '\0';
          ftab[i].rd_host[0]  = '\0';
          ftab[i].fs_name[0]  = '\0';
                   
          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          return((int32_t)i);
       }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ENOMEM);
    return(-1);
}




/*------------------------------------------------------------------------*/
/* Get the file table entry which corresponds to descriptor (returns (-1) */
/* if there is no entry)                                                  */
/*------------------------------------------------------------------------*/

_PUBLIC int32_t pups_get_ftab_index(const des_t fdes)

{   uint32_t i;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].fdes == fdes)
       { 

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return((int32_t)i);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




/*-----------------------------------------*/
/* Update PSRP channel entry in file table */
/*-----------------------------------------*/

_PUBLIC int32_t pups_update_psrp_channel_ftab_entry(const char *chname, const char *chid)

{   uint32_t i;


    /*--------------*/
    /* Sanity check */
    /*--------------*/

    if(strcmp(chid,"#in") != 0 && strcmp(chid,"#out") != 0)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(strin(ftab[i].fname,chid) == TRUE)
       {  (void)strlcpy(ftab[i].fname,chname,SSIZE); 

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return((int32_t)i);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




/*-----------------------------------------*/
/* Set file table entry identification tag */
/*-----------------------------------------*/

_PUBLIC int32_t pups_set_ftab_id(const des_t fdes, const int32_t id)

{   uint32_t i;
    int32_t  f_index;

    if(fdes < 0 || fdes >= appl_max_files || id < 0)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if((f_index = pups_get_ftab_index(fdes)) == (-1))
       return(-1);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].id == id)
       {  

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(EEXIST);
          return(-1);
       }
    }

    ftab[f_index].id = id; 

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}




/*----------------------------------------------*/
/* Get file table entry (by identification tag) */
/*----------------------------------------------*/

_PUBLIC int32_t pups_get_ftab_index_by_id(const int32_t id)

{   uint32_t i;

    if(id < 0)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].id == id)
       {  

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return((int32_t)i);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}





/*------------------------------------*/
/* Get file table entry (by filename) */
/*------------------------------------*/

_PUBLIC int32_t pups_get_ftab_index_by_name(const char *name)

{   uint32_t i;

    if(name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].fname != (char *)NULL && strcmp(ftab[i].fname,name) == 0)
       {  

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return((int32_t)i);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}





/*-----------------------*/
/* Clear file table slot */
/*-----------------------*/

_PUBLIC int32_t pups_clear_ftab_slot(const _BOOLEAN destroy, const int32_t f_index)

{   if(f_index < 0 || f_index >= appl_max_files)
    {  pups_set_errno(EINVAL);
       return(-1);
    } 

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    ftab[f_index].st_mode           = 0;
    ftab[f_index].fdes              = (-1);
    ftab[f_index].id                = (-1);
    ftab[f_index].mode              = (-1);
    ftab[f_index].stream            = (FILE *)NULL;
    ftab[f_index].psrp              = FALSE;
    ftab[f_index].homeostatic       = 0;
    ftab[f_index].handler           = (void *)NULL;
    ftab[f_index].homeostat         = (void *)NULL;
    ftab[f_index].fs_blocks         = 128;
    ftab[f_index].mounted           = FALSE;
    ftab[f_index].rd_pid            = (-1);
    ftab[f_index].fifo_pid          = (-1);
    ftab[f_index].locked            = FALSE;

    #ifdef ZLIB_SUPPORT
    ftab[f_index].zstream           = (gzFile)NULL;
    #endif /* ZLIB_SUPPORT */

    if(destroy == TRUE)
    {  (void)strlcpy(ftab[f_index].fname,  "",SSIZE); 
       (void)strlcpy(ftab[f_index].hname,  "",SSIZE);
       (void)strlcpy(ftab[f_index].fshadow,"",SSIZE);

       #ifdef SSH_SUPPORT
       (void)strlcpy(ftab[f_index].rd_host,    "",SSIZE); 
       (void)strlcpy(ftab[f_index].rd_ssh_port,"",SSIZE);
       #endif /* SSH_SUPPORT */

       (void)strlcpy( ftab[f_index].fs_name,"",SSIZE);

       ftab[f_index].creator           = FALSE;
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}



/*----------------------------------------------------*/
/* Initialise the PUPS extended file descriptor table */
/*----------------------------------------------------*/

_PRIVATE void initftab(int max_files, int32_t stdio_homeostasis)

{   uint32_t    i;
    struct stat buf;

    if((ftab = (ftab_type *)pups_calloc(max_files,sizeof(ftab_type))) == (ftab_type *)NULL)
    {  (void)snprintf(errstr,SSIZE,"[initftab] could not allocate PUPS file table (%d entries)\n",max_files);
       pups_error(errstr);
    }

    (void)fstat(0,&buf);
    ftab[0].st_mode     = (int32_t)buf.st_mode;
    ftab[0].stream      = stdin;
    ftab[0].mode        = 0;

    (void)fstat(1,&buf);
    ftab[1].st_mode     = (int32_t)buf.st_mode;
    ftab[1].stream      = stdout;
    ftab[1].mode        = 1;

    (void)fstat(2,&buf);
    ftab[2].st_mode     = (int32_t)buf.st_mode;
    ftab[2].stream      = stderr;
    ftab[2].mode        = 2;


    /*------------------*/
    /* Initialise stdio */
    /*------------------*/

    if(isatty(0) == 1)
    {  ftab[0].named = TRUE;
       (void)snprintf(ftab[0].fname,SSIZE,"%s/pups.%s.%s.stdin.pst.%d:%d", appl_fifo_dir,appl_name,appl_host,getpid(),getuid());
    }
    else
    {  ftab[0].named = FALSE;
       (void)snprintf(ftab[0].fname,SSIZE,"<redirected.0>");
    }

    if(isatty(1) == 1)
    {  ftab[1].named = TRUE;
       (void)snprintf(ftab[1].fname,SSIZE,"%s/pups.%s.%s.stdout.pst.%d:%d",appl_fifo_dir,appl_name,appl_host,getpid(),getuid());
    }
    else
    {  ftab[1].named = FALSE;
       (void)snprintf(ftab[1].fname,SSIZE,"<redirected.1>");
    }

    if(isatty(2) == 1)
    {  ftab[2].named = TRUE;
       (void)snprintf(ftab[2].fname,SSIZE,"%s/pups.%s.%s.stderr.pst.%d:%d",appl_fifo_dir,appl_name,appl_host,getpid(),getuid());
    }
    else
    {  ftab[2].named = FALSE;
       (void)snprintf(ftab[2].fname,SSIZE,"<redirected.2>");
    }

    for(i=0; i<3; ++i)
    {  char stdio_homeostat_name[SSIZE];

       #ifdef ZLIB_SUPPORT
       ftab[i].zstream           = (gzFile)NULL;
       #endif /* ZLIB_SUPPORT */

       ftab[i].psrp              = FALSE;
       ftab[i].creator           = TRUE;
       ftab[i].fs_blocks         = 128;
       ftab[i].fdes              = i;
       ftab[i].id                = (-1);
       ftab[i].homeostat         = (void *)NULL;
       ftab[i].handler           = (void *)NULL;
       ftab[i].homeostatic       = 0;
       ftab[i].locked            = FALSE;
       ftab[i].mounted           = FALSE;
       ftab[i].rd_pid            = (-1);
       ftab[i].fifo_pid          = (-1);

       (void)strlcpy(ftab[i].fs_name,".",SSIZE);
       (void)strlcpy(ftab[i].fshadow,"none",SSIZE);

       #ifdef SSH_SUPPORT
       (void)strlcpy(ftab[i].rd_host,"none",SSIZE);
       (void)strlcpy(ftab[i].rd_ssh_port,ssh_remote_port,SSIZE);
       #endif /* SSH_SUPPORT */

       (void)strlcpy(ftab[i].hname,"none",SSIZE);
       if(isatty(i) == 1)
          (void)symlink("/dev/tty",ftab[i].fname);

       if(stdio_homeostasis == STDIO_LIVE && strncmp(ftab[i].fname,"<redirected",11) != 0)
       {  switch(i)
          {     case 0: (void)snprintf(stdio_homeostat_name,SSIZE,"default_fd_homeostat: stdin ");
                        break;

                case 1: (void)snprintf(stdio_homeostat_name,SSIZE,"default_fd_homeostat: stdout");
                        break;

                case 2: (void)snprintf(stdio_homeostat_name,SSIZE,"default_fd_homeostat: stderr");
                        break;

                default: break;
          }

          (void)pups_fd_alive(i,stdio_homeostat_name,pups_default_fd_homeostat);
       }
    }


    /*-------------------------------*/
    /* Initialise rest of file table */
    /*-------------------------------*/

    for(i=3; i<appl_max_files; ++i)
       pups_clear_ftab_slot(FALSE,i);
}





/*------------------------------------------------------------------*/
/* Get the number of time a given filesystem resource has been lost */
/*------------------------------------------------------------------*/

_PUBLIC int32_t pups_lost(const des_t fdes)

{   uint32_t i;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].fdes == fdes)
       {  

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK); 
          return((int32_t)ftab[i].lost_cnt);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}



/*-----------------------------------*/
/* Test if file descriptor is living */
/*-----------------------------------*/

_PUBLIC _BOOLEAN pups_fd_islive(const des_t fdes)

{   uint32_t i;

    pups_set_errno(OK);
    for(i=0; i<appl_max_files; ++i)
    {  

       /*------------------------*/
       /* File descriptor living */
       /*------------------------*/

       if(ftab[i].fdes == fdes && ftab[i].homeostatic > 0)
          return(TRUE);
    }


    /*-----------------------*/
    /* Is descriptor valid ? */
    /*-----------------------*/

    if(i == appl_max_files)
    { 

       /*-------------------------*/
       /* Invalid file descriptor */
       /*-------------------------*/

       pups_set_errno(EBADF);
       return(FALSE);
    }

    /*----------------------*/   
    /* Dead file descriptor */
    /*----------------------*/
    
    return(FALSE); 
}




/*-------------------------------*/
/* Make a file descriptor living */
/*-------------------------------*/

_PUBLIC int32_t pups_fd_alive(const des_t fdes, const char *handler_name, const void *handler)

{   uint32_t i;

    char     handler_args[SSIZE] = "",
             cntl_chars[]        = {'\1', '\2', '\3', '\4', '\5', '\6'};

    if(handler_name == (const char *)NULL || handler == (const void *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {   int32_t ret;

       if(ftab[i].fdes == fdes)
       {   int32_t t_index;
 
          if(ftab[i].creator == FALSE)
          {  

            #ifdef PTHREAD_SUPPORT
            (void)pthread_mutex_unlock(&ftab_mutex);
            #endif /* PTHREAD_SUPPORT */

             pups_set_errno(EACCES);
             return(-1);
          }

          if(ftab[i].psrp == TRUE)
          {  

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&ftab_mutex);
             #endif /* PTHREAD_SUPPORT */

             pups_set_errno(EACCES);
             return(-1);
          }


          /*------------------------------------------*/
          /* Check that this file is not already live */
          /* if it is simply increment the count on   */
          /* its homeostasis lock  and return         */
          /*------------------------------------------*/

          if(ftab[i].homeostatic > 0)
          {  int32_t ret;

             ++ftab[i].homeostatic;
             ret = ftab[i].homeostatic - 1;

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&ftab_mutex);
             #endif /* PTHREAD_SUPPORT */

             pups_set_errno(OK);
             return(ret);
          }


          /*---------------------*/
          /* File not live (yet) */
          /*---------------------*/

          else
          {  ftab[i].homeostatic = 1;
             ftab[i].lost_cnt    = 0;
          }


          /*-------------*/
          /* Shadow file */
          /*-------------*/

          if(isatty(ftab[i].fdes))
             (void)snprintf(ftab[i].fshadow,SSIZE,"/dev/tty");


          /*------------------------------------------------*/
          /* Directory protection files do not have shadows */
          /* they are simply recreated when lost            */
          /*------------------------------------------------*/

          else
          {  uint32_t j,
                      cnt,
                      c_index;

             char     cntl_str[SSIZE] = ""; 


             /*--------------------------------------------------*/
             /* Check to see if we are either in (or a link to)  */
             /* a patchboard directory                           */
             /*--------------------------------------------------*/

             if(strin(ftab[i].fname,"fifos") == TRUE)
             {  c_index                    = rch_pos(ftab[i].fname,'/');
                (void)strlcpy(ftab[i].fshadow,ftab[i].fname,SSIZE);

                ftab[i].fshadow[c_index+1] = '.';
                ftab[i].fshadow[c_index+2] = '\0';
             }
             else
                (void)strlcpy(ftab[i].fshadow,".",SSIZE);


             /*----------------*/
             /* Crypted shadow */
             /*----------------*/

             if(appl_snames_crypted == TRUE)
             {  char fshadow_name[SSIZE] = "";


                /*--------------------------------------------------*/
                /* Make sure that the shadow file is well protected */
                /* so that rm -rf * does not remove the shadow file */
                /* before it can be used to re-instate the file it  */
                /* is supposed to be protecting                     */
                /*--------------------------------------------------*/

                for(j=1; j<8; ++j)
                {  c_index = (int32_t)(drand48()*6.0);
                   (void)snprintf(cntl_str,SSIZE,"%c",cntl_chars[c_index]);
                   (void)strlcat(fshadow_name,cntl_str,SSIZE);
                }


                /*---------------------------------------------------*/
                /* Put shadow in the same directory as its principle */
                /*---------------------------------------------------*/

                if(strin(ftab[i].fname,"/") == TRUE)
                {  char fdir[SSIZE] = "";

                   (void)strlcpy(fdir,ftab[i].fname,SSIZE);
                   c_index = rch_index(fdir,'/');
                   fdir[c_index] = '\0';

                   (void)snprintf(ftab[i].fshadow,SSIZE,"%s.%s",fdir,fshadow_name);
                }
                else
                   (void)snprintf(ftab[i].fshadow,SSIZE,".%s",fshadow_name);
             }


             /*------------------*/
             /* Plaintext shadow */
             /*------------------*/

             else
             {

                /*---------------------------------------------------*/
                /* Put shadow in the same directory as its principle */
                /*---------------------------------------------------*/

                if(strin(ftab[i].fname,"/") == TRUE)
                {  char fdir[SSIZE] = "";

                   (void)strlcpy(fdir,ftab[i].fname,SSIZE); 
                   c_index = rch_index(fdir,'/');
                   fdir[c_index] = '\0';

                   (void)snprintf(ftab[i].fshadow,SSIZE,"%s/.%d.%d.shadow.tmp",fdir,i,appl_pid);
                }
                else 
                   (void)snprintf(ftab[i].fshadow,SSIZE,".%d.%d.shadow.tmp",i,appl_pid);
             } 

             (void)link(ftab[i].fname,ftab[i].fshadow);
          }


          /*--------------------------------------*/
          /* Increment homeostat protection level */
          /*--------------------------------------*/

          ++ftab[i].homeostatic;


          /*-----------------------------------*/
          /* Set up virtual time (and payload) */
          /*-----------------------------------*/

          (void)snprintf(ftab[i].hname,SSIZE,"%s: (%s)",handler_name,ftab[i].fname);
          ftab[i].handler = handler;
          (void)snprintf(handler_args,SSIZE,"%d",i);

          t_index = pups_setvitimer(ftab[i].hname,
                                    1,VT_CONTINUOUS,10,
                                    handler_args,
                                    (void *)ftab[i].handler);

          ret = ftab[i].homeostatic - 1;

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */


          /*--------*/
          /* Error */
          /*-------*/

          if(t_index == (-1))
          {  pups_set_errno(EINVAL);
             return(-1);
          }


          /*-----*/
          /* Log */
          /*-----*/

          else if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): \"%s\" is now live (status polling via vtimer %d (payload function: %s)\n",
                                                date,appl_name,appl_pid,appl_host,appl_owner,ftab[i].fname,t_index,handler_name);
             (void)fflush(stderr);
          }

          pups_set_errno(OK);
          return(ret);
       }
    }

    pups_set_errno(ESRCH);
    return(-1);
}




/*-----------------------------------------------------------------------------*/
/* Make the current application the creator of the object associated with      */
/* file descriptor (which means it will have the responsibility of             */
/* destroying it in a multiprocess environment)                                */
/*                                                                             */
/* To quote J. Robert Oppenhiemer "I am become death, the destroyer of worlds" */
/*-----------------------------------------------------------------------------*/

_PUBLIC int32_t pups_creator(const des_t fdes)

{   uint32_t i;


    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].fdes == fdes)
       {
          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): %s has acquired creator privileges for \"%s\"\n",
                                 date,appl_name,appl_pid,appl_host,appl_owner,appl_name,ftab[i].fname);
             (void)fflush(stderr);
          }

          pups_set_errno(OK);
          return(0);
       }

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */
    }

    pups_set_errno(ESRCH);
    return(-1);
} 




/*----------------------------------------------------------------------------------*/
/* Relieve this application of the onerous burden of being the creator/destroyer of */
/* the object referenced by fdes                                                    */
/*----------------------------------------------------------------------------------*/

_PUBLIC int32_t pups_not_creator(const des_t fdes)

{   uint32_t i;

    if(fdes < 0)
    {  pups_set_errno(EBADF);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i) 
    {  if(ftab[i].fdes == fdes) 
       {  ftab[i].creator = FALSE;

          if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): creator privilidges for \"%s\" relinquished\n",
                                         date,appl_name,appl_pid,appl_host,appl_owner,ftab[i].fname);
             (void)fflush(stderr);
          }

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return(0);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
} 




/*-----------------------------*/
/* Test to see if file is live */
/*-----------------------------*/

_PUBLIC _BOOLEAN pups_isalive(const char *name)

{   uint32_t i;

    pups_set_errno(OK);

    if(name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].fname              != (char *)NULL    &&
          strcmp(ftab[i].fname,name) == 0               &&
          ftab[i].homeostatic        >  0                )
       {
          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          return(TRUE);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */
 
    return(FALSE);
}




/*--------------------------*/
/* Protect an unopened file */
/*--------------------------*/

_PUBLIC int32_t pups_protect(const char *name, const char *handler_name, const void *handler)

{    des_t fdes = (-1);

     if(name == (const char *)NULL || handler_name == (const char *)NULL || handler == (const void *)NULL)
     {  pups_set_errno(EINVAL);
        return(-1);
     }

     if(access(name,F_OK | R_OK | W_OK) == (-1))
     {  pups_set_errno(EEXIST);
        return(-1);
     }

     fdes = pups_open(name,O_DUMMY,LIVE);

     (void)pups_creator(fdes);
     (void)pups_fd_alive(fdes,handler_name,handler);
     (void)close(fdes);

     pups_set_errno(OK);
     return(0);
}





/*-------------------------------*/
/* Stop protecting unopened file */
/*-------------------------------*/

_PUBLIC int32_t pups_unprotect(const char *name)

{   uint32_t i;

    if(name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */


    for(i=0; i<appl_max_files; ++i)
    {  if(strcmp(ftab[i].fname,name) == 0 && ftab[i].homeostatic > 0)
       {

          /*---------------------------------------------*/
          /* Do not remove protection from PSRP channels */
          /* (unless application is exiting).            */
          /*---------------------------------------------*/

          if(ftab[i].psrp == TRUE && do_closeall == FALSE)
          {  pups_set_errno(EACCES);
             return(-1);
          }


          /*-------------------------------------------*/
          /* If the resource is closed then we have to */
          /* terminate homeostat                       */
          /*-------------------------------------------*/

          if(in_close_routine == FALSE && ftab[i].homeostatic > 1)
          {  --ftab[i].homeostatic;

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&ftab_mutex);
             #endif /* PTHREAD_SUPPORT */

             pups_set_errno(PSRP_OK);
             return(0);
          }


          /*--------------------------------------------*/
          /* Do not remove protection from heap objects */
          /* (unless application is exiting).           */
          /*--------------------------------------------*/

          if(ftab[i].pheap == TRUE && do_closeall == FALSE)
          {  pups_set_errno(EACCES);
             return(-1);
          }


          /*--------------------------------------------*/
          /* Remove homeostat associated with this item */
          /*--------------------------------------------*/

          if(ftab[i].psrp == TRUE && do_closeall == TRUE)
             (void)pups_clearvitimer("psrp_homeostat");
          else
             (void)pups_clearvitimer(ftab[i].hname);


          /*---------------------------------------------------------*/
          /* Make sure we don't remove /dev/tty if descriptor is     */
          /* 0,1 or 2 (these are symlinks which are shadowed by      */
          /* dev/tty). If application is suid root and we do not     */
          /* do this test, we could remove /dev/tty with potentially */
          /* serious consequences                                    */
          /*---------------------------------------------------------*/

          if(strcmp(ftab[i].fshadow,"/dev/tty") != 0)
             (void)unlink(ftab[i].fshadow);

          (void)strlcpy(ftab[i].fshadow,"none",SSIZE);

          ftab[i].homeostatic = 0;
          ftab[i].handler     = (void *)NULL;

          (void)strlcpy(ftab[i].hname,"none",SSIZE);

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return(0);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




/*------------------------------------------------------------------------*/
/* Make a living descriptor inanimate (and remove hoemeostatic protection */
/* of underlying inode-object)                                            */
/*------------------------------------------------------------------------*/

_PUBLIC int32_t pups_fd_dead(const des_t fdes)

{   uint32_t i; 
    char     fname[SSIZE] = "";

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i) 
    {  if(ftab[i].fdes == fdes)
       {  

          /*--------------*/
          /* Already dead */
          /*--------------*/

          if(ftab[i].homeostatic == 0)         
          {   
             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&ftab_mutex);
             #endif /* PTHREAD_SUPPORT */

             pups_set_errno(PSRP_OK);
             return(0);
          }


          /*---------------------------------------*/
          /* Decrease homeostatic protection level */
          /*---------------------------------------*/

          else
             --ftab[i].homeostatic;


          if(in_close_routine == FALSE && ftab[i].homeostatic > 1)
          {

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&ftab_mutex);
             #endif /* PTHREAD_SUPPORT */

             pups_set_errno(PSRP_OK);
             return(ftab[i].homeostatic - 1);
          }


          /*--------------------------------------------*/
          /* Remove homeostat associated with this item */
          /*--------------------------------------------*/

          (void)pups_clearvitimer(ftab[i].hname);


          /*---------------------------------------------------------*/
          /* Make sure we don't remove /dev/tty if descriptor is     */
          /* 0,1 or 2 (these are symlinks which are shadowed by      */
          /* dev/tty). If application is suid root and we do not     */
          /* do this test, we could remove /dev/tty with potentially */
          /* serious consequences                                    */
          /*---------------------------------------------------------*/
         
          if(strcmp(ftab[i].fshadow,"/dev/tty") != 0)
             (void)unlink(ftab[i].fshadow);

          (void)strlcpy(ftab[i].fshadow,"none",SSIZE);

          ftab[i].homeostatic = 0;
          ftab[i].handler     = (void *)NULL;

          (void)strlcpy(ftab[i].hname,"none",SSIZE);
          (void)strlcpy(fname,ftab[i].fname,SSIZE);

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): \"%s\" is now dead\n",
                           date,appl_name,appl_pid,appl_host,appl_owner,fname);
             (void)fflush(stderr);
          }

          pups_set_errno(OK);
          return(0);
       }
   }

   #ifdef PTHREAD_SUPPORT
   (void)pthread_mutex_unlock(&ftab_mutex);
   #endif /* PTHREAD_SUPPORT */

   pups_set_errno(ESRCH);
   return(-1);
}




/*-----------------------------------------------------------------*/
/* Check the file table closing all open files and releasing their */
/* resources                                                       */
/*-----------------------------------------------------------------*/

_PUBLIC void pups_closeall(void)

{   uint32_t i;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    do_closeall = TRUE;
    for(i=3; i<appl_max_files; ++i)
    {  if(ftab[i].fdes != (-1) && ftab[i].fname != (char *)NULL)
       {  if(appl_verbose == TRUE)
          {  (void)strdate(date); 
             (void)fprintf(stderr,"%s %s (%d@%s:%s): closing %-48s [ftab slot %04d]\n",
                                          date,appl_name,appl_pid,appl_host,appl_owner,
                                                                         ftab[i].fname,
                                                                                     i);
             (void)fflush(stderr);
          }
 
          (void)pups_fd_dead(ftab[i].fdes);
          (void)pups_close(ftab[i].fdes);
       }
    }

    if(appl_verbose == TRUE)
    {  (void)fprintf(stderr,"\n");
       (void)fflush(stderr);
    }


    /*--------------------------------------------------*/
    /* Close any non file table files which may be open */
    /*--------------------------------------------------*/

    for(i=3; i<appl_max_files; ++i)
       if(i != chlockdes)
         (void)close(i); 

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
}




/*---------------------------------------*/
/* Get ftab index (from file descriptor) */
/*---------------------------------------*/

_PUBLIC int32_t pups_get_ftab_index_from_fd(const des_t fdes)

{   uint32_t i;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {   if(ftab[i].fdes == fdes)
        {

           #ifdef PTHREAD_SUPPORT
           (void)pthread_mutex_unlock(&ftab_mutex);
            #endif /* PTHREAD_SUPPORT */

            pups_set_errno(OK);
            return((int32_t)i);
        }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




/*--------------------------------------*/
/* Get ftab index (from stream pointer) */
/*--------------------------------------*/

_PUBLIC int32_t pups_get_ftab_index_from_stream(const FILE *stream)

{   uint32_t i;

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {   if(ftab[i].stream == stream)
        {  

           #ifdef PTHREAD_SUPPORT
           (void)pthread_mutex_unlock(&ftab_mutex);
            #endif /* PTHREAD_SUPPORT */

            pups_set_errno(OK);
            return((int32_t)i);
        }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




#ifdef ZLIB_SUPPORT
/*--------------------------------------------------*/
/* Open file associated with compressed data stream */
/*--------------------------------------------------*/

_PUBLIC gzFile pups_gzopen(const char *f_name, const char *mode, const int32_t h_state)

{    int32_t i_mode,
             z_index,
             zdes  = (-1);

    gzFile zstream = (gzFile)NULL;

    if(f_name == (char *)NULL  ||
       mode   == (char *)NULL   )
    {  pups_set_errno(EINVAL);
       return((gzFile)NULL);
    }


    /*----------------------*/
    /* Set up mode switches */
    /*----------------------*/

    if(strcmp(mode,"r") == 0)
       i_mode = 0;
    else
    {  if(strcmp(mode,"w") == 0)
          i_mode = 1;
       else
       {  if(strcmp(mode,"w+") == 0   ||
             strcmp(mode,"r+") == 0)
             i_mode = 2;
          else
          {  pups_set_errno(EINVAL);
             return((gzFile)NULL);
          }
       }
    }


    /*---------------------------------*/
    /* Open underlying file descriptor */
    /*---------------------------------*/

    if((zdes = pups_open(f_name,i_mode,h_state)) == (-1))
       return((gzFile)NULL);


    /*----------------------------------------*/
    /* Associate zstream with this descriptor */
    /*----------------------------------------*/

    if((zstream = gzdopen(zdes,mode)) == (gzFile)NULL)
       return((gzFile)NULL);

    z_index               = pups_get_ftab_index(zdes); 
    ftab[z_index].zstream = zstream;

    pups_set_errno(OK);
    return(zstream);
}




/*---------------------------------------*/
/* Get ftab index (from zstream pointer) */
/*---------------------------------------*/

_PUBLIC int32_t pups_get_ftab_index_from_zstream(const gzFile zstream)

{   uint32_t i;

    if(zstream == (const gzFile)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].zstream == zstream)
       {

         #ifdef PTHREAD_SUPPORT
         (void)pthread_mutex_unlock(&ftab_mutex);
         #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return((int32_t)i);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




/*-------------------------------------------------------*/
/* Close a file associated with a compressed data stream */
/*-------------------------------------------------------*/

_PUBLIC gzFile pups_gzclose(const gzFile zstream)

{    int32_t z_index;

    if(zstream == (gzFile *)NULL)
    {  pups_set_errno(EINVAL);
       return((gzFile)NULL);
    }
 
    if((z_index = pups_get_ftab_index_from_zstream(zstream)) == (-1))
    {  pups_set_errno(EBADF);
       return((gzFile)NULL);
    }

    (void)gzclose(zstream);
    (void)pups_close(ftab[z_index].fdes);

    pups_set_errno(OK);
    return((gzFile)NULL);
}
#endif /* ZLIB_SUPPORT */




/*--------------------------------*/
/* Check to see if file is hidden */
/*--------------------------------*/

_PRIVATE _BOOLEAN pups_hidden_leaf(const char *fname)

{   char leaf[SSIZE]   = ""; 

    (void)strleaf(fname,leaf);
    if(leaf[0] == '.')
       return(TRUE);

    return(FALSE);
}




/*----------------------------------------------------------------------*/
/* Check for the existence of a file - if it exists, open it, otherwise */
/* print error code and abort                                           */
/*----------------------------------------------------------------------*/
/*--------------------------------------------*/
/* Forward declaration of ch_index() function */
/*--------------------------------------------*/

_PUBLIC ssize_t ch_index(const char *, const char);


/*----------------------------------------------*/
/* Forward declaration of the strinp() function */
/*----------------------------------------------*/

_PUBLIC _BOOLEAN strinp(size_t *, const char *, const char *);


_PUBLIC FILE *pups_fopen(const char *f_name, const char *mode, int32_t h_state)

{   FILE          *stream = (FILE *)NULL;
    int32_t       pos;
    struct statfs buf;


    /*-------------------------*/     
    /* Disable PUPS/P3 signals */
    /*-------------------------*/     

    (void)pupshold(ALL_PUPS_SIGS);


    /*---------------------------*/
    /* Check function parameters */
    /*---------------------------*/

    if(f_name == (char *)NULL || mode == (char *)NULL || (h_state != LIVE && h_state != DEAD))
    {  (void)pupsrelse(ALL_PUPS_SIGS);
       pups_set_errno(EINVAL);
       return((FILE *)NULL);
    }


    /*-------------------------------*/
    /* Hidden leaves are not allowed */
    /*-------------------------------*/

    if(pups_hidden_leaf(f_name) == TRUE)
    {  (void)pupsrelse(ALL_PUPS_SIGS);
       pups_set_errno(EINVAL);
       return((FILE *)NULL);
    }


    (void)statfs(f_name,&buf);
    if(buf.f_type == ISOFS_SUPER_MAGIC)
       h_state = DEAD;


    /*---------------------------------*/
    /* Try to open a local pipe stream */
    /*---------------------------------*/

    if(strinp((size_t *)&pos,f_name,"|") == TRUE)
    {  if((stream = pups_fcopen(&f_name[pos+1],shell,mode)) == (FILE *)NULL)
       {  (void)pupsrelse(ALL_PUPS_SIGS);
          return((FILE *)NULL);
       }

       else
       {  (void)pupsrelse(ALL_PUPS_SIGS);
          return(stream);
       }
    }


    /*---------------------------*/
    /* Process as a regular file */
    /*---------------------------*/

    else
    {  int32_t f_index,
               st_mode,
               cnt = 0;

       struct   stat buf;
       _BOOLEAN fix_perms = FALSE;


       /*----------------------------------------------------------*/
       /* Fix up permissions if file is live (homeostasis enabled) */
       /*----------------------------------------------------------*/

       if(h_state == LIVE || strin(mode,"l") == TRUE || strin(mode,"L") == TRUE)
          fix_perms = TRUE;


       /*-----------------------------*/
       /* Check to see if file exists */
       /*-----------------------------*/

       if(access(f_name,F_OK) == (-1))
       {  (void)pupsrelse(ALL_PUPS_SIGS);
          pups_set_errno(EEXIST);
          return((FILE *)NULL);
       }


       if((f_index = pups_find_free_ftab_index()) == (-1))
       {  (void)pups_clear_ftab_slot(FALSE,f_index);
          (void)pupsrelse(ALL_PUPS_SIGS);
          pups_set_errno(ESRCH);
          return((FILE *)NULL);
       }


       /*-----------------------------------------------------------------------*/
       /* If the file mode is O_DUMMY don't actually open it - simply place the */
       /* file in the file table so it may be homeostatically protected.        */
       /*-----------------------------------------------------------------------*/
       /*----------------------------------------------------------*/
       /* Stat the file to extract information on permissions and  */
       /* ownership etc from its inode                             */
       /*----------------------------------------------------------*/

       (void)stat(f_name,&buf);
       st_mode = buf.st_mode;

       if(mode[0] != 'D')
       {  

          /*---------------------------------------------------------*/
          /* FIFOS are always opened read/write irrespective of mode */
          /*---------------------------------------------------------*/

          if(S_ISFIFO(st_mode))
             stream = fopen(f_name,"r+");
          else
             stream = fopen(f_name,mode);

          if(stream == (FILE *)NULL)
          {  

             /*----------------------------------------------------------*/
             /* If permissions are to be forced fix up permissions so we */
             /* can do the desired operation on the file                 */
             /*----------------------------------------------------------*/
 
             if(fix_perms == LIVE) 
             {  if(strin(mode,"r") == TRUE)
                   st_mode |= S_IRUSR;

                if(strin(mode,"w") == TRUE || strin(mode,"r+") == TRUE)
                   st_mode |= S_IWUSR;

                if(chmod(f_name,st_mode) == (-1))
                {  (void)pups_clear_ftab_slot(FALSE,f_index);
                   (void)pupsrelse(ALL_PUPS_SIGS);
                   pups_set_errno(EACCES);
                   return((FILE *)NULL);
                }


                /*---------------------------------------------------------*/
                /* FIFOS are always opened read/write irrespective of mode */
                /*---------------------------------------------------------*/

                if(S_ISFIFO(st_mode))
                    stream = fopen(f_name,"r+");
                 else
                    stream = fopen(f_name,mode);
             }
             else
             {  (void)pups_clear_ftab_slot(FALSE,f_index);
                (void)pupsrelse(ALL_PUPS_SIGS);
                pups_set_errno(EACCES);
                return((FILE *)NULL);
             }
          }

          (void)strlcpy(ftab[f_index].fname,f_name,SSIZE);


          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_lock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          ftab[f_index].st_mode = st_mode;
          ftab[f_index].named   = TRUE;

          if(mode[0] == 'D')
          {  ftab[f_index].fdes = (-f_index - 1000);
             ftab[f_index].mode = O_DUMMY;
          }
          else
             ftab[f_index].fdes = fileno(stream);

          ftab[f_index].stream = stream;

          if(mode[0] == 'r' && mode[1] != '+')
             ftab[f_index].mode = 0;
          else
          {  if(mode[0] == 'w')
                ftab[f_index].mode = 1;
             else
                ftab[f_index].mode = 2;
          }


          /*--------------------------------------------*/
          /* If l or L flag is specified make file live */
          /*--------------------------------------------*/

          if(strin(mode,"l") == TRUE || strin(mode,"L") == TRUE || h_state == LIVE && ftab[f_index].psrp == FALSE)
          {  char default_fd_hname[SSIZE] = "";

             ftab[f_index].creator = TRUE;
             (void)snprintf(default_fd_hname,SSIZE,"default_fd_homeostat: %s(%d)",f_name,fileno(stream));
             (void)pups_fd_alive(fileno(stream),default_fd_hname,(void *)&pups_default_fd_homeostat);
          }       


          /*----------------------------------------------------------------------*/
          /* Get information about mount point (if this is a mounted file system) */
          /*----------------------------------------------------------------------*/

          ftab[f_index].mounted = pups_get_fs_mountinfo(f_name,ftab[f_index].rd_host);
       }

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */
    }


    /*------------------------*/
    /* Enable PUPS/P3 signals */
    /*------------------------*/

    (void)pupsrelse(ALL_PUPS_SIGS);


    pups_set_errno(OK);
    return(stream);
}




/*-------------------------------------------------------------------------*/
/* Extended fdopen command -- adds stream information to PUPS filetable if */
/* the passed descriptor is associated with PUPS filetable                 */
/*-------------------------------------------------------------------------*/

_PUBLIC FILE *pups_fdopen(const des_t fdes, const char *mode)

{   int32_t f_index;
    FILE    *ret = (FILE *)NULL;

    if(mode == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return((FILE *)NULL);
    }

    if((ret = fdopen(fdes,mode)) == (FILE *)NULL)
    {  pups_set_errno(EBADF);
       return((FILE *)NULL);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if((f_index = pups_get_ftab_index(fdes)) == (-1))
    {  ftab[f_index].stream = (FILE *)NULL;

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(ENFILE);
       return((FILE *)NULL);
    }


    /*--------------------------------------------*/
    /* If l or L flag is specified make file live */
    /*--------------------------------------------*/

    if(strin(mode,"l") == TRUE || strin(mode,"L") == TRUE)
    {  char default_fd_hname[SSIZE];
 
       (void)snprintf(default_fd_hname,SSIZE,"default_fd_homeostat: %s(%d)",ftab[f_index].fname,ftab[f_index].fdes);
       (void)pups_fd_alive(fileno(ret),"default_fd_hname",&pups_default_fd_homeostat);
    }


    /*--------------------------------------------*/
    /* if d or D flag is specified make file dead */
    /*--------------------------------------------*/

    else if(strin(mode,"d") == TRUE || strin(mode,"D") == TRUE)
       (void)pups_fd_dead(fdes);
 
    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(ret);
}




/*----------------------------------------------------------------------*/
/* Check for the existence of a file - if it exists, open it, otherwise */
/* print error code and abort                                           */
/*----------------------------------------------------------------------*/

_PUBLIC int32_t pups_open(const char *f_name, const int32_t mode, int32_t h_state)

{    int32_t pos;
     des_t   fdes  = (-1);

    struct statfs  buf;


    /*-------------------------*/
    /* Disable PUPS/P3 signals */
    /*-------------------------*/

    (void)pupshold(ALL_PUPS_SIGS);


    /*---------------------------*/
    /* Check function parameters */
    /*---------------------------*/

    if(f_name == (const char *)NULL || mode < 0 || (h_state != LIVE && h_state != DEAD))
    {  (void)pupsrelse(ALL_PUPS_SIGS);
       pups_set_errno(EINVAL);
       return(-1);
    }


    /*------------------------------*/
    /* Hidden files are not allowed */
    /*------------------------------*/

    if(pups_hidden_leaf(f_name) == TRUE)
    {  (void)pupsrelse(ALL_PUPS_SIGS);
       pups_set_errno(EINVAL);
       return(-1);
    }


    /*-------------------------------------------------*/
    /* ISO9660 and similar file systems do not support */
    /* file homeostasis                                */
    /*-------------------------------------------------*/

    (void)statfs(f_name,&buf);
    if(buf.f_type == ISOFS_SUPER_MAGIC)
       h_state = DEAD;


    /*---------------------------------*/
    /* Try to open a local pipe stream */
    /*---------------------------------*/

    if(strinp((size_t *)&pos,f_name,"|") == TRUE)
    {  if((fdes = pups_copen(&f_name[pos+1],shell,mode)) == (-1))
       {  (void)pupsrelse(ALL_PUPS_SIGS);
          pups_set_errno(EEXIST);
          return(-1);
       }

       (void)pupsrelse(ALL_PUPS_SIGS);
       pups_set_errno(OK);
       return(fdes);
    }


    /*----------------------------*/
    /* Process as a regular file. */
    /*----------------------------*/

    else
    {  int32_t st_mode,
               f_index;

       struct   stat buf;
       char     f_lock_name[SSIZE] = "";
       _BOOLEAN fix_perms          = FALSE;


       /*----------------------------------------------------------*/
       /* Fix up permissions if file is live (homeostasis enabled) */
       /*----------------------------------------------------------*/

       if(h_state == LIVE || mode & O_LIVE)
          fix_perms = TRUE;   


       /*------------------------------------*/
       /* Check to see if we can access file */
       /*------------------------------------*/

       if(access(f_name,F_OK) == (-1))
       {  (void)pupsrelse(ALL_PUPS_SIGS);
          pups_set_errno(EEXIST);
          return(-1);
       }


       if((f_index = pups_find_free_ftab_index()) == (-1)) 
       {  (void)pupsrelse(ALL_PUPS_SIGS);
          pups_set_errno(ENFILE);
          return(-1);
       }
 

       /*------------------------------------*/
       /* Check to see if we can access file */
       /*------------------------------------*/

       if(access(f_name,F_OK) == (-1))
       {  (void)pups_clear_ftab_slot(FALSE,f_index);
          (void)pupsrelse(ALL_PUPS_SIGS);
          pups_set_errno(ESRCH);
          return(-1);
       }

       if(mode & O_DUMMY)
          fdes = (-f_index - 1000);
       else
       {  

          /*---------------------------------------------------------*/
          /* FIFOS are always opened read/write irrespective of mode */
          /*---------------------------------------------------------*/

          if(S_ISFIFO(st_mode))
             fdes  = open(f_name,2);
          else
              fdes = open(f_name,mode);
       }

       if(mode & 001000)
          (void)chmod(f_name,0744);


       /*----------------------------------------------------------*/
       /* Stat the file to extract information on permissions and  */
       /* ownership etc from its inode                             */
       /*----------------------------------------------------------*/

       (void)stat(f_name,&buf);
       st_mode = (int32_t)buf.st_mode;

       if(fdes == (-1))
       {

          /*----------------------------------------------------------*/
          /* If permissions are to be forced fix up permissions so we */
          /* can do the desired operation on the file                 */
          /*----------------------------------------------------------*/

          if(fix_perms == LIVE)
          {  if(mode == 0)
                st_mode |= S_IRUSR;
             else if(mode == 1)
                st_mode |= S_IWUSR;
             else if(mode == 2)
                st_mode |= (S_IWUSR | S_IRUSR);

             if(chmod(f_name,st_mode) == (-1))
             {   if(appl_verbose == TRUE)
                 {  (void)fprintf(err_stream,"%sERROR%s pusp_open: problem with permissions for file \"%s\" [%s:%d]\n",
                                                                                                        boldOn,boldOff,
                                                                                             f_name,appl_name,appl_pid);
                    (void)fflush(err_stream);
                 }

                 (void)pups_clear_ftab_slot(FALSE,f_index);
                 (void)pupsrelse(ALL_PUPS_SIGS);
                 pups_set_errno(EACCES);
                 return(-1);
             }
             else
             {  

                /*---------------------------------------------------------*/
                /* FIFOS are always opened read/write irrespective of mode */
                /*---------------------------------------------------------*/

                if(S_ISFIFO(st_mode))
                   fdes = open(f_name,2);
                else
                   fdes = open(f_name,mode);
             }
          }
          else
          {  if(appl_verbose == TRUE)
             {  (void)fprintf(err_stream,errstr,"%sERROR%s pups_open: problem with permissions for file \"%s\" [%s:%d]\n",
                                                                                                           boldOn,boldOff,
                                                                                                f_name,appl_name,appl_pid);
                (void)fflush(err_stream);
             }

             pups_clear_ftab_slot(FALSE,f_index);
             (void)pupsrelse(ALL_PUPS_SIGS);
             pups_set_errno(EACCES);
             return(-1);
          }
       }

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_lock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       ftab[f_index].homeostat = (void *)NULL;
       ftab[f_index].handler   = (void *)NULL;
       ftab[f_index].st_mode   = st_mode;
       ftab[f_index].named     = TRUE;
       ftab[f_index].fdes      = fdes;
       ftab[f_index].mode      = mode;
       ftab[f_index].stream    = (FILE *)NULL;

       (void)strlcpy(ftab[f_index].fname,f_name,SSIZE);


       /*----------------------------------------------------------------------*/
       /* Get information about mount point (if this is a mounted file system) */
       /*----------------------------------------------------------------------*/

       ftab[f_index].mounted = pups_get_fs_mountinfo(f_name,ftab[f_index].rd_host);

       if(mode & O_LIVE || h_state == LIVE && ftab[f_index].psrp == FALSE)
       {  char default_fd_hname[SSIZE] = "";

          ftab[f_index].creator = TRUE;
          (void)snprintf(default_fd_hname,SSIZE,"default_fd_homeostat: (%s) %d",f_name,fdes);
          (void)pups_fd_alive(fdes,default_fd_hname,(void *)&pups_default_fd_homeostat);
       }
    }


    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*------------------------*/
    /* Enable PUPS/P3 signals */
    /*------------------------*/

    (void)pupsrelse(ALL_PUPS_SIGS);

    pups_set_errno(OK);
    return(fdes);
}




/*---------------------------------------*/
/* Close (extended mode) file descriptor */
/*---------------------------------------*/

_PUBLIC int32_t pups_close(const des_t fdes)

{    int32_t i,
             ret,
             my_errno;


    /*-------------------------*/
    /* Disable PUPS/P3 signals */
    /*-------------------------*/

    (void)pupshold(ALL_PUPS_SIGS);


    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    in_close_routine = TRUE;
    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].fdes == fdes)
       {  char f_lock_name[SSIZE] = "";

          if(ftab[i].homeostatic > 0 && ftab[i].psrp == TRUE)
             (void)pups_fd_dead(ftab[i].fdes);


          #ifdef SSH_SUPPORT
          /*-------------------------------------------------------*/
          /* If there is a remote daemon servicing this descriptor */
          /* kill it now                                           */
          /*-------------------------------------------------------*/

          if(ftab[i].rd_pid > 0)
          {  char pidname[SSIZE] = "";

             (void)snprintf(pidname,SSIZE,"%d",-ftab[i].rd_pid);
             (void)pups_rkill(ftab[i].rd_host,ftab[i].rd_ssh_port,appl_owner,pidname,SIGTERM);
          }
          #endif /* SSH_SUPPORT */


          /*------------------------------------------------------------*/
          /* Kill (embedded) pipestream associated with this descriptor */
          /*------------------------------------------------------------*/

          if(ftab[i].fifo_pid > 0 && kill(ftab[i].fifo_pid,SIGALIVE) != (-1))
          {  int32_t status;

             (void)pups_noauto_child();


             /*-------------------------------------------------------------*/
             /* Note that we need to kill the process group of the shell    */
             /* which started the pipestream in order to be sure of killing */
             /* the pipestream command itself                               */
             /*-------------------------------------------------------------*/

             (void)killpg(ftab[i].fifo_pid,SIGTERM);

             (void)pupswait(FALSE,&status);
             (void)pups_auto_child();
          }

          (void)pups_release_fd_lock(ftab[i].fdes);
          do {   ret = close(fdes);
                 my_errno = errno;

                 (void)sched_yield();
             } while(ret == (-1) && my_errno != EBADF);


          /*-----------------------------------------------------------*/
          /* If we have a pipeline attached to this descriptor we must */
          /* confirm that it has exited before continuing              */
          /*-----------------------------------------------------------*/

          if(ftab[i].fifo_pid < 0)
          {  int32_t status;

             (void)pupswaitpid(FALSE,(-ftab[i].fifo_pid),&status);
          } 


          /*-------------------------------------------------*/
          /* Make sure that we do not remove /dev/tty        */
          /* this is possible if application is runnning as  */
          /* suid root and we do not protect /dev/tty with   */
          /* a guard link                                    */
          /*-------------------------------------------------*/

          if(getuid() == 0)
          {  if(strcmp(ftab[i].fshadow,"/dev/tty") == 0)
                (void)link("/dev/tty","/dev/ttyprot");
          }
 
          (void)unlink(ftab[i].fshadow);


          /*------------------------------*/
          /* Restore controlling terminal */
          /*------------------------------*/

          if(getuid() == 0)
          {  if(strcmp(ftab[i].fshadow,"/dev/tty") == 0)
                (void)rename("/dev/ttyprot","/dev/tty");
          }

          pups_clear_ftab_slot(FALSE,i);

          in_close_routine = FALSE;
          (void)pupsrelse(ALL_PUPS_SIGS);

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pupsrelse(ALL_PUPS_SIGS);
          pups_set_errno(OK);

          return(0);
       }
    }

    in_close_routine = FALSE;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*------------------------*/
    /* Enable PUPS/P3 signals */
    /*------------------------*/

    pupsrelse(ALL_PUPS_SIGS);

    pups_set_errno(ESRCH);
    return(-1);
}




/*-------------------------------*/
/* Close (extended mode) stream) */
/*-------------------------------*/

_PUBLIC FILE *pups_fclose(const FILE *stream)

{   int32_t i,
            fdes = (-1);

    FILE    *ret = (FILE *)NULL;


    /*-------------------------*/
    /* Disable PUPS/P3 signals */
    /*-------------------------*/

    (void)pupshold(ALL_PUPS_SIGS);

    if(stream == (const FILE *)NULL)
    {  (void)pupsrelse(ALL_PUPS_SIGS);
       pups_set_errno(EINVAL);
       return((FILE *)NULL);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    in_close_routine = TRUE;
    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].fdes == fileno(stream))
       {  char f_lock_name[SSIZE] = "";

          if(ftab[i].homeostatic > 0 && ftab[i].psrp == FALSE)
             (void)pups_fd_dead(ftab[i].fdes);


          #ifdef SSH_SUPPORT
          /*-------------------------------------------------------*/
          /* If there is a remote daemon servicing this descriptor */
          /* kill it now                                           */
          /*-------------------------------------------------------*/

          if(ftab[i].rd_pid > 0)
          {  char pidname[SSIZE] = "";

             (void)snprintf(pidname,SSIZE,"%d",ftab[i].rd_pid);
             (void)pups_rkill(ftab[i].rd_host,ftab[i].rd_ssh_port,appl_owner,pidname,SIGTERM);
          }
          #endif /* SSH_SUPPORT */


          /*------------------------------------------------------------*/
          /* Kill (embedded) pipestream associated with this descriptor */
          /*------------------------------------------------------------*/

          if(ftab[i].fifo_pid > 0 && kill(ftab[i].fifo_pid,SIGALIVE) != (-1))
          {  int32_t status;

             (void)pups_noauto_child();


             /*-------------------------------------------------------------*/
             /* Note that we need to kill the process group of the shell    */
             /* which started the pipestream in order to be sure of killing */
             /* the pipestream command itself                               */
             /*-------------------------------------------------------------*/

             (void)killpg(ftab[i].fifo_pid,SIGTERM);
             (void)pupswait(FALSE,&status);
             (void)pups_auto_child();
          }

          (void)pups_release_fd_lock(ftab[i].fdes);
          if(ftab[i].mode != O_DUMMY)
          {  (void)fclose(stream);


             /*-----------------------------------------------------------*/
             /* If we have a pipeline attached to this descriptor we must */
             /* confirm that it has exited before continuing              */
             /*-----------------------------------------------------------*/

             if(ftab[i].fifo_pid < 0)
             {  int32_t status;
                (void)pupswaitpid(FALSE,(-ftab[i].fifo_pid),&status);
             }
          }

          (void)unlink(ftab[i].fshadow);
          pups_clear_ftab_slot(FALSE,i);

          in_close_routine = FALSE;

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          (void)pupsrelse(ALL_PUPS_SIGS); 
          pups_set_errno(OK);
          return((FILE *)NULL);
       }
    }

    in_close_routine = FALSE;
    (void)fclose(stream);


    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */
 

    /*------------------------*/
    /* Enable PUPS/P3 signals */
    /*------------------------*/

    (void)pupsrelse(ALL_PUPS_SIGS);

    pups_set_errno(ESRCH);
    return((FILE *)NULL);
}
  
 



/*--------------------------------------------------*/
/* Display extended file descriptors which are open */ 
/*--------------------------------------------------*/

_PUBLIC void pups_show_open_fdescriptors(const FILE *stream)

{   uint32_t i,
             files = 0;

    char     ftype[SSIZE]  = "",
             fstate[SSIZE] = "",
             fbuf[SSIZE]   = "",
             fdid[SSIZE]   = "";


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_open_fdescriptors] attempt by non root thread to perform PUPS/P3 utility operation");

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }


    (void)fprintf(stream,"\n\n    Open file descriptors\n");
    (void)fprintf(stream,"    =====================\n\n");
    (void)fflush(stream);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].fdes != (-1))
       {  struct stat buf;

          ++files;
          if(isatty(ftab[i].fdes) == 1)
             (void)strlcpy(ftype,"TTY  ",SSIZE);
          else
          {  (void)fstat(ftab[i].fdes,&buf);
             if(S_ISFIFO(buf.st_mode))
                (void)strlcpy(ftype,"FIFO ",SSIZE);
             else
             {  if(ftab[i].pheap == FALSE)
                   (void)strlcpy(ftype,"REGF ",SSIZE);
                else
                   (void)strlcpy(ftype,"PHEAP",SSIZE);
             }
          }

          if(ftab[i].homeostatic > 0)
             (void)strlcpy(fstate,"live",SSIZE); 
          else
             (void)strlcpy(fstate,"dead",SSIZE);

          if((void *)ftab[i].homeostat != (void *)NULL)
             (void)strlcat(fstate," dynamic homeostat",SSIZE);

          #ifdef ZLIB_SUPPORT
          if(ftab[i].stream == (FILE *)NULL && ftab[i].zstream == (gzFile *)NULL)
          #else
          if(ftab[i].stream == (FILE *)NULL)
          #endif /* ZLIB_SUPPORT */

             (void)strlcpy(fbuf,"",SSIZE);
          else if(ftab[i].stream != (FILE *)NULL)
             (void)snprintf(fbuf,SSIZE,"(file handle %016lx)", (uint64_t)ftab[i].stream);

          #ifdef ZLIB_SUPPORT
          else if(ftab[i].zstream != (gzFile *)NULL)
             (void)snprintf(fbuf,SSIZE,"(file zhandle %016lx)",(uint64_t)ftab[i].zstream);
          #endif /* ZLIB_SUPPORT */


          /*--------------*/
          /* PSRP channel */
          /*--------------*/

          if(ftab[i].psrp == TRUE)
          {   int32_t idum,
                      schan;

             char     strdum[SSIZE] = "";


             /*-----------------------------------*/
             /* PSRP server primary input channel */
             /*-----------------------------------*/

             if(ftab[i].fname != (char *)NULL && strin(ftab[i].fname,"in") == TRUE)
                (void)strlcpy(fdid,"[PSRP channel in ]",SSIZE);


             /*-----------------------------------*/
             /* PSRP server primary input channel */
             /*-----------------------------------*/

             else if(ftab[i].fname != (char *)NULL && strin(ftab[i].fname,"out") == TRUE)
                (void)strlcpy(fdid,"[PSRP channel out]",SSIZE);


             
             /*---------------- --------------*/
             /* PSRP server SIC input channel */
             /*-------------------------------*/

             else if(ftab[i].fname              != (char *)NULL  &&
                     strin(ftab[i].fname,"in")  == TRUE          &&
                     strin(ftab[i].fname,"sic") == TRUE           )
             {  (void)sscanf (ftab[i].fname,"%s%s%s%s%s%d%d%s%d",strdum,strdum,strdum,strdum,strdum,&idum,&idum,strdum,&schan);
                (void)snprintf(fdid,SSIZE,"[SIC channel %d in ]",schan);
             }


             /*--------------------------------*/
             /* PSRP server SIC output channel */
             /*--------------------------------*/

             else if(ftab[i].fname              != (char *)NULL  &&
                     strin(ftab[i].fname,"out") == TRUE          &&
                     strin(ftab[i].fname,"sic") == TRUE           )
             {  (void)sscanf (ftab[i].fname,"%s%s%s%s%s%d%d%s%d",strdum,strdum,strdum,strdum,strdum,&idum,&idum,strdum,&schan);
                (void)snprintf(fdid,SSIZE,"[SIC channel %d out]",schan);
             }


             /*----------------------*/
             /* Generic PSRP channel */
             /*----------------------*/

             else
                (void)strlcpy(fdid,"[PSRP channel]",SSIZE);
          }
          else
          {  

             #ifdef PERSISTENT_HEAP_SUPPORT
             if(ftab[i].pheap == TRUE) 
             {   int32_t f_index;

                f_index = msm_fdes2hdes(ftab[i].fdes);
                (void)snprintf(fdid,SSIZE,"[peristent heap at %016lx virtual]",(uint64_t)htable[f_index].addr);
             }
             else
             #endif /* PERSISTENT_HEAP_SUPPORT */

             (void)strlcpy(fdid,"",SSIZE);
          }

          (void)fprintf(stream,"    %04d: \"%-48s\"\tfdes %04d,\t(%s, %s) %s %s\n",i,
                                                                       ftab[i].fname,
                                                                        ftab[i].fdes,
                                                                               ftype,
                                                                              fstate,
                                                                                fbuf,
                                                                                fdid);
          (void)fflush(stream);
       }
    }

    if(files == 1)
       (void)fprintf(stream,"\n\n    %04d file slots in use (%04d slots free)\n",1,appl_max_files - files);
    if(files  > 1)
       (void)fprintf(stream,"\n\n    %04d file slots in use (%04d slots free)\n",files,appl_max_files - files);
    else
       (void)fprintf(stream,"    No file slots in use (%04d slots free)\n",appl_max_files,files);

    (void)fflush(stream); 

    if(strcmp(appl_ttyname,"none") == 0)
       (void)fprintf(stream,"\n    Process has no controlling tty\n\n");
    else
    {  if(appl_fgnd == TRUE)
          (void)fprintf(stream,"\n    Controlling tty is \"%s\"\n\n",appl_ttyname);
       else
       {  started_detached = TRUE;
          (void)fprintf(stream,"\n    Controlling tty is \"%s\" [detached]\n\n",appl_ttyname);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    (void)fflush(stream);
    pups_set_errno(OK);
}




/*--------------------------------*/
/* Decode the sign character, '-' */
/*--------------------------------*/

_PUBLIC int32_t chsign(const char ch_arg)

{   pups_set_errno(OK);

    if(ch_arg == '-')
       return((int32_t)-1);
    else
       return((int32_t)1);
}



/*----------------------------------*/
/* Standard error handling routines */
/*----------------------------------*/
/*----------------------------------------------------*/
/* Set PUPS error handler parameters (not threadsafe) */
/*----------------------------------------------------*/

_PUBLIC void pups_seterror(const FILE *stream, const int32_t action, const int32_t code)

{   err_stream = stream;
    err_action = action;
    err_code   = code;
}



/*---------------------------------*/
/* PUPS error handler (threadsafe) */
/*---------------------------------*/

_PUBLIC int32_t pups_error(const char *errstr)

{   if(err_action == NONE)
       return(0); 

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&errstr_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(err_action & PRINT_ERROR_STRING)
    {  (void)fprintf(err_stream,"    %sERROR%s %s [%s (%d@%s:%s)]\n", boldOn,boldOff,errstr,
                                                    appl_name,appl_pid,appl_host,appl_owner);
       (void)fflush(err_stream);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&errstr_mutex);
    #endif /* PHTREAD_SUPPORT */

    if(err_action & EXIT_ON_ERROR)
       pups_exit(err_code);

    if(err_action & CHILD_EXIT_ON_ERROR)
       exit(err_code);

    return(err_code);
}




/*-----------------------------------------------------------------------*/
/* Routine to find any occurence of string s2 in sting s1, the length of */
/* s1 must be greater then s2                                            */
/*-----------------------------------------------------------------------*/

_PUBLIC _BOOLEAN strncmps(const char *s1, const char *s2)

{   size_t template,
           sch_length,
           start  = 0;

    if(s1 == (char *)NULL || s2 == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    sch_length = strlen(s1);
    template   = strlen(s2);


    /*------------------------------------*/
    /* Do not process zero length strings */
    /*------------------------------------*/

    if(sch_length > 0 && template > 0)
    {  pups_set_errno(OK);


       /*----------------------------------------------*/
       /* Test for s1 <= s2, if so flag error and exit */ 
       /*----------------------------------------------*/

       if(sch_length < template)
       {  pups_set_errno(ERANGE);
          return(-1);
       }

       do {  if(strncmp(&s1[start],s2,template) == 0)
               return(TRUE);
             else
                ++start;
          } while(start < sch_length);

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*---------------------------------------------*/
/* Routine to strip an extension from a string */
/*---------------------------------------------*/

_PUBLIC _BOOLEAN strrext(const char *s1, char *s2, const char dm_char)

{   size_t i,
           size;

    if(s1 == (const char *)NULL || s2 == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s1);

    /*-----------------------------*/
    /* Do not process empty string */
    /*-----------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       for(i=0; i<size; ++i)
       {  if(s1[i] == dm_char)
          {  s2[i] = '\0';

             return(TRUE);
          }
          else
             s2[i] = s1[i];
       }

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*-----------------------------------------------------*/
/* Routine to reverse strip an extension from a string */
/*-----------------------------------------------------*/

_PUBLIC _BOOLEAN strrextr(const char *s1, char *s2, const char dm_char)

{   size_t i,
           size;

    if(s1 == (const char *)NULL || s2 == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s1);

    /*-----------------------------*/
    /* Do not process empty string */
    /*-----------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       for(i=strlen(s1); i != 0; --i)
       {  if(s1[i] == dm_char)
          {  (void)strlcpy(s2,(char *)&s1[i+1],SSIZE);
             return(TRUE);
          }
       }

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*----------------------------------------------------------------------*/
/* Routine to extract substrings which are demarkated by a user defined */
/* character                                                            */
/*----------------------------------------------------------------------*/
                                          /*----------------------------------*/
_PUBLIC _BOOLEAN strext(const char dm_ch, /* Demarcation character            */
                        char       *s1,   /* Extracted sub string             */
                        const char *s2)   /* Argument string                  */
                                          /*----------------------------------*/

{   size_t s1_index = 0;

                                          /*---------------------------------*/
    _IMMORTAL size_t s2_index      = 0;   /* Current pointer into arg string */
    _IMMORTAL char   s2_was[SSIZE] = "";  /* Copy of current argument string */
                                          /*---------------------------------*/


    /*---------------------------------------------------------*/
    /* Only the root thread perform non thread-safe operations */
    /*---------------------------------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[strext] attempt by non root thread to perform PUPS/P3 non thread safe operation");

    if(s1 == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);


    /*-------------------------------------------------------------------*/
    /* Entry with null parameters forces reset of pointers within strext */
    /* function.                                                         */
    /*-------------------------------------------------------------------*/

    if(s2 == (const char *)NULL || strlen(s2) == 0)
    {  s2_index = 0;
       return(FALSE);
    }


    /*--------------------------------------------------------------------*/
    /* Save string length for swap test if this is first pass for current */
    /* extraction string.                                                 */
    /*--------------------------------------------------------------------*/

    if(s2_index == 0)
       (void)strlcpy(s2_was,s2,SSIZE);
    else
    {

       /*-------------------------------------------------*/
       /* If extraction string has changed re-initialise. */
       /*-------------------------------------------------*/

       if(strcmp(s2_was,s2) != 0)
       {  s2_index = 0;
          (void)strlcpy(s2_was,s2,SSIZE);
       }
    }


    /*------------------------------------*/
    /* Wind to substring to be extracted. */
    /*------------------------------------*/

    while(s2[s2_index] == dm_ch && s2[s2_index] != '\0' && s2[s2_index] != '\n')
          ++s2_index;


    /*---------------------------------------------------------*/
    /* If we have reached the end of the string - reinitialise */
    /*---------------------------------------------------------*/

    if(s2[s2_index] == '\0' || s2[s2_index] == '\n')
    {  s2_index = 0;
       s1[0]    = '\0';
       return(FALSE);
    }


    /*--------------------------------------------------*/
    /* Extract substring to next demarcation character. */
    /*--------------------------------------------------*/

    while(s2[s2_index] != dm_ch && s2[s2_index] != '\0' && s2[s2_index] != '\n')
    {     s1[s1_index] = s2[s2_index];
          ++s1_index;
          ++s2_index;
    }
    s1[s1_index] = '\0';


    /*--------------------------------------------------------------------*/
    /* If the end of the extraction string has been reached, reinitialise.*/
    /*--------------------------------------------------------------------*/

    if(s2[s2_index] == '\0' || s2[s2_index] == '\n')
    {  s2_index = 0;

                              /*----------------------------------------------*/
       return(END_STRING);    /* Messy but some applications need to know if  */
                              /* dm_ch was really matched or if we have hit   */
                              /* the end of the string                        */
                              /*----------------------------------------------*/
    }

    return(TRUE);
}




/*----------------------------------------------------------------------*/
/* Routine to extract substrings which are demarcated by a user defined */
/* character for multiple comparison operations                         */
/*----------------------------------------------------------------------*/
                                                /*--------------------------------*/
_PUBLIC _BOOLEAN m_strext(const  int32_t i_key, /* Instance key for operation     */
                          const char     dm_ch, /* Demarcation character          */
                          char             *s1, /* Extracted sub string           */
                          const char       *s2) /* Argument string                */
                                                /*--------------------------------*/

{   size_t i,
           s1_index = 0;

    _IMMORTAL _BOOLEAN entered = FALSE;                  /*---------------------------------*/ 
    _IMMORTAL size_t   s2_index[N_STREXT_STRINGS];       /* Current pointer into arg string */
    _IMMORTAL char     s2_was[N_STREXT_STRINGS][SSIZE];  /* Copy of current argument string */
                                                         /*---------------------------------*/


    /*---------------------------------------------------------*/
    /* Only the root thread perform non thread-safe operations */
    /*---------------------------------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[m_strext] attempt by non root thread to perform PUPS/P3 non thread safe operation");


    /*--------------*/
    /* Sanity check */
    /*--------------*/

    if(s1 == (char *)NULL && dm_ch != MSTREXT_RESET_ALL && dm_ch != MSTREXT_RESET_CUR)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);


    /*----------------------------------------------------------------*/
    /* If dm_ch is '*', '@' re-initialise all or the i_key'th channel */
    /* respectively.                                                  */
    /*----------------------------------------------------------------*/
    /*------------*/
    /* Initialise */
    /*------------*/

    if(entered == FALSE)
    {  for(i=0; i<N_STREXT_STRINGS; ++i)
       {  s2_index[i] = 0;
          (void)strlcpy(s2_was[i],"",SSIZE);
       }

       entered = TRUE;


       /*-----------------------*/
       /* First call is a reset */
       /*-----------------------*/

       if(dm_ch == MSTREXT_RESET_ALL || dm_ch != MSTREXT_RESET_CUR)
          return(FALSE);
    }


    /*-----------------------*/
    /* Reset all key strings */
    /*-----------------------*/

    else if(dm_ch == MSTREXT_RESET_ALL)
    {  for(i=0; i<N_STREXT_STRINGS; ++i)
       {  s2_index[i] = 0;
          (void)strlcpy(s2_was[i],"",SSIZE);
       }

       return(FALSE);
    }


    /*--------------------------*/
    /* Reset current key string */
    /*--------------------------*/

    else if(dm_ch == MSTREXT_RESET_CUR)
    {  s2_index[i_key] = 0;
       (void)strlcpy(s2_was[i_key],"",SSIZE);

       return(FALSE);
    }


    /*---------------------*/
    /* Check key is valid. */
    /*---------------------*/

    if(i_key > N_STREXT_STRINGS)
       return(STREXT_ERROR);


    /*--------------------------------------------------------------------*/
    /* Save string length for swap test if this is first pass for current */
    /* extraction string.                                                 */
    /*--------------------------------------------------------------------*/

    if(s2_index[i_key] == 0)
       (void)strlcpy(s2_was[i_key],s2,SSIZE);
    else
    {

    /*-------------------------------------------------*/
    /* If extraction string has changed re-initialise. */
    /*-------------------------------------------------*/

       if(strcmp(s2_was[i_key],s2) != 0)
       {  s2_index[i_key] = 0;
          (void)strlcpy(s2_was[i_key],s2,SSIZE);
       }
    }


    /*--------------------------------------------------*/
    /* Extract substring to next demarcation character. */
    /*--------------------------------------------------*/

    while(s2[s2_index[i_key]] != dm_ch &&
          s2[s2_index[i_key]] != '\0'  &&
          s2[s2_index[i_key]] != '\n'   )
    {
          s1[s1_index] = s2[s2_index[i_key]];
          ++s1_index;
          ++s2_index[i_key];
    }
    s1[s1_index] = '\0';


    /*--------------------------------------------------------------------*/
    /* If the end of the extraction string has been reached, reinitialise */
    /*--------------------------------------------------------------------*/

    if(s2[s2_index[i_key]] == '\0' || s2[s2_index[i_key]] == '\n' || s2_index[i_key] == strlen(s2))
    {  s2_index[i_key] = 0;

                              /*---------------------------------------------*/
       return(END_STRING);    /* Messy but some applications need to know if */
                              /* dm_ch was really matched or if we have hit  */
                              /* the end of the string                       */
                              /*---------------------------------------------*/
    }

    ++s2_index[i_key];
    return(TRUE);
}




/*-------------------------------------------------------------------*/
/* Routine to copy a string filtering out characters which have been */
/*  marked as excluded                                               */
/*-------------------------------------------------------------------*/

_PUBLIC ssize_t strexccpy(const char *s1, char *s2, const char *ex_ch)

{   size_t i,
           j,
           k,
           size_1,
           size_2;

    if(s1    == (const char *)NULL    ||
       s2    == (char *)      NULL    ||
       ex_ch == (const char *)NULL     )
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size_1 = strlen(s1);
    size_2 = strlen(ex_ch);


    /*-----------------------------*/
    /* Do not process empty string */
    /*-----------------------------*/

    if(size_1 > 0 && size_2 > 0)
    {
       j = 0;
       for(i=0; i<size_1; ++i)
       {   for(k=0; k<size_2; ++k) 
           {  if(s1[i] == ex_ch[k])
                 goto exclude;
           }

           s2[j] = s1[i];
           ++j;

exclude:   continue;

       }

       pups_set_errno(OK);
       return(j);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*-----------------------------------------------------------------*/
/* Routine to return the position of a character within a string   */
/* (distance from start of string). If the character is not in the */
/* string (-1) is returned                                         */
/*-----------------------------------------------------------------*/

_PUBLIC ssize_t ch_pos(const char *s, const char ch)

{   size_t i,
           size;

    if(s == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);


    /*-----------------------------*/
    /* Do not process empty string */
    /*-----------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       for(i=0; i<size; ++i)
       {  if(s[i] == ch)
             return((int32_t)i);
       }
 
       return(-1);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*-----------------------------------------------------------------------*/
/* Routine to return the reverse position of a character within a string */
/* (distance from end of string). If the character is not in the string  */
/* (-1) is returned                                                      */ 
/*-----------------------------------------------------------------------*/

_PUBLIC ssize_t rch_pos(const char *s, const char ch)

{   size_t i,
           size;

    if(s == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);


    /*-----------------------------*/
    /* Do not process empty string */
    /*-----------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       for(i=strlen(s)-1; i != 0; --i)
       {  if(s[i] == ch)
             return((int32_t)i);
       }

       return(-1);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*--------------------------------------------------------------------*/
/* Test for empty string (contains only whitespace and control chars) */
/*--------------------------------------------------------------------*/

_PUBLIC _BOOLEAN strempty(const char *s)

{   size_t i,
           size;

    if(s == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);


    /*------------------------------*/
    /* Do not process empnty string */
    /*------------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       for(i=0; i<strlen(s); ++i)
       {  if(s[i] != ' ' && s[i] != '\n')
             return(FALSE);
       }

       return(TRUE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*----------------------------------------------------------*/
/* Generate random string containing hexidecimal characters */
/*----------------------------------------------------------*/

_PUBLIC int32_t strand(const size_t size, char *s)

{   size_t  i;
    char    next_char;
    int16_t next_digit;


    if(s == (const char *)NULL || size == 0)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*--------------------------------------------------------------*/
    /* Populate string with random (hexidecimal) numeric characters */
    /*--------------------------------------------------------------*/

    for(i=0; i<size; ++i)
    {  next_digit = (int32_t)FLOOR(drand48()*16.0);
       switch(next_digit)
       {     case 0:  next_char  = '0';
                      break;

             case 1:  next_char  = '1';
                      break;

             case 2:  next_char  = '2';
                      break;

             case 3:  next_char  = '3';
                      break;

             case 4:  next_char  = '4';
                      break;

             case 5:  next_char  = '5';
                      break;

             case 6:  next_char  = '6';
                      break;

             case 7:  next_char  = '7';
                      break;

             case 8:  next_char  = '8';
                      break;

             case 9:  next_char  = '9';
                      break;

             case 10:  next_char = 'a';
                       break;

             case 11:  next_char = 'b';
                       break;

             case 12:  next_char = 'c';
                       break;

             case 13:  next_char = 'd';
                       break;

             case 14:  next_char = 'e';
                       break;

             case 15:  next_char = 'f';
                       break;


             default: break;
       }

       s[i] = next_char;
    }


    /*------------------*/
    /* Terminate string */
    /*------------------*/

    s[i] = '/0'; 

    pups_set_errno(OK);
    return(0);
}



 
/*-------------------------------------------------------*/
/* Extract substrings demarcated by arbitrary characters */
/*-------------------------------------------------------*/
                                                  /*--------------------------------*/
_PUBLIC _BOOLEAN mdc_strext(const char   *dm_ch,  /* Demarcation characters string  */
                            int32_t      *dm_bit, /* Bit codes for matched dm chars */
                            char         *s1,     /* Extracted sub string           */
                            const char   *s2)     /* Argument string                */
                                                  /*--------------------------------*/

{   size_t dm_ch_index,
           s1_index            = 0;
                                               /*---------------------------------*/
    _IMMORTAL size_t s2_index  = 0;            /* Current pointer into arg string */
    _IMMORTAL char *s2_was     = (char *)NULL; /* Copy of current argument string */
                                               /*---------------------------------*/


    /*---------------------------------------------------------*/
    /* Only the root thread perform non thread-safe operations */
    /*---------------------------------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[strext] attempt by non root thread to perform PUPS/P3 non thread safe operation");

    if(s1     == (char *)NULL          ||
       s2     == (const char *)NULL    ||
       dm_ch  == (const char *)NULL    ||
       dm_bit == (int32_t *)   NULL     )
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);


    /*--------------------------------------------------------------------*/
    /* Save string length for swap test if this is first pass for current */
    /* extraction string.                                                 */
    /*--------------------------------------------------------------------*/

    if(s2_index == 0)
    {  s2_was = (char *)pups_malloc(SSIZE);
       (void)strlcpy(s2_was,s2,SSIZE);
    }
    else
    {

       /*------------------------------------------------*/
       /* If extraction string has changed re-initialise */
       /*------------------------------------------------*/

       if(strcmp(s2_was,s2) != 0)
       {  s2_index = 0;
          (void)strlcpy(s2_was,s2,SSIZE);
       }
    }


    /*-----------------------------------*/
    /* Wind to substring to be extracted */
    /*-----------------------------------*/

    while((dm_ch_index = ch_pos(dm_ch,s2[s2_index])) != (-1) &&
          s2[s2_index] != '\0'                               &&
          s2[s2_index] != '\n'                                )
    {     *dm_bit |= (int32_t)pow(2.0,(FTYPE)dm_ch_index);
          ++s2_index;
    }


    /*--------------------------------------------------------------------*/
    /* If we have reached the end of the extraction string - reinitialise */
    /*--------------------------------------------------------------------*/

    if(s2[s2_index] == '\0' || s2[s2_index] == '\n')
    {  s2_index = 0;
       (void)pups_free(s2_was);
       return(FALSE);
    }


    /*-------------------------------------------------*/
    /* Extract substring to next demarcation character */
    /*-------------------------------------------------*/

    if(ch_pos(dm_ch,s2[s2_index]) != (-1))
       ++s2_index;

    while((dm_ch_index = ch_pos(dm_ch,s2[s2_index])) == (-1)      &&
          s2[s2_index] != '\0' && s2[s2_index] != '\n'             )
    {
          s1[s1_index] = s2[s2_index];
          ++s1_index;
          ++s2_index;
    }
    s1[s1_index] = '\0';


    /*--------------------------------------------------------------------*/
    /* If the end of the extraction string has been reached, reinitialise */
    /*--------------------------------------------------------------------*/

    if(s2[s2_index] == '\0' || s2[s2_index] == '\n')
    {  s2_index = 0;
       (void)pups_free(s2_was);
       return(FALSE);
    }
    else
    {  *dm_bit |= (int32_t)pow(2.0,(FTYPE)dm_ch_index);
       return(TRUE);
    }

    return(-1);
}




/*-----------------------------------------------------------------*/
/* Extract substrings which are demarcated by arbitrary characters */
/* ----------------------------------------------------------------*/
                                                    /*--------------------------------------*/
_PUBLIC _BOOLEAN mdc_strext2(const char  *dm_ch,    /* Demarcation characters string        */
                             int32_t     *dm_bit_l, /* Bit codes for left matched dm chars  */
                             int32_t     *dm_bit_r, /* Bit codes for right matched dm chars */
                             size_t      *pos,      /* Current position in argument string  */
                             char        *s1,       /* Extracted sub string                 */
                             const char  *s2)       /* Argument string                      */
                                                    /*--------------------------------------*/

{   size_t dm_ch_index,
           s1_index = 0;

    _IMMORTAL _BOOLEAN entered = FALSE;
                                               /*--------------------------------------*/
    _IMMORTAL size_t s2_index  = 0;            /* Current pointer into arg string      */
    _IMMORTAL char   *s2_was = (char *)NULL;;  /* Copy of current argument string      */
                                               /*--------------------------------------*/


    /*---------------------------------------------------------*/
    /* Only the root thread perform non thread-safe operations */
    /*---------------------------------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[strext] attempt by non root thread to perform PUPS/P3 non thread safe operation");


    if(s1       == (char *)      NULL    ||
       s2       == (const char *)NULL    ||
       dm_ch    == (const char *)NULL    ||
       dm_bit_l == (int32_t *)   NULL    ||
       dm_bit_r == (int32_t *)   NULL    ||
       pos      == (int32_t *)   NULL     )
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);

    if(entered == FALSE)
       entered = TRUE;
    else
       --s2_index;

    if(s1 == (char *)NULL && s2 == (const char *)NULL)
    {  s2_index = 0;
       *pos = s2_index;
       return(FALSE);
    }


    /*--------------------------------------------------------------------*/
    /* Save string length for swap test if this is first pass for current */
    /* extraction string.                                                 */
    /*--------------------------------------------------------------------*/

    if(s2_index == 0)
    {  s2_was = (char *)pups_malloc(SSIZE);
       (void)strlcpy(s2_was,s2,SSIZE);
    }
    else
    {

       /*------------------------------------------------*/
       /* If extraction string has changed re-initialise */
       /*------------------------------------------------*/

       if(strcmp(s2_was,s2) != 0)
       {  s2_index = 0;
          (void)strlcpy(s2_was,s2,SSIZE);
       }
    }


    /*-----------------------------------*/
    /* Wind to substring to be extracted */
    /*-----------------------------------*/


    while((dm_ch_index = ch_pos(dm_ch,s2[s2_index])) != (-1) &&
          s2[s2_index] != '\0'                               &&
          s2[s2_index] != '\n'                                )
    {    *dm_bit_l |= (int32_t)pow(2.0,(FTYPE)dm_ch_index); 
         ++s2_index;
    }


    /*--------------------------------------------------------------------*/
    /* If we have reached the end of the extraction string - reinitialise */
    /*--------------------------------------------------------------------*/

    if(s2[s2_index] == '\0' || s2[s2_index] == '\n')
    {  s2_index = 0;
       *pos = s2_index;
       (void)pups_free(s2_was);
       return(FALSE);
    }


    /*-------------------------------------------------*/
    /* Extract substring to next demarcation character */
    /*-------------------------------------------------*/

    if(ch_pos(dm_ch,s2[s2_index]) != (-1))
       ++s2_index;

    while((dm_ch_index = ch_pos(dm_ch,s2[s2_index])) == (-1)      &&
          s2[s2_index] != '\0' && s2[s2_index] != '\n'           )
    {     s1[s1_index] = s2[s2_index];
          ++s1_index;
          ++s2_index;
    }
    s1[s1_index] = '\0';


    /*--------------------------------------------------------------------*/
    /* If the end of the extraction string has been reached, reinitialise */
    /*--------------------------------------------------------------------*/

    if(s2[s2_index] == '\0' || s2[s2_index] == '\n')
    {  s2_index = 0;
       *pos = s2_index;
       *dm_bit_r |= (int32_t)pow(2.0,(FTYPE)dm_ch_index);
       (void)pups_free(s2_was);
       return(FALSE);
    }
    else
    {  *dm_bit_r |= (int32_t)pow(2.0,(FTYPE)dm_ch_index);
       ++s2_index;
       *pos = s2_index;
       return(TRUE);
    }

    return(-1);
}




/*--------------------------------------------------------------------------------*/
/* Look for the occurence of string s2 within string s1 (retruning index of start */
/* of string 1 within string 2)                                                   */
/*--------------------------------------------------------------------------------*/

_PUBLIC _BOOLEAN strinp(size_t *c_index, const char *s1, const char *s2)

{   size_t i,
           size_1,
           size_2,
           chk_limit;

    if(s1 == (const char *)NULL || s2 == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if(c_index != (uint64_t *)NULL)
       *c_index = 0;

    size_1    = strlen(s1);
    size_2    = strlen(s2);


    /*----------------------------------*/
    /* Do not process zero size strings */
    /*----------------------------------*/

    if(size_1 > 0 && size_2 > 0)
    {  if(size_2 > size_1)
       {  pups_set_errno(ERANGE);
          return(-1);
       }

       pups_set_errno(OK);
       chk_limit = strlen(s1) - strlen(s2) + 1;

       for(i=0; i<chk_limit; ++i)
       {  if(strncmp(&s1[i],s2,size_2) == 0)
          {  if(c_index != (uint64_t *)NULL)
                *c_index = i;

             return(TRUE);
          }
       }

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}



/*-----------------------------------------------------------------------------*/
/* Case insensitive comparison of two strings (up to n significant characters) */
/*-----------------------------------------------------------------------------*/

_PUBLIC int32_t strncmpi(const char *s1, const char *s2, size_t n)
{
    while(n--)
    {
        char c1 = *s1,
             c2 = *s2;

        if (c1 == '\0' && c2 == '\0')
           break;

        if (isalpha(c1) && isupper(c1))
           c1 = tolower(c1);

        if (isalpha(c2) && isupper(c2))
           c2 = tolower(c2);

        if (c1 < c2)
           return (-1);

        if (c1 > c2)
           return (1);

        s1++; s2++;
    }

    return (0);
}




/*------------------------------*/
/* convert string to upper case */
/*------------------------------*/

_PUBLIC char *strlower (const char *in)
{
    char *s = (char *)NULL;

    for (s = in; *s; s++)
    {   if (isupper(*s))
           *s = tolower(*s);
    }

    return (in);
}





/*------------------------------*/
/* convert string to upper case */
/*------------------------------*/

_PUBLIC char *strtoupper(const char *in)
{
    char *s = (char *)NULL;

    for (s = in; *s; s++)
    {   if (islower(*s))
           *s = toupper(*s);
    }

    return (in);
}




/*------------------------------------------------------*/
/* Look for the occurence of string s2 within string s1 */
/*------------------------------------------------------*/

_PUBLIC _BOOLEAN strin(const char *s1, const char *s2)

{   size_t i,
           size_1,
           size_2,
           chk_limit;

    if(s1 == (const char *)NULL || s2 == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size_1 = strlen(s1);
    size_2 = strlen(s2);


    /*---------------------------------------*/
    /* Don't not process zero length strings */
    /*---------------------------------------*/

    if(size_1 > 0 && size_2 > 0)
    {
       if(size_2 > size_1)
       {  pups_set_errno(ERANGE);
          return(FALSE);
       }

       pups_set_errno(OK);
       chk_limit = strlen(s1) - strlen(s2) + 1;

       for(i=0; i<chk_limit; ++i)
       {  if(strncmp(&s1[i],s2,size_2) == 0)
             return(TRUE);
       }

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}



/*--------------------------------------------------*/
/* Return tail of string s1 beginning at string s2  */
/*--------------------------------------------------*/

_PUBLIC char *strin2(const char *s1, const char *s2)

{   size_t i,
           size_1,
           size_2,
           chk_limit;

    if(s1 == (const char *)NULL || s2 == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return((char *)NULL);
    }

    size_1 = strlen(s1);
    size_2 = strlen(s2);


    /*-------------------------------------*/
    /* Don not process zero length strings */
    /*-------------------------------------*/

    if(size_1 > 0 && size_2 > 0)
    {  if(size_2 > size_1)
       {  pups_set_errno(ERANGE);
          return((char *)NULL);
       }

       pups_set_errno(OK);
       chk_limit = size_1 - size_2 + 1;

       for(i=0; i<chk_limit; ++i)
       {  if(strncmp(&s1[i],s2,size_2) == 0)
             return((char *)&s1[i+size_2]);
       }

       return((char *)NULL);
    }

    pups_set_errno(ERANGE);
    return((char *)NULL);
}




/*-----------------------------------------------------------------------------*/
/* Look for character token demarcated occurence of string s2 within string s1 */
/*-----------------------------------------------------------------------------*/

_PUBLIC _BOOLEAN strintok(const char *s1, const char *s2, const char *dm_chars)

{   size_t i,
           j,
           k,
           size_1,
           size_2,
           chk_limit;

    char dm_char_l = '\0',
         dm_char_r = '\0';

    _BOOLEAN left_tokenised,
             right_tokenised;

    if(s1 == (const char *)NULL || s2 == (const char *)NULL || dm_chars == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size_1 = strlen(s1);
    size_2 = strlen(s2);


    /*------------------------------------*/
    /* Do not process zero length strings */
    /*------------------------------------*/

    if(size_1 > 0 && size_2 > 0)
    {  if(size_2 > size_1)
       {  pups_set_errno(ERANGE);
          return(-1);
       }

       pups_set_errno(OK);
       chk_limit = size_1 - size_2 + 1;

       for(i=0; i<chk_limit; ++i)
       {  left_tokenised  = FALSE;
          right_tokenised = FALSE;

          if(strncmp(&s1[i],s2,size_2) == 0)
          {  if(i + size_2 < size_1)
             {  dm_char_r = s1[i + size_2];

                for(j=0; j<strlen(dm_chars); ++j)
                    if(dm_char_r == dm_chars[j])
                       right_tokenised = TRUE;
             }
             else
                right_tokenised = TRUE;

             if(i - 1 >= 0)
             {  dm_char_l = s1[i-1];
          
                for(j=0; j<strlen(dm_chars); ++j)
                   if(dm_char_l == dm_chars[j])
                      left_tokenised = TRUE;
             }
             else
                left_tokenised = TRUE;

             if(left_tokenised == TRUE && right_tokenised == TRUE)
                return(TRUE);
          }
       }

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*---------------------------------------------------------------*/
/* Check if string 3 is to the right of string 2 within string 1 */
/*---------------------------------------------------------------*/

_PUBLIC _BOOLEAN stright(const char *s1, const char *s2, const char *s3)

{   size_t i,
           size_1,
           size_2,
           chk_limit;

    if(s1 == (const char *)NULL || s2 == (const char *)NULL || s3 == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size_1 = strlen(s1);
    size_2 = strlen(s2);


    /*------------------------------------*/
    /* Do not process zero length strings */
    /*------------------------------------*/

    if(size_1 > 0 && size_2 > 0)
    {  if(size_2 > size_1)
       {  pups_set_errno(ERANGE);
          return(FALSE);
       }

       chk_limit = size_1 - size_2 + 1;

       for(i=0; i<chk_limit; ++i)
       {  if(strncmp(&s1[i],s2,size_2) == 0)
          {  pups_set_errno(OK);
 
             if(strin(&s1[i + size_2],s3) == TRUE)
                return(TRUE);
             else
                return(FALSE);
          }
       }

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*---------------------------------------------*/
/* Replace sub-string with constant characters */
/*---------------------------------------------*/

_PUBLIC _BOOLEAN strepch(char *s1, const char *s2, const char rep_ch)

{   size_t i,
           j,
           size_1,
           size_2,
           scan_size;

    if(s1 == (char *)NULL || s2 == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size_1 = strlen(s1);
    size_2 = strlen(s2);

    if(size_1 > 0 && size_2 > 0)
    {  scan_size = size_1 - size_2;

       pups_set_errno(OK);
       for(i=0; i<scan_size; ++i)
       {  if(strncmp(&s1[i],s2,size_2) == 0)
          {  for(j=i; j<i + size_2; ++j)
                s1[j] = rep_ch;

             return(TRUE);
          }
       }

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}

    


/*-------------------------------------------------------------------------*/
/* Globally eplace all occurences of string s3 in string s1 with string s4 */
/* returning result in string s2                                           */
/*-------------------------------------------------------------------------*/

_PUBLIC int32_t strep(const char *s1, char *s2, const char *s3, const char *s4)

{   size_t i,
           j,
           out_index,
           size_1,
           size_2,
           cmp_size,
           rep_size;

    if(s1 == (const char *)NULL    ||
       s2 == (char       *)NULL    ||
       s3 == (const char *)NULL    ||
       s4 == (const char *)NULL     )
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    size_1   = strlen(s1);
    size_2   = strlen(s1);
    cmp_size = strlen(s3);
    rep_size = strlen(s4);


    /*------------------------------------*/
    /* Do not process zero length strings */
    /*------------------------------------*/

    if(size_1 > 0 && size_2 > 0 && cmp_size > 0 && rep_size > 0)
    {  if(size_2 > size_2)
       {  pups_set_errno(ERANGE);
          return(-1);
       }


       /*--------------------------*/
       /* Initialise output string */
       /*--------------------------*/

       pups_set_errno(OK);
       (void)strlcpy(s2,"",SSIZE);

       out_index = 0;

       for(i=0; i<size_1; ++i)
       {  if(strncmp(&s1[i],s3,cmp_size) == 0)
          {  s2[out_index] = '\0';

             (void)strlcat(s2,s4,SSIZE);

             out_index += rep_size;
             i         += cmp_size - 1;
          }

          /*--------------------------------------------------------------*/
          /* Cannot find valid demarcation character - simply copy string */
          /*--------------------------------------------------------------*/

          else
          {   s2[out_index] = s1[i];
              ++out_index;
          }
       }

       return(0);
    }

    s2[out_index] = '\0';
    pups_set_errno(ERANGE);
    return(-1);
}




/*------------------------------------------------------------------------*/
/* Globally eplace all token demarcated occurences of string s3 in string */
/* s1 with string s4 returning result in string s2                        */
/*------------------------------------------------------------------------*/

_PUBLIC int32_t streptok(const char *s1, char *s2, const char *s3, const char *s4, const char *dm_chars)

{   size_t i,
           j,
           out_index,
           size_1,
           size_2,
           dm_chars_size,
           cmp_size,
           rep_size;

    char dm_char_l = '\0',
         dm_char_r = '\0';

    _BOOLEAN left_tokenised,
             right_tokenised;

    if(s1       == (const char *)NULL    ||
       s2       == (char       *)NULL    ||
       s3       == (const char *)NULL    ||
       s4       == (const char *)NULL    ||
       dm_chars == (const char *)NULL     )
    {  pups_set_errno(EINVAL);
       return(-1);
    }
 
    size_1        = strlen(s1);
    size_2        = strlen(s1);
    dm_chars_size = strlen(dm_chars);
    cmp_size      = strlen(s3);
    rep_size      = strlen(s4);


    /*------------------------------------*/
    /* Do not process zero length strings */
    /*------------------------------------*/

    if(size_1 > 0 && size_2 > 0 && dm_chars_size > 0 && cmp_size > 0 && rep_size > 0)
    {  if(size_2 > size_1)
       {  pups_set_errno(ERANGE);
          return(-1);
       }


       /*--------------------------*/
       /* Initialise output string */
       /*--------------------------*/

       pups_set_errno(OK);
       (void)strlcpy(s2,"",SSIZE);

       out_index = 0;

       for(i=0; i<size_1; ++i)
       {  if(strncmp(&s1[i],s3,cmp_size) == 0)
          {  

             /*--------------------------------------------------------------*/
             /* Check that valid demarcation character appended to string s2 */
             /*--------------------------------------------------------------*/

             left_tokenised  = FALSE;
             right_tokenised = FALSE;
             if(i + cmp_size < size_1)
             {  dm_char_r = s1[i + cmp_size];

                for(j=0; j<dm_chars_size; ++j)
                    if(dm_chars[j] == dm_char_r)   
                       right_tokenised = TRUE;           
             }
             else
                right_tokenised = TRUE;

             if(i - 1 >= 0)
             {  dm_char_l = s1[i - 1];

                for(j=0; j<dm_chars_size; ++j)
                    if(dm_chars[j] == dm_char_l)   
                       left_tokenised = TRUE;           
             }
             else
                left_tokenised = TRUE;

             if(left_tokenised == TRUE && right_tokenised == TRUE)
             {  s2[out_index] = '\0';

                (void)strlcat(s2,s4,SSIZE);
                out_index += rep_size;
                i         += cmp_size - 1;
             }


             /*--------------------------------------------------------------*/
             /* Cannot find valid demarcation character - simply copy string */
             /*--------------------------------------------------------------*/

             else
                 s2[out_index] = s1[i];

              ++out_index;
          }
          else
          {  s2[out_index] = s1[i];
             ++out_index;
          }
       }

       return(0);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*--------------------------------------------------------------------------------------*/
/* Reverse truncate string at demarcation character (c_cnt instance from end of string) */
/*--------------------------------------------------------------------------------------*/

_PUBLIC _BOOLEAN strtrnc(char *s, const char trunc_c, size_t c_cnt)

{   size_t f_cnt   = 0,
           s_index = 0;

    if(s == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    s_index = strlen(s);


    /*------------------------------------*/
    /* Do not try to process empty string */
    /*------------------------------------*/

    if(s_index > 0)
    {

       /*--------------------------------------------*/
       /* Search for specified demarcation character */
       /*--------------------------------------------*/

       pups_set_errno(OK);
       while(s_index > 0)
       {  if(s[s_index] == trunc_c)
             ++f_cnt;


          /*-----------------------------------------------*/
          /* We have found specified demarcation character */
          /* return truncated string                       */
          /*-----------------------------------------------*/

          if(f_cnt == c_cnt)
          {  s[s_index] = '\0';
             return(TRUE);
          }

          --s_index;
       }

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*--------------------------------------------------------------------------------*/
/* Truncate string at demarcation character (c_cnt instance from start of string) */
/*--------------------------------------------------------------------------------*/

_PUBLIC _BOOLEAN strtrnch(char *s, const char trunc_c, size_t c_cnt)

{   size_t size,
           f_cnt   = 0,
           s_index = 0;

    if(s == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    s_index = 0;
    size    = strlen(s);


    /*-----------------------------*/
    /* Do not process empty string */
    /*-----------------------------*/

    if(size > 0)
    {

       /*--------------------------------------------*/
       /* Search for specified demarcation character */
       /*--------------------------------------------*/

       while(s_index < size && f_cnt != c_cnt)
       {   if(s[s_index] == trunc_c)
              ++f_cnt;
         ++s_index;
       }


       /*-----------------------------------------------*/
       /* We have found specified demarcation character */
       /* return truncated string                       */
       /*-----------------------------------------------*/

       pups_set_errno(OK);
       if(f_cnt == c_cnt)
       {  s[s_index-1] = '\0';
          return(TRUE);
       }

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}



/*--------------------------------------------------*/
/* Extract string starting at demarcation character */
/*--------------------------------------------------*/

_PUBLIC _BOOLEAN strfrm(const char *s1, const char start_c, const int32_t c_cnt, char *s2)

{   size_t size_1,
           f_cnt   = 0,
           s_index = 0;

    if(s1 == (const char *)NULL || s2 == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size_1 = strlen(s1);


    /*----------------------------------*/
    /* Don't process zero length string */
    /*----------------------------------*/

    if(size_1 > 0)
    {  pups_set_errno(OK);
      
       if(c_cnt == 0)
          (void)strlcpy(s2,s1,SSIZE);
       else
       {  while(s_index < size_1 && f_cnt != c_cnt)
          {   if(s1[s_index] == start_c)
                 ++f_cnt;
               ++s_index;
          }

          (void)strlcpy(s2,&s1[s_index],SSIZE);
       }

       if(f_cnt == c_cnt)
         return(TRUE);

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*----------------------------------------------------*/
/* Strip trailing characters to demarcation character */
/*----------------------------------------------------*/

_PUBLIC _BOOLEAN strail(char *s, const char c)

{   size_t i,
           size;

    _BOOLEAN has_linefeed = FALSE;

    if(s == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);

    /*-----------------------------*/
    /* Do not process empty string */
    /*-----------------------------*/

    if(size >= 1)
    {  pups_set_errno(OK);

       --size;

       for(i=size; i>=0; --i)
       {  if(s[i] != c && s[i] != '\n' && s[i] != '\0') 
             return(TRUE);
          else if(s[i] == c)
          {  if(has_linefeed == TRUE)
             {  s[i] = '\n';
                s[i+1] = '\0';
             }
             else
                s[i] = '\0';

             return(TRUE);
          } 

          if(s[i] == '\n')
             has_linefeed = TRUE;
       }

       return(FALSE);
    }

     pups_set_errno(ERANGE);
    return(-1);
}



/*-------------------------------------------*/
/* Extract leaf from pathname (/branch/leaf) */
/*-------------------------------------------*/

_PUBLIC _BOOLEAN strleaf(const char *pathname, char *leaf)

{   size_t   i;
    _BOOLEAN ret = FALSE;

    if(pathname == (char *)NULL || leaf == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    for(i=strlen(pathname); i> 0; --i)
    {  if(pathname[i] == '/')
       {  ret = TRUE;
          break;
       }
    }


    /*------*/
    /* Leaf */
    /*------*/

    if(i > 0)
       (void)strlcpy(leaf,(char *)&pathname[i+1],SSIZE);


    /*---------*/
    /* No leaf */
    /*---------*/

    else
       (void)strlcpy(leaf,pathname,SSIZE);

    pups_set_errno(OK);
    return(ret);
}




/*---------------------------------------------*/
/* Extract branch from pathname (/branch/leaf) */
/*---------------------------------------------*/

_PUBLIC _BOOLEAN strbranch(const char *pathname, char *branch)

{   size_t   i;
    _BOOLEAN ret = FALSE;

    if(pathname == (char *)NULL || branch == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    for(i=strlen(pathname); i> 0; --i)
    {  if(pathname[i] == '/')
       {  ret = TRUE;
          break;
       }
    }


    /*--------*/
    /* Branch */
    /*--------*/

    if(i > 0)
    {  (void)strlcpy(branch,pathname,SSIZE);
       branch[i] = '\0';
    }


    /*-----------*/
    /* No branch */
    /*-----------*/

    else
       (void)strlcpy(branch,pathname,SSIZE);

    pups_set_errno(OK);
    return(ret);
}





/*-----------------------------------------------------------*/
/* Strip first digit in string (and all characters after it) */
/*-----------------------------------------------------------*/

_PUBLIC _BOOLEAN strdigit(char *s)

{   size_t i,
           size;

    if(s == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);


    /*--------------------------------*/
    /* Don't process zero size string */
    /*--------------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       for(i=0; i<size; ++i)
       {  if(isdigit(s[i]))
          {  s[i] = '\0';
            return(TRUE);
          }
       }

       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*---------------------------------------------------------------------*/
/* Strip leading characters from string (before demarcation character) */ 
/*---------------------------------------------------------------------*/

_PUBLIC char *strlead(char *s, const char c)

{   size_t size,
           i = 0;

    if(s == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return((char *)NULL);
    }

    size = strlen(s);

    /*--------------------------------*/
    /* Don't process zero size string */
    /*--------------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       while(s[i] == c)
       {     if(i == size)
                return((char *)NULL);
             else
                ++i;
       }

       return((char *)&s[i]);
    }

    pups_set_errno(ERANGE);
    return((char *)NULL);
}
 




/*---------------------------------------------------*/
/* Count number of occurences of character in string */
/*---------------------------------------------------*/

_PUBLIC ssize_t strchcnt(const char c, const char *s)

{   size_t i,
           size,
           cnt = 0;

    if(s == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);


    /*--------------------------------*/
    /* Don't process zero size string */
    /*--------------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       for(i=0; i<strlen(s); ++i)
       {  if(s[i] == c)
             ++cnt;
       }

       return(cnt);
    }

    pups_set_errno(ERANGE);
    return(-1);
}





/*-----------------------------------------------------------------------*/
/* Strip numeric characters from string (including '#' and '^' which are */
/* used to demarcate checksums in file names)                            */
/*-----------------------------------------------------------------------*/

_PUBLIC _BOOLEAN strpdigit(const char *s, char *stripped)

{   size_t i,
           size,
           cnt  = (-1);

    if(s == (const char *)NULL || stripped == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);


    /*----------------------------------*/
    /* Don't process zero length string */
    /*----------------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       for(i=0; i<size; ++i)
       {  if(isdigit(s[i]))
          {  if(stripped[cnt] == '-')
                stripped[cnt]   = '\0';
             else
                stripped[cnt+1] = '\0';
             return(TRUE);
          }
          else if(s[i] == '_' || s[i] == '-' ||  s[i] == '.' || s[i] == '^' || s[i] == '#')
          {  ++cnt;
             stripped[cnt] = '-';
          }
          else if(isalpha(s[i]))
          {  ++cnt;
             stripped[cnt] = s[i];
          }
       }

       stripped[cnt+1] = '\0';
       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}





/*------------------------------------------------*/
/* Strip character from string (starting at tail) */
/*------------------------------------------------*/

_PUBLIC char *strpch(const char token, char *s)

{   size_t start_str,
           size,
           i     = 0;
   
    _BOOLEAN c_ret = FALSE;
 
    if(s == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return((char *)NULL);
    }

    size = strlen(s);


    /*----------------------------------*/
    /* Don't process zero length string */
    /*----------------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);


       /*-----------------------*/
       /* Remove leading tokens */
       /*-----------------------*/

       while(s[i] == token)
       {     ++i;

            /*-----------------------------------*/
            /* String just contains token and is */
            /* effectively empty                 */
            /*-----------------------------------*/

            if(i == 0)
            {  pups_set_errno(ESRCH);
               return((char *)NULL);
            }
       }   

       start_str = i;


       /*------------------------*/
       /* Remove trailing tokens */
       /*------------------------*/

       i = size;
       if(s[i] == '\n')
       {  c_ret = TRUE;
          --i;
       }

       while(s[i] == token)
            --i;
       ++i;

       if(c_ret == TRUE)
          s[i] = '\n';
       else
          s[i] = '\0';

       return((char *)&s[start_str]);
    }

    pups_set_errno(ERANGE);
    return((char *)NULL);
}




/*-----------------------------------------------------*/
/* Replace multiple chracters in string with character */
/*-----------------------------------------------------*/

_PUBLIC int32_t mchrep(const char rep_ch, const char *ch_to_rep, char *s1)

{   size_t   i,
             j,
             size,
             rep_size;

    uint32_t cnt  = 0;

    if(s1 == (char *)NULL || ch_to_rep == (const char *)NULL )
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    
    size     = strlen(s1);
    rep_size = strlen(ch_to_rep);


    /*--------------------------------*/
    /* Don't process zero size string */
    /*--------------------------------*/

    if(size > 0 && rep_size > 0)
    {  pups_set_errno(OK);

       for(i=0; i<size; ++i)
       {  for(j=0; j<rep_size; ++j)
          {  if(s1[i] == ch_to_rep[j] || (ch_to_rep[j] == 'D' && isdigit(s1[i])))
             {  s1[i] = rep_ch;
                ++cnt;
             }
          }
       }

       return(cnt);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*----------------------------------------------------------*/
/* Copy string checking for NULL or invalid argument string */
/*----------------------------------------------------------*/

_PUBLIC char *strccpy(char *s1, const char *s2)

{   size_t size_2,
           i  = 0;

    if(s2 == (char *)INVALID_ARG)
       return((char *)INVALID_ARG);
    else if(s1 == (char *)NULL || s2 == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return((char *)NULL);
    }



    /*-----------------------------------*/
    /* Don't process zero length strings */
    /*-----------------------------------*/

    size_2 = strlen(s2);
    if(size_2 > 0)
    {  while(s2[i] != '\0')
       {  s1[i] = s2[i];
          ++i;
       }
       s1[i] = '\0';

       pups_set_errno(OK);
       return(s1);
    }

    pups_set_errno(ERANGE);
    return((char *)NULL);
}




/*-----------------------------------------*/
/* Return the position of nth occurence of */
/* character in string (relative to start) */
/*-----------------------------------------*/

_PUBLIC ssize_t ch_index(const char *s, const char ch)

{   size_t i,
           size;

    if(s == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);


    /*-----------------------------------*/
    /* Don't process zero length string */
    /*-----------------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       for(i=0; i<size; ++i)
       {  if(s[i] == ch)
             return((int32_t)i);
       }

       return(-1);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*-----------------------------------------*/
/* Return the position of nth occurence of */
/* character in string (relative to end) */
/*-----------------------------------------*/

_PUBLIC ssize_t rch_index(const char *s, const char ch)

{   size_t i,
           size;

    if(s == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);


    /*----------------------------------*/
    /* Don't process zero length string */
    /*----------------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       for(i=size; i>= 0; --i)
       {  if(s[i] == ch)
             return((int32_t)i);
       }

       return(-1);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*-----------------------------------*/
/* Is character first one in string? */
/*-----------------------------------*/

_PUBLIC _BOOLEAN ch_is_first(const char *s, const char ch)

{   size_t size;

    if(s == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);


    /*----------------------------------*/
    /* Don't process zero length string */
    /*----------------------------------*/

    if(size > 0)
    {  pups_set_errno(OK);

       if(s[0] == ch)
          return(TRUE);
 
       return(FALSE);
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*----------------------------------*/
/* Is character last one in string? */
/*----------------------------------*/

_PUBLIC _BOOLEAN ch_is_last(const char *s, const char ch)

{   size_t size;

    if(s == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);


    /*----------------------------------*/
    /* Don't process zero length string */
    /*----------------------------------*/

    if(size > 0)
    {  if(s[size] == ch)
          return(TRUE);

       return(FALSE); 
    }

    pups_set_errno(ERANGE);
    return(-1);
}




/*-------------------------------------*/
/* Locate argument in the command tail */
/*-------------------------------------*/

_PUBLIC int32_t pups_locate(_BOOLEAN *init, const char *swtch, uint32_t *cnt, char *args[], uint32_t nchars)

{     uint32_t j;
      char     ctail_alias_file_name[SSIZE] = "";

      if(init  == (_BOOLEAN *)      NULL  ||
         swtch == (const char *)    NULL  ||
         cnt   == (int32_t *)       NULL  ||
         args  == (const char **)   NULL   )
      {  pups_set_errno(EINVAL);
         return(-1);
      }
      else
         pups_set_errno(OK);
  

      /*-----------------------------------------------*/
      /* If we have a command tail alias argument file */
      /* see if this switch is aliased                 */
      /*-----------------------------------------------*/

      (void)snprintf(ctail_alias_file_name,SSIZE,"%s/.%s.ctailrc",appl_home,appl_name);

      if(access(ctail_alias_file_name,F_OK) == 0)
      {  FILE *ctail_alias_stream = (FILE *)NULL;

          int32_t eof = 0;

         char     next_alias[SSIZE]  = "",
                  alias[SSIZE]       = "",
                  substitute[SSIZE]  = "";

         if((ctail_alias_stream = fopen(ctail_alias_file_name,"r")) == (FILE *)NULL)
            pups_error("[pups_locate] cannot open alias file");

         do {   (void)fgets(next_alias,SSIZE,ctail_alias_stream);


                /*---------------------------------------------------*/
                /* feof() is an alias -- and fail to expand properly */
                /* if tested in a multi-term if statement.           */
                /*---------------------------------------------------*/

                eof = feof(ctail_alias_stream);
                if(eof == 0 && next_alias[0] != '\n' && next_alias[0] != '#')
                {  if(sscanf(next_alias,"%s%s",alias,substitute) != 2)
                      pups_error("[pups_locate] corrupt alias file");

                   if(nchars > 0)
                   {  if(strncmp(swtch,alias,nchars) == 0)
                      {  (void)strlcpy(swtch,substitute,SSIZE);
                         (void)fclose(ctail_alias_stream);
                         goto aliased;
                      }
                   }
                   else
                   {  if(strcmp(swtch,alias) == 0)
                      {  (void)strlcpy(swtch,substitute,SSIZE);
                         (void)fclose(ctail_alias_stream);
                         goto aliased;
                      }
                   }
                }
            } while(feof(ctail_alias_stream) == 0);
      }

aliased:

      if(*cnt == 0)
         return((int32_t)NOT_FOUND);

      for(j=0; j<=t_args; ++j)
      {     if(args[j]    != (char *)NULL      &&
               args[j][0] == '-'               &&
               strcmp(swtch,&args[j][1]) == 0   )
              {  (*cnt)--;
                 argd[j] = TRUE;
                 return((int32_t)j);
              }
      }

      pups_set_errno(ERANGE);
      return((int32_t)NOT_FOUND);
}




/*----------------------------------------*/
/* Get integer from command tail argument */
/*----------------------------------------*/

_PUBLIC int32_t pups_i_dec(int32_t *ptr, uint32_t *argc, char *argv[])

{    int32_t ret;

    if(ptr  == (int32_t  *)NULL  ||
       argc == (int32_t  *)NULL  ||
       argv == (char     **)NULL   ) 
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);

    if(*argc > t_args)
       return((int)INVALID_ARG);

    ++(*ptr);
    if(argv[(*ptr)] == NULL ||
       (argv[(*ptr)][0] == '-' && !isdigit(argv[(*ptr)][1])))
       return((int)INVALID_ARG);
    else
    {  (*argc)--;
       argd[*ptr] = TRUE;
       if(sscanf(argv[(*ptr)],"%d",&ret) != 1)
          return((int)INVALID_ARG); 
       else
          return(ret);   
    }
}




/*---------------------------------------------*/
/* Get float/double from command tail argument */
/*---------------------------------------------*/

_PUBLIC FTYPE pups_fp_dec(int32_t *ptr, uint32_t *argc, char *argv[])

{   FTYPE ret;

    if(ptr  == (int32_t   *)NULL  ||
       argc == (int32_t   *)NULL  ||
       argv == (char     **)NULL   ) 
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);

    if(*argc > t_args)
       return((FTYPE)INVALID_ARG);

    ++(*ptr);
    if(argv[(*ptr)] == (char *)NULL ||
       (argv[(*ptr)][0] == '-' && !isdigit(argv[(*ptr)][1])))
       return((FTYPE)INVALID_ARG);
    else
    {  (*argc)--;
       argd[*ptr] = TRUE;
       if(sscanf(argv[*ptr],"%F",&ret) != 1)
          return((FTYPE)INVALID_ARG);
       else
          return(ret);
    }
}




/*-------------------------------------------*/
/* Get string from the command tail argument */
/*-------------------------------------------*/

_PUBLIC char *pups_str_dec(int32_t *ptr, uint32_t *argc, char *argv[])

{   _IMMORTAL char aggregate_str[SSIZE] = ""; 

    if(ptr  == (int32_t  *)NULL  ||
       argc == (int32_t  *)NULL  ||
       argv == (char    **)NULL   ) 
    {  pups_set_errno(EINVAL);
       return((char *)INVALID_ARG);
    }
    else
       pups_set_errno(OK);

    if(*argc > t_args)
       return((char *)INVALID_ARG);

    ++(*ptr);
    if(argv[(*ptr)] == (char *)NULL || argv[(*ptr)][0] == '-' || strlen(argv[(*ptr)]) == 0)
       return((char *)((char *)INVALID_ARG));
    else
    {  if(argv[(*ptr)][0] != '"') 
       {  (*argc)--;
          argd[*ptr] = TRUE;
          return((char *)argv[(*ptr)]);
       }
       else
       {  (void)strlcat(aggregate_str,"",SSIZE);

          while(argv[(*ptr)][strlen(argv[(*ptr)])-1] != '"' && (*argc) > 0)
          {  (void)strlcat(aggregate_str,argv[(*ptr)],SSIZE);
             (void)strlcat(aggregate_str," ",SSIZE);

             (*argc)--;
             ++(*ptr);
          }

          (void)strlcat(aggregate_str,argv[(*ptr)],SSIZE);
          aggregate_str[strlen(aggregate_str)-1] = '\0';

          return(&aggregate_str[1]);
       }
    }
}




/*---------------------------------*/
/* Get character from command tail */
/*---------------------------------*/

_PUBLIC char pups_ch_dec(int32_t *ptr, uint32_t *argc, char *argv[])

{   if(ptr  == (int32_t  *)NULL  ||
       argc == (int32_t  *)NULL  ||
       argv == (char    **)NULL   ) 
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);

    if(*argc > t_args)
       return((char)INVALID_ARG);

    ++(*ptr);
    if(argv[(*ptr)] == (char *)NULL || argv[(*ptr)][0] == '-')
       return((char)INVALID_ARG);
    else
    {  (*argc)--;
       argd[*ptr] = TRUE;
       return((char)argv[(*ptr)][0]);
    }
}




/*--------------------------------*/
/* Load argument vector from file */
/*--------------------------------*/

_PUBLIC void pups_argfile(int32_t ptr, uint32_t *argc, char *argv[], _BOOLEAN argd[])

{    int32_t i,
             n_args;

    size_t c_index;

    _BOOLEAN looper          = TRUE,
             quoted          = FALSE,
             default_argfile = FALSE;

    char tmp_f_name[SSIZE]   = "",
         next_token[SSIZE]   = "";

    FILE *arg_file = (FILE *)NULL,
         *tmp_file = (FILE *)NULL;

    if(ptr  == (int32_t  *)NULL  ||
       argc == (int32_t  *)NULL  ||
       argv == (char    **)NULL  ||
       argd == (char    **)NULL   ) 
    {  pups_set_errno(EINVAL);
       return;
    }


    if(ptr >= (*argc) || strccpy(arg_f_name,pups_str_dec(&ptr,argc,argv)) == (char *)NULL)
    {  (void)snprintf(arg_f_name,SSIZE,"%s.agf",appl_name);
       if(access(arg_f_name,F_OK | R_OK) == (-1))
          return;

       default_argfile = TRUE;
       --ptr;
    }
    else
       ++ptr;


     #ifdef UTILIB_DEBUG
     (void)strdate(date);
     if(default_argfile == TRUE)
        (void)fprintf(stderr,"%s %s (%d@%s:%s): argfile: reading command tail from default argfile %s\n",
                                                 date,appl_name,appl_pid,appl_host,appl_owner,arg_f_name);
     else
        (void)fprintf(stderr,"%s %s (%d@%s:%s): argfile: reading command tail from argfile %s\n",
                                         date,appl_name,appl_pid,appl_host,appl_owner,arg_f_name);
     (void)fflush(stderr);
     #endif /* UTILIB_DEBUG */


    /*--------------------------------------------------------------------*/
    /* Shift all existing arguments left and then add arguments read from */
    /* the argument file to the end of the secondary argument vectors     */ 
    /*--------------------------------------------------------------------*/

    if(default_argfile == FALSE)
    {  ptr -= 2;
       for(i=ptr; i<t_args-1; ++i)
           argv[i] = argv[i+1];
    }

    if((arg_file = (FILE *)fopen(arg_f_name,"r")) == (FILE *)NULL)
    {  (void)snprintf(errstr,SSIZE,"[pups_argfile] cannot find argument file (%s)\n",arg_f_name);
       pups_error(errstr);
    }


    /*------------------------------------------------------------------------*/
    /* Strip all comments from the argument file. Arguments are prefixed by a */
    /* '#' character                                                          */
    /*------------------------------------------------------------------------*/

    (void)snprintf(tmp_f_name,SSIZE,"/tmp/stripc.dat.%s.%d",appl_host,appl_pid);
    tmp_file = (FILE *)pups_strp_commnts('#',arg_file,tmp_f_name);


    /*---------------------------------------------------------------*/
    /* Allocate sufficient space for the replacement argument vector */
    /*---------------------------------------------------------------*/

    t_args = ptr;
    do {  n_args = fscanf(tmp_file,"%s",next_token);
 
          if(n_args != 1)
             goto exit;
          else
          {  if(quoted == FALSE)
             {  argv[t_args] = (char *)pups_malloc(SSIZE);
                argd[t_args] = FALSE;
             }

             /*--------------------------------------------------------------------*/
             /* Test to see if string is '"' or contains '"' - if it does catenate */
             /*  all items read until anoth '"' is found                           */
             /*--------------------------------------------------------------------*/

             if((c_index = ch_index(next_token,'"')) != -1)
             {  if(quoted == TRUE)
                {  next_token[c_index] = '\0';
                   (void)strlcat(args[t_args],next_token,SSIZE);
                   quoted = FALSE;
                 }
                 else
                 {  quoted = TRUE;
                    if(strlen(next_token) > 1)
                    {  (void)strlcpy(args[t_args],&next_token[1],SSIZE);
                       (void)strlcat(args[t_args]," ",SSIZE);
                    }
                    else
                       (void)strlcpy(args[t_args],"",SSIZE);
                 }
              }
              else
              {  if(quoted == TRUE)
                 {  (void)strlcat(args[t_args],next_token,SSIZE);
                    (void)strlcat(args[t_args]," ",SSIZE);
                 }
                 else
                    (void)strlcpy(args[t_args],next_token,SSIZE);
              }

              ++t_args;
           }
        } while(looper == TRUE);


    /*---------------------------*/
    /* Adjust the argument count */
    /*---------------------------*/

exit:

    init = TRUE;
    *argc = appl_t_args = t_args;


    #ifdef UTILIB_DEBUG
    for(i=0; i<*argc; ++i)
    {  (void)fprintf(stderr,"UTILIB COPY[%d]: %s\n",i,args[i]);
       (void)fflush(stderr);
    }

    (void)strdate(date);
    (void)fprintf(stderr,"%s %s (%d@%s:%s): %d additional arguments read\n",
                  date,appl_name,appl_pid,appl_host,appl_owner,t_args - ptr);
    (void)fflush(stderr);
    #endif /* UTILIB_DEBUG */


    (void)fclose(tmp_file);
    (void)unlink(tmp_f_name);

    pups_set_errno(OK);
}



/*----------------------------------------------------------------*/
/* Generate effective command line from secondary argument vector */
/*----------------------------------------------------------------*/

_PUBLIC void pups_argtline(char *cmd_tail)

{   uint32_t i;

    if(cmd_tail == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }
    else
       pups_set_errno(OK);

    #ifdef UTILIB_DEBUG
    for(i=0; i<appl_t_args; ++i)
    {   (void)fprintf(stderr,"UTILIB ARGS[%d]: %s\n",i,args[i]);
        (void)fflush(stderr);
    }
    (void)fprintf(stderr,"\n\n");
    (void)fflush(stderr);
    #endif /* UTILIB_DEBUG */

    (void)strlcpy(cmd_tail,"",SSIZE);
    for(i=0; i<appl_t_args; ++i)
    { 

       #ifdef UTILIB_DEBUG
       (void)fprintf(stderr,"UTILIB IN ARG[%d]: %s\n",i,args[i]);
       (void)fflush(stderr);
       #endif /* UTILIB_DEBUG */


       /*------------------------------------------------------------*/
       /* Note that -psrp_reactivate_client, -psrp_segment, -pen and */
       /* -bin are generated automatically by psrp_new_segment       */
       /*------------------------------------------------------------*/

       if(in_psrp_new_segment == TRUE) 
       {  if(strcmp(args[i],"-psrp_reactivate_client") == 0)
             i += 1;
          else if(strcmp(args[i],"-psrp_segment") == 0)
             i += 1;
          else if(strcmp(args[i],"-pen") == 0)
             i += 1;
          else if(strcmp(args[i],"-bin") == 0)
             i += 1;
          else
          {

             #ifdef UTILIB_DEBUG
             (void)fprintf(stderr,"UTILIB NEW SEGMENT OUT ARG[%d]: %s\n",i,args[i]);
             (void)fflush(stderr);
             #endif /* UTILIB_DEBUG */

             (void)strlcat(cmd_tail,args[i],SSIZE);
          }
       }
       else
       {

          #ifdef UTILIB_DEBUG
          (void)fprintf(stderr,"UTILIB OUT ARG[%d]: %s\n",i,args[i]);
          (void)fflush(stderr);
          #endif /* UTILIB_DEBUG */

          (void)strlcat(cmd_tail,args[i],SSIZE);
       }

       if(i < appl_t_args - 1)
          (void)strlcat(cmd_tail," ",SSIZE);
    }

    #ifdef UTILIB_DEBUG
    (void)fprintf(stderr,"UTILIB OUTPUT CMD TAIL: %s\n",cmd_tail);
    (void)fflush(stderr);
    #endif /* UTILIB_DEBUG */

}




/*-----------------------------------*/
/* Signal to catch really bad errors */
/*-----------------------------------*/

_PRIVATE void catch_sigsegv(void)

{   pups_error("[catch_segv] cannot recover from error");
}





/*--------------------------------*/
/* Do we have resources required? */
/*--------------------------------*/

_PUBLIC _BOOLEAN pups_have_resources(const FTYPE proc_loading, const int32_t mem_required)

{   FTYPE    current_proc_loading;
     int32_t mem_free;

    if(pups_get_resource_loading(&current_proc_loading,&mem_free) == (-1))
       return(-1);

    if(current_proc_loading > proc_loading || mem_free < mem_required)
    {  pups_set_errno(EAGAIN);
       return(FALSE);
    }

    (void)pups_set_errno(OK);
    return(TRUE);
}




/*-------------------------------------*/
/* Make copy of command line arguments */
/*-------------------------------------*/

_PUBLIC void pups_copytail(int *argc, char *args[], const char *argv[])

{   uint32_t i,
             start                  = 1,
             cnt                    = 0;

    char     ssh_remote_port[SSIZE] = "",
             ssh_remote_host[SSIZE] = "",
             ssh_remote_user[SSIZE] = "",
             ssh_remote_cmd[SSIZE]  = "";

    _BOOLEAN do_bg                  = FALSE,
             do_remote_exec         = FALSE;

    if(argc == (int *)  NULL  ||
       args == (char **)NULL  ||
       argv == (char **)NULL   ) 
    {  pups_set_errno(EINVAL);
       return;
    }
    else
       pups_set_errno(OK);


    /*----------------------------------------*/
    /* Is this process going to run remotely? */
    /*----------------------------------------*/

    for(i=0; i<(*argc); ++i)
    {  if(strcmp(argv[i],"-on") == 0)
       {  do_remote_exec = TRUE;
          break;
       }
    }

    if(do_remote_exec == TRUE)
    {  if(strccpy(ssh_remote_cmd,pups_search_path("PATH",argv[0])) == (char *)NULL)
          pups_error("[set_pen] failed to resolve ben path");


       /*---------------------------------------*/
       /* Parse command tail for remote command */
       /*---------------------------------------*/

       if((*argc) > 1)
       {  (void)strlcat(ssh_remote_cmd," ",SSIZE);

          for(i=1; i<(*argc); ++i)
          { 

             /*-------------------------*/
             /* Get name of remote host */
             /*------------------------*/

             if(strcmp(argv[i],"-on") == 0)
             {  (void)strlcpy(ssh_remote_host,argv[i+1],SSIZE);
                ++i;
             }


             /*---------------------------------*/
             /* Get the username on remote host */
             /*---------------------------------*/

             else if(strcmp(argv[i],"-user") == 0)
             {  (void)snprintf(ssh_remote_user,SSIZE,"-l %s",argv[i+1]);
                ++i;
             }


             /*--------------------------------------------------*/
             /* Get the port number to connect to on remote host */
             /*--------------------------------------------------*/

             else if(strcmp(argv[i],"-ssh_port") == 0)
             {  (void)snprintf(ssh_remote_port,SSIZE,"-p %s",argv[i+1]);

                (void)strlcat(ssh_remote_cmd,argv[i],SSIZE);
                if(i < (*argc) - 1)
                   (void)strlcat(ssh_remote_cmd," ",SSIZE);
                ++i;

                if(i == (*argc))
                   pups_error("expecting remote port specification");

                (void)strlcat(ssh_remote_cmd,argv[i],SSIZE);
                if(i < (*argc) - 1)
                   (void)strlcat(ssh_remote_cmd," ",SSIZE);

                #ifdef UTILIB_DEBUG
                (void)fprintf(stderr,"UTILIB COPYTAIL SSH_PORT: %s\n",ssh_remote_cmd);
                (void)fflush(stderr);
                #endif /* UTILIB_DEBUG */
             }


             /*--------------------------------------------------*/
             /* are we going to compress data in the ssh tunnel? */
             /*--------------------------------------------------*/

             else if(strcmp(argv[i],"-ssh_compress") == 0)
             {  ssh_compression = TRUE;

                (void)strlcat(ssh_remote_cmd,argv[i],SSIZE);
                if(i < (*argc) -1)
                   (void)strlcat(ssh_remote_cmd," ",SSIZE);

                #ifdef UTILIB_DEBUG
                (void)fprintf(stderr,"UTILIB COPYTAIL SSH_COMPRESS: %s\n",ssh_remote_cmd);
                (void)fflush(stderr);
                #endif /* UTILIB_DEBUG */
             }


             /*----------------------------------*/
             /* Are we detaching remote command? */
             /*----------------------------------*/

             else if(strcmp(argv[i],"-bg") == 0)
                   do_bg = TRUE; 
             else
             {  

                /*-------------------------------------------*/
                /* Next component of argument vector globbed */
                /*-------------------------------------------*/

                if(strin(argv[i]," ") == TRUE)
                {  char tmpstr[SSIZE] = "";

                   (void)snprintf(tmpstr,SSIZE,"\"%s\"",argv[i]);
                   (void)strlcat(ssh_remote_cmd,  tmpstr,SSIZE);
                }


                /*----------------------------------------------*/
                /* Next component of argment vector not globbed */
                /*----------------------------------------------*/

                else
                   (void)strlcat(ssh_remote_cmd,argv[i],SSIZE);

                if(i < (*argc) -1)
                   (void)strlcat(ssh_remote_cmd," ",SSIZE);
             }
          }
       }


       /*----------------------------------------*/
       /* is this command running in background? */
       /*----------------------------------------*/

       if(do_bg == TRUE)
          (void)strlcat(ssh_remote_cmd,">& /dev/null &",SSIZE);


       #ifdef UTILIB_DEBUG
       (void)fprintf(stderr,"UTILIB UTILS REMOTE CMD [%d]: ssh %s %s %s %s\n",appl_pid,ssh_remote_port,ssh_remote_user,ssh_remote_host,ssh_remote_cmd);
       (void)fflush(stderr);
       #endif /* UTILIB_DEBUG */


       /*------------------------------------------*/
       /* Build argument vector for remote command */
       /*------------------------------------------*/

       args[0] = (char *)pups_malloc(256*sizeof(_BYTE));
       (void)strlcpy(args[cnt],"ssh",SSIZE);
       ++cnt;

       /*--------------------------------------------*/
       /* Preseve terminal connection if we have one */
       /*--------------------------------------------*/

       if(do_bg == FALSE && isatty(0) == 1)
       {   args[cnt] = (char *)pups_malloc(256*sizeof(_BYTE));
           (void)strlcpy(args[cnt],"-t",SSIZE);
           ++cnt;
       }


       /*-------------------------------*/
       /* Compress ssh data if required */
       /*-------------------------------*/

       if(ssh_compression == TRUE)
       {   args[cnt] = (char *)pups_malloc(256*sizeof(_BYTE));
           (void)strlcpy(args[cnt],"-C",SSIZE);
           ++cnt;
       }


       /*---------------------------------*/
       /* Remote port number if specified */
       /*---------------------------------*/

       if(strcmp(ssh_remote_port,"") != 0)
       {   args[cnt] = (char *)pups_malloc(256*sizeof(_BYTE));
           (void)strlcpy(args[cnt],ssh_remote_port,SSIZE);
           ++cnt;
       }


       /*-------------*/
       /* Remote user */
       /*-------------*/
       
       if(strcmp(ssh_remote_user,"") != 0)
       {   args[cnt] = (char *)pups_malloc(256*sizeof(_BYTE));
           (void)strlcpy(args[cnt],ssh_remote_user,SSIZE);
           ++cnt;
       }


       /*-------------*/
       /* Remote host */
       /*-------------*/

       args[cnt] = (char *)pups_malloc(256*sizeof(_BYTE));
       (void)strlcpy(args[cnt],ssh_remote_host,SSIZE);
       ++cnt;


       /*----------------*/
       /* Remote command */
       /*----------------*/

       args[cnt] = (char *)pups_malloc(256*sizeof(_BYTE));
       (void)strlcpy(args[cnt],ssh_remote_cmd,SSIZE);


       #ifdef UTILIB_DEBUG
       for(i=0; i<cnt; ++i)
       {  (void)fprintf(stderr,"UTILIB ARG[%d]: %s\n",i,args[i]);
          (void)fflush(stderr);
       }
       #endif /* UTILIB_DEBUG */


       /*-----------------------*/
       /* Launch remote command */
       /*-----------------------*/

       if(pups_fork(FALSE,FALSE) != 0)
          (void)execvp("ssh",args);
       else
          pups_exit(0);
    }


    /*-------------------------------------------------------------------*/
    /* Is this process going to run under an assumed name - if it is the */
    /* secondary argument vector must include its assumed name           */
    /*-------------------------------------------------------------------*/

    for(i=0; i<(*argc); ++i)
    {  if(strcmp(argv[i],"-pen") == 0)
       {  start = 0;
          break;
       }
    }

    for(i=start; i<(*argc); ++i)
    {  args[cnt] = (char *)pups_malloc(255);
       if(strcmp(argv[i],"-pen") == 0)
       {  (void)strlcpy(args[0],  argv[i+1],SSIZE);
          appl_have_pen = TRUE;


          /*------------------------------------------------------*/
          /* Tell the child process it is a pen rather than a ben */
          /*------------------------------------------------------*/

          (void)strlcpy(args[cnt],"-bin",SSIZE);
          argd[cnt] = FALSE;
          ++cnt;

          args[cnt] = (char *)pups_malloc(255);
          (void)strlcpy(args[cnt],argv[0],SSIZE);
          argd[cnt] = FALSE;
          ++cnt;

          ++i;
       }
       else
       {  (void)strlcpy(args[cnt],argv[i],SSIZE);
          argd[cnt] = FALSE;
          ++cnt;
       }
    }

    if(appl_have_pen == TRUE)
    {

       /*----------------------------------------------------*/
       /* Remove the pen flag and pen name from command tail */
       /*----------------------------------------------------*/

       pups_set_pen(args,argv[0],args[0]);
    }

    appl_t_args = cnt; 
    t_args      = (*argc) - 1;
}




/*--------------------------------*/
/* Initialise PUPS/P3 environment */
/*--------------------------------*/
                                                  /*--------------------------------------*/
_PUBLIC void pups_std_init(_BOOLEAN open_source,  /* TRUE if GPL copyrighted open source  */
                           int32_t        *argc,  /* Argument count                       */
                           char         *v_name,  /* Version of filter                    */
                           char        *au_name,  /* Author name                          */
                           char       *app_name,  /* Application name                     */
                           char         *d_made,  /* Revision date                        */
                           const char   *argv[])  /* Secondary argument vector            */
                                                  /*--------------------------------------*/
           
{    int32_t i;

    sigset_t blocked_set;


                                  /*-----------------------------*/
    _BOOLEAN stdin_bg    = FALSE, /* TRUE if stdin backgrounded  */
             stdout_bg   = FALSE, /* TRUE if stdout backgrounded */
             stderr_bg   = FALSE; /* TRUE if stderr bacgrounded  */
                                  /*-----------------------------*/

    struct stat   buf;                             /*-----------------------*/
    struct passwd *pwent = (struct passwd *)NULL;  /* Passwd entry of owner */
                                                   /*-----------------------*/


    #ifdef CRIU_SUPPORT
    /*------*/
    /* Criu */
    /*------*/

    appl_criu_ssaves     = 0;
    appl_criu_poll_time  = DEFAULT_CRIU_POLL_TIME;

    #endif /* Criu support */


    /*----------------------------*/
    /* Get PID and  process group */
    /*----------------------------*/

    appl_pid      = getpid();
    appl_sid      = getpgrp();


    /*----------------------------------*/
    /* Standard error stream is stderr. */
    /*----------------------------------*/

    err_stream = stderr;


    /*-------------------------------------------------*/
    /* Get owner of this application (and its parent). */
    /*-------------------------------------------------*/

    appl_uid      = getuid();
    appl_gid      = getgid();
    appl_ppid     = getppid();


    /*----------------------------------------------------*/
    /* Get the name of the host running this application. */
    /*----------------------------------------------------*/
 
    (void)gethostname(appl_host,SSIZE);


    /*---------*/
    /* Softdog */
    /*---------*/

    appl_softdog_timeout = DEFAULT_SOFTDOG_TIMEOUT;
    appl_softdog_pid     = (-1);
    appl_softdog_enabled = FALSE;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_std_init] attempt by non root thread to perform PUPS/P3 global utility operation");

    if(argc     == (int32_t *)NULL  ||
       v_name   == (char    *)NULL  ||
       au_name  == (char    *)NULL  ||
       app_name == (char    *)NULL  ||
       d_made   == (char    *)NULL  ||
       argv     == (char   **)NULL   ) 
    {  pups_set_errno(EINVAL);
       return;
    }


    /*----------------------------------------------------------------------*/
    /* Mark application as PSRP enabled but ignoring PSRP protocol signals. */
    /*----------------------------------------------------------------------*/

    appl_psrp = TRUE;
    for(i=1; i<MAX_SIGS; ++i)
       pupsighold_cnt[i] = 0;
    (void)pupshold(PSRP_SIGS);


    /*----------------------------------------------------------*/
    /* If we are accessing files via an MFS partition appl_home */
    /* will be none-NULL                                        */
    /*----------------------------------------------------------*/

    if(getenv("HOME") == (char *)NULL)
       (void)strlcpy(appl_home,"homeless",SSIZE);
    else
       (void)strlcpy(appl_home,getenv("HOME"),SSIZE);


    /*---------------------------*/
    /* Initialise signal status. */
    /*---------------------------*/

    initsigstatus();


    /*----------------------------------------------------------------------*/
    /* Delete shadow files if we have a segmentation, bus or floating point */
    /* error.                                                               */
    /*----------------------------------------------------------------------*/

    (void)pups_sighandle(SIGSEGV,"segbusfpe_handler",(void *)&segbusfpe_handler,(sigset_t *)NULL);
    (void)pups_sighandle(SIGBUS ,"segbusfpe_handler",(void *)&segbusfpe_handler,(sigset_t *)NULL);
    (void)pups_sighandle(SIGFPE ,"segbusfpe_handler",(void *)&segbusfpe_handler,(sigset_t *)NULL);

    vitimer_quantum = VITIMER_QUANTUM;


    /*--------------------------------------------------------------------------*/
    /* Are we a foreground process - if not arrange for all TERMINAL O/P to be  */
    /* sent to the data sink (/dev/null)                                        */
    /*--------------------------------------------------------------------------*/

    (void)pups_sighandle(SIGTTIN,"fgio_handler",(void *)&fgio_handler,(sigset_t *)NULL);
    (void)pups_sighandle(SIGTTOU,"fgio_handler",(void *)&fgio_handler,(sigset_t *)NULL);


    /*--------------------------------------------------*/
    /* Get name of controlling tty for this application */
    /*--------------------------------------------------*/

    appl_tty = open("/dev/tty",2);

    if(appl_tty != (-1))
    {  

       /*--------------------------------------------*/
       /* Try to get precise name of controlling tty */
       /*--------------------------------------------*/

       if(isatty(0) == 1)
          (void)strlcpy(appl_ttyname,ttyname(0),SSIZE);
       else if(isatty(1) == 1)
          (void)strlcpy(appl_ttyname,ttyname(1),SSIZE);
       else if(isatty(2) == 1)
          (void)strlcpy(appl_ttyname,ttyname(2),SSIZE);
       else
          (void)strlcpy(appl_ttyname,"/dev/tty",SSIZE);


       /*-------------------------------*/
       /* Are we a foreground process ? */
       /*-------------------------------*/

       if(tcgetpgrp(appl_tty) == getpgrp())
          appl_fgnd = TRUE;
       else
          appl_fgnd = FALSE;
    }
    else
       appl_fgnd = FALSE;


    /*------------------------------------------------------------*/
    /* Get unique i.d. for host machine running owner application */
    /*------------------------------------------------------------*/

    if(access("/etc/machine-id",F_OK | R_OK) == (-1))
       pups_error("[pups_std_init] cannot get (unique) machine i.d.");
    else
    {  des_t fdes = (-1);

       fdes = open("/etc/machine-id",O_RDONLY);
       (void)read(fdes,appl_machid,32);
       (void)close(fdes);
    }

    #ifndef SINGLE_THREAD
    #ifdef _OPENMP
    appl_omp_threads = omp_get_max_threads();
    #endif /* _OPENMP       */
    #endif /* SINGLE_THREAD */

    #ifdef PTHREAD_SUPPORT
    appl_root_tid = pthread_self();
    #endif /* PTHREAD_SUPPORT */

    pwent = getpwuid(appl_uid);
    if(pwent != (struct passwd *)NULL)
       (void)strlcpy(appl_owner,pwent->pw_name,SSIZE);
    else
       (void)snprintf(appl_owner,SSIZE,"uid%d",appl_uid);

    (void)strlcpy(appl_password,"notset",SSIZE);


    /*----------------------------------*/
    /* Get user shell from environment. */
    /*----------------------------------*/
 
    if(strccpy(shell,(char *)getenv("EXEC_SHELL")) == (char *)NULL)
       (void)strlcpy(shell,"/bin/sh",SSIZE);


    if(strccpy(appl_fifo_dir,(char *)getenv("FIFO_PATCHBOARD")) == (char *)NULL)
    {  char tmp_fifos[SSIZE] = "";


       /*----------------------------------------------------*/
       /* If we have a /fifos directory for this host use it */
       /*----------------------------------------------------*/

       (void)snprintf(tmp_fifos,SSIZE,"/fifos.%s",appl_host);
       if(access(tmp_fifos,F_OK | R_OK | W_OK | X_OK) == 0)
          (void)strlcpy(appl_fifo_dir,"/fifos",SSIZE);
       else
          (void)snprintf(appl_fifo_dir,SSIZE,"/tmp");
    }


    /*-------------------------------------------------------------*/
    /* Iniitialise the slot manager and register all active slots. */
    /*-------------------------------------------------------------*/

    slot_manager_init();


    /*----------------------------------*/
    /* Make a copy of the command tail. */
    /*----------------------------------*/

    pups_copytail(argc,args,argv);


    /*--------------------------------------------*/
    /* Copy application data to global variables. */
    /*--------------------------------------------*/

    (void)strlcpy(version,v_name, SSIZE);
    (void)strlcpy(author, au_name,SSIZE);

    if(strrextr(argv[0],appl_name,'/') == FALSE)
    {  (void)strlcpy(appl_name,    argv[0],SSIZE); 
       (void)strlcpy(appl_bin_name,argv[0],SSIZE);
    }
    else
       (void)strlcpy(appl_bin_name,appl_name,SSIZE);

    (void)strlcpy(revdate,d_made,SSIZE);


    /*----------------------------------*/
    /* Initialise heap object tracking. */
    /*----------------------------------*/

    tinit();


    /*-----------------------------------------*/
    /* Read in command line tail if requested. */
    /*-----------------------------------------*/

    if((ptr = pups_locate(&init,"argf",argc,args,0)) != NOT_FOUND)
       pups_argfile(ptr,argc,args,argd);
    else
    {

       /*-------------------------------------*/
       /* Read default argfile (if it exists) */
       /*-------------------------------------*/

       default_argfile = TRUE;
       pups_argfile((*argc),argc,args,argd);
    }


    /*------------------*/
    /* Resident process */
    /*------------------*/

    if(appl_enable_resident == TRUE && (ptr = pups_locate(&init,"resident",argc,args,0)) != NOT_FOUND)
    {  if(strccpy(appl_argfifo,pups_str_dec(&ptr,/*&*/argc,args)) == (char *)INVALID_ARG)
          pups_error("[resident process] expecting name of command parameter FIFO");


       /*-------------------------------*/
       /* Create command parameter FIFO */
       /*-------------------------------*/

       if(mkfifo(appl_argfifo,0600) == (-1))
          pups_error("[resident process] cannot create FIFO");

       appl_resident = TRUE;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s(%d@%s:%s): process is memory resident (argument FIFO \"%s\")\n",
                                            date,appl_name,appl_pid,appl_host,appl_owner,appl_argfifo);
          (void)fflush(stderr);
       }
    }


    /*------------------------------------------------------------*/
    /* If verbose switch is set, log status information to stderr */
    /*------------------------------------------------------------*/

    if(pups_locate(&init,"verbose",argc,args,0) != NOT_FOUND)
    {  appl_verbose = TRUE;
      (void)pups_get_fd_lock(2,GETLOCK);


      /*---------------------------------------------------------------------*/
      /* If test mode is set provide additional status information to stderr */
      /*---------------------------------------------------------------------*/

      if(pups_locate(&init,"test",argc,args,0) != NOT_FOUND)
         test_mode = TRUE;
    }


    /*------------------------------------*/
    /* Don't seed random number generator */
    /*------------------------------------*/

    if(pups_locate(&init,"noseed",argc,args,0) != NOT_FOUND)
    { if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): not seeding random number generator\n",
                                           date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }
    }

    
    /*------------------------------*/
    /* Seed random number generator */
    /*------------------------------*/

    else
    {  (void)srand48((int64_t)appl_pid);

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): seeding random number generator with pid [%d]\n",
                                             date,appl_name,appl_pid,appl_host,appl_owner,appl_pid);
          (void)fflush(stderr);
       }
    }


    /*-------------------------------------------------------*/
    /* Get (minimal) resource allocation to run application. */
    /*-------------------------------------------------------*/

    if((ptr = pups_locate(&init,"mra",argc,args,0)) != NOT_FOUND)
    {  FTYPE    mincpu;

        int32_t ret,
                minmem;

       time_t   tdum,
                timeout,
                start_time;

       if((ptr = pups_locate(&init,"cpu",argc,args,0)) != NOT_FOUND)
       {  if((mincpu = pups_fp_dec(&ptr,argc,args)) == (FTYPE)INVALID_ARG)
              mincpu = PUPS_DEFAULT_MIN_CPU;
       }
       else
          mincpu = PUPS_DEFAULT_MIN_CPU;

       if((ptr = pups_locate(&init,"mem",argc,args,0)) != NOT_FOUND)
       {  if((minmem = pups_i_dec(&ptr,argc,args)) == (int32_t)INVALID_ARG)
             minmem = PUPS_DEFAULT_MIN_MEM;
       }
       else
           minmem = PUPS_IGNORE_MIN_MEM;

       if((ptr = pups_locate(&init,"timeout",argc,args,0)) != NOT_FOUND)
       {  if((timeout = pups_i_dec(&ptr,argc,args)) == (int32_t)INVALID_ARG)
             timeout = PUPS_DEFAULT_RESOURCE_WAIT;
       }
       else
          timeout = PUPS_DEFAULT_RESOURCE_WAIT;


       /*------------------------------------------------------------------------*/
       /* Do we have resources to run application - if not hibernate until we do */
       /*------------------------------------------------------------------------*/

       if((ret = pups_have_resources((FTYPE)mincpu,minmem)) == FALSE)
       {  if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s(%d@%s:%s): waiting for minimum resources to run [CPU usage < %f, at least %d KB memory]\n",
                                                                         date,appl_name,appl_pid,appl_host,appl_owner,mincpu,minmem);
             (void)fflush(stderr);
          }

          start_time = time(&tdum);
          while(pups_have_resources((FTYPE)mincpu,minmem) == FALSE)
          {    if(time(&tdum) - start_time > timeout*3600)
               {  if(appl_verbose == TRUE)
                  {  (void)strdate(date);
                     (void)fprintf(stderr,"%s %s(%d@%s:%s): failed to get minimum resources to run (after %f hours) -- aborting\n",
                                                                        date,appl_name,appl_pid,appl_host,appl_owner,mincpu,minmem);
                     (void)fflush(stderr);
                  }

                  pups_exit(255);
               }
               (void)pups_usleep(10000);
          }

          if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s(%d@%s:%s): resources to run now available [CPU usage < %f, at least %d KB memory]\n",
                                                                   date,appl_name,appl_pid,appl_host,appl_owner,mincpu,minmem);
             (void)fflush(stderr);
          }
       }
       else
          pups_error("[pups_std_init] flag \"-mra\" is not supported");
    }


    /*----------------------------------------*/
    /* Don't detach stdio when in background. */
    /*----------------------------------------*/

    if((ptr = pups_locate(&init,"nodetach",argc,args,0)) != NOT_FOUND)
        appl_nodetach = TRUE;


    /*--------------------------------------------------------------------*/
    /* Display name and mode of controlling tty (if in appl_verbose mode) */
    /*--------------------------------------------------------------------*/

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       if(appl_ttyname == (char *)NULL)
          (void)fprintf(stderr,"%s %s (%d@%s:%s): application has no controlling tty\n",
                                           date,appl_name,appl_pid,appl_owner,appl_host);
       else
       {  if(appl_fgnd == TRUE)
             (void)fprintf(stderr,"%s %s (%d@%s:%s): application controlling tty is %s\n",
                                date,appl_name,appl_pid,appl_host,appl_owner,appl_ttyname);
          else
             (void)fprintf(stderr,"%s %s (%d@%s:%s): application controlling tty is %s [detached]\n",
                                           date,appl_name,appl_pid,appl_host,appl_owner,appl_ttyname);
       }

       (void)fflush(stderr);
    }


    /*-----------------------------------------------------------*/
    /* Terminal streams associated with a background process     */
    /* are redirected to the datasink to avoid raising SIGTTIN   */
    /* and SIGTTOU                                               */
    /* Note that we need to have the PUPS file table initialised */
    /* which is why we do the background redirection here        */
    /*-----------------------------------------------------------*/

    if(appl_fgnd == FALSE && appl_nodetach == FALSE)
    {

       /*------------------------------------------------------*/
       /* We need to keep stdin oen here otherwise /dev/tty    */
       /* will go away (and we won't be able to reattach stdio */
       /* if this process is brought into the foreground       */
       /*------------------------------------------------------*/

       if(isatty(0) == 1)
       {  stdin_bg = TRUE;
          (void)fclose(stdin);
          stdin = fopen("/dev/null","r");
       }

       if(isatty(1) == 1)
       {  stdout_bg = TRUE;
          (void)fclose(stdout);
          stdout = fopen("/dev/null","w");
       }

       if(isatty(2) == 1)
       {  stderr_bg = TRUE;
          (void)fclose(stderr);
          stderr = fopen("/dev/null","w");
       }

       started_detached = TRUE;
    }


    /*---------------------------------------------------------------------------*/
    /* Are we a secure application? If so get authentication token (from stdin). */
    /*---------------------------------------------------------------------------*/

    if(pups_locate(&init,"secure",argc,args,0) != NOT_FOUND)
    {  appl_secure = TRUE;


       /*----------------------------------*/
       /* Get password from standard input */
       /*----------------------------------*/

       if(isatty(0) == 0)
       {  (void)fgets(appl_password,SSIZE,stdin);
          appl_secure = TRUE;

          if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): secure application mode %s\n",
                           date,appl_name,appl_pid,appl_host,appl_owner,appl_password);
             (void)fflush(stderr);
          }
       }
       else
       {  

          /*---------------------*/
          /* Prompt for password */
          /*---------------------*/

           int32_t  cnt = 0,
               max_trys = 3;

          char password_banner[SSIZE]  = "";

          do {    int32_t verify_cnt   = 0;

                  char tmp_pw_1[SSIZE] = "",
                       tmp_pw_2[SSIZE] = "";

                  do {   if(verify_cnt > 0)
                         {  (void)fprintf(stdout,"\nnot confirmed -- try again\n\n");
                            (void)fflush(stdout);
                         }

                         (void)snprintf(password_banner,SSIZE,"%s PSRP (protocol %5.2F) password: ",appl_host,PSRP_PROTOCOL_VERSION);
                         (void)strlcpy(tmp_pw_1,getpass(password_banner),SSIZE);
                         (void)snprintf(password_banner,SSIZE,"%s PSRP (protocol %5.2F) verify password:",appl_host,PSRP_PROTOCOL_VERSION);
                         (void)strlcpy(tmp_pw_2,getpass(password_banner),SSIZE);

                         ++verify_cnt;
                      } while(strcmp(tmp_pw_1,tmp_pw_2) != 0);

                  (void)strlcpy(appl_password,tmp_pw_1,SSIZE);
                  ++cnt;

                  if(cnt >= max_trys)
                  {  if(appl_verbose == TRUE)
                     {  (void)strdate(date);
                        (void)fprintf(stdout,"%s %s(%d@%s:%s): too many attempts to set password\n",
                                                       date,appl_name,appl_pid,appl_owner,appl_host);
                        (void)fflush(stdout);
                     }

                     pups_exit(255);
                  }
               } while (strlen(appl_password) == 0);

          appl_secure = TRUE;

          if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stdout,"%s %s(%d@%s:%s): authentication token set for secure application services\n",
                                                                   date,appl_name,appl_pid,appl_owner,appl_host);
             (void)fflush(stdout);
          }
       }
    }

    #ifndef SINGLE_THREAD
    #ifdef _OPENMP
    /*-----------------------------------------------------*/
    /* Set number of OMP threads avaialble to application. */
    /*-----------------------------------------------------*/

    if((ptr = pups_locate(&init,"omp_threads",argc,args,0)) != NOT_FOUND)
    {  if((appl_omp_threads = pups_i_dec(&ptr,argc,args)) == (int)INVALID_ARG || appl_omp_threads < 1)
          pups_error("[pups_std_init] expecting number of OMP threads (integer >= 1)");
       else
       {   omp_set_num_threads(appl_omp_threads); 

           if(appl_verbose == TRUE)
           {  (void)strdate(date);
              (void)fprintf(stdout,"%s %s(%d@%s:%s): %d OMP threads\n",
                            date,appl_name,appl_pid,appl_owner,appl_host,appl_omp_threads);
              (void)fflush(stdout);
           }
       }
    }
    #endif /* _OPENMP       */
    #endif /* SINGLE_THREAD */


    /*---------------------------------------------------------------------------*/
    /* Nominate an effective parent (in the case of system() and similar command */
    /* processors it is not the parent which should be notified of the death of  */
    /* a child, it is the grandparent!).                                         */
    /*---------------------------------------------------------------------------*/

    if((ptr = pups_locate(&init,"parent",argc,args,0)) != NOT_FOUND)
    {  pid_t i_tmp;
       char  pname[SSIZE] = "";

       if((i_tmp = pups_i_dec(&ptr,argc,args)) == (int32_t)INVALID_ARG)
       {  --ptr; 
          if(strccpy(pname,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
             pups_error("[pups_std_init] expecting effective parent PID or process name");

          if((i_tmp = psrp_pname_to_pid(pname)) < 0)
          {  if(i_tmp == PSRP_DUPLICATE_PROCESS_NAME)
                pups_error("[pups_std_init] ambiguous parent process name specified");
             else
                pups_error("[pups_std_init] effective parent does not exist");
          }
       }   
       else
       {  if(psrp_pid_to_pname(i_tmp,pname) == FALSE)
             pups_error("[pups_std_init] effective parent does not exist");
       }

       appl_ppid = i_tmp;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): effective parent is %d (%s@%s)\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,appl_ppid,pname,appl_host);
          (void)fflush(stderr);
       }
    }


    /*-----------------------------------------------*/
    /* Are we to exit if our nominated parent exits? */
    /*-----------------------------------------------*/

    if(pups_locate(&init,"parent_exit",argc,args,0) != NOT_FOUND)
    {  appl_ppid_exit = TRUE; 

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): process will exit if effective parent terminates\n",
                                                date,appl_name,appl_pid,appl_host,appl_owner,appl_pid);
          (void)fflush(stderr);
       }
    }


    /*-----------------------------------------------*/
    /* Rooted processes cannot migrate to a new host */     
    /*-----------------------------------------------*/

    if(pups_locate(&init,"rooted",argc,args,0) != NOT_FOUND)
    {  appl_rooted = TRUE;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): process is rooted (system context cannot migrate to new host)\n",
                                                             date,appl_name,appl_pid,appl_host,appl_owner,appl_pid);
          (void)fflush(stderr);
       }
    }    


    /*-----------------------------------------------------------------------*/
    /* If process has tunneled to a new host tell its invocation on the old  */
    /* host that tunneling has completed without errors.                     */
    /*-----------------------------------------------------------------------*/

    if((ptr = pups_locate(&init,"tunnel",argc,args,0)) != NOT_FOUND)
    {

       /*----------------------------------------------------*/
       /* We need to set path to (tunnel) binary and context */
       /*----------------------------------------------------*/

       if(strccpy(appl_tunnel_path,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
                  (void)snprintf(appl_tunnel_path,SSIZE,"/tmp/tunnel.%d.tmp",appl_pid);


       /*-----------------------------------------------------------*/
       /* We must make sure that binary name point to tunnel binary */
       /*-----------------------------------------------------------*/

       (void)strlcpy(appl_bin_name,"tunnel",SSIZE);

       #ifdef UTILIB_DEBUG
       (void)fprintf(stdrr,"UTILIB OK %s [%d@%s]\n",appl_name,appl_pid,appl_host);
       (void)fflush(stdout);
       #endif /* UTILIB_DEBUG */

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): process has tunneled to current host (tunnel path is \"%s\")\n",
                                                    date,appl_name,appl_pid,appl_host,appl_owner,appl_tunnel_path);
          (void)fflush(stderr);
       }
    }


    /*-----------------------------------------------------------*/
    /* If we need to close (and reopen) stdio -- lets do it now. */
    /*-----------------------------------------------------------*/

    if(pups_locate(&init,"closestdio",argc,args,0) != NOT_FOUND)
    {

       /*----------------------------------------------------------*/
       /* Reopen stdin, stdout or stderr if asscoiated with a FIFO */
       /*----------------------------------------------------------*/

       (void)fstat(0,&buf);
       if(S_ISFIFO(buf.st_mode))
       {  (void)fclose(stdin);
          stdin = fopen("/dev/tty","r");
       }

       (void)fstat(1,&buf);
       if(S_ISFIFO(buf.st_mode))
       {  (void)fclose(stdout);
          stdout = fopen("/dev/tty","w");
       }

       (void)fstat(2,&buf);
       if(S_ISFIFO(buf.st_mode))
       {  (void)fclose(stderr);
          stderr = fopen("/dev/tty","w");
       }
    }


    /*----------------------------------------*/
    /* Associate stderr with (named) logfile. */
    /*----------------------------------------*/

    if((ptr = pups_locate(&init,"log",argc,args,0)) != NOT_FOUND)
    {  if(strccpy(appl_logfile,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          pups_error("[pups_std_init] expecting name of error/log file");

       (void)fclose(stderr);
       stderr = fopen(appl_logfile,"w");
    } 


    #ifdef SSH_SUPPORT
    /*---------------------------------------------*/
    /* Set ssh parameters (for remote connection). */
    /*---------------------------------------------*/
    /*----------*/
    /* Set port */
    /*----------*/

    if((ptr = pups_locate(&init,"ssh_port",argc,args,0)) != NOT_FOUND)
    {  int32_t idum;

       if(strccpy(ssh_remote_port,pups_str_dec(&ptr,argc,args)) == (char *)INVALID_ARG)
          pups_error("[pups_std_init] expecting (remote ssh) port name"); 

       if(sscanf(ssh_remote_port,"%d",&idum) != 1 || idum < 0)
          pups_error("[pups_std_init] expecting integer value (>= 0)");

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): secure shell (ssh) remote port is \"%s\"\n",
                                 date,appl_name,appl_pid,appl_host,appl_owner,ssh_remote_port);
          (void)fflush(stderr);
       }
    }


    /*-------------------------------------*/
    /* Is ssh connection to be compressed? */
    /*-------------------------------------*/

    if(pups_locate(&init,"ssh_compress",argc,args,0) != NOT_FOUND)
    {  ssh_compression = TRUE;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): secure shell (ssh) compression enabled\n",
                                               date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }
    }


    /*-------------------------------------*/
    /* Default (for new instance creation) */
    /*-------------------------------------*/

    (void)strlcpy(ssh_remote_uname,appl_owner,SSIZE);
    #endif /* SSH_SUPPORT */


    /*-----------------------------------------------------*/
    /* Are we going to wait for debugger attachment in the */
    /* event of an error?                                  */
    /*-----------------------------------------------------*/

    if((ptr = pups_locate(&init,"etrap",argc,args,0)) != NOT_FOUND)
    {  appl_etrap = TRUE;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): errors trapped (for subsequent analysis by gdb)\n",
                                                        date,appl_name,appl_pid,appl_host,appl_owner); 
          (void)fflush(stderr);
       }
    }


    /*--------------------------------*/
    /* Get memory allocation options. */
    /*--------------------------------*/

    if(pups_locate(&init,"malloc_homeostatic",argc,args,0) != NOT_FOUND)
    {  appl_alloc_opt = MALLOC_HOMEOSTATIC;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): memory allocation homeostatic\n",
                                      date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }
    }
    else if(getenv("MALLOC_HOMEOSTATIC") != (char *)NULL)
    {  appl_alloc_opt = MALLOC_HOMEOSTATIC;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): memory allocation homeostatic\n",
                                      date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }
    }
    else
       appl_alloc_opt = MALLOC_DUMB;
        

    /*--------------------------------------------------------------------*/
    /* Are we going to encrypt shadow files? This is done if we expect a  */
    /* serious attempt to delete our resources is likely to be made.      */
    /*--------------------------------------------------------------------*/

    if(pups_locate(&init,"crypt_shadows",argc,args,0) != NOT_FOUND)
    {  appl_snames_crypted = TRUE;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): encryption for shadow files enabled\n",
                                            date,appl_name,appl_pid,appl_host,appl_owner); 
  
          (void)fflush(stderr);
       }
    } 


    /*---------------------------------------------------------------------*/
    /* Set application FIFO directory to user defined directory -- this    */
    /* is done if the user wants to create a private patchboard (which may */
    /* or may not be local).                                               */
    /*---------------------------------------------------------------------*/

    if((ptr = pups_locate(&init,"patchboard",argc,args,0)) != NOT_FOUND)
    {  if(strccpy(appl_fifo_dir,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          (void)strlcpy(appl_fifo_dir,"/tmp",SSIZE);
       else
       {  if(access(appl_fifo_dir,F_OK | R_OK | W_OK | X_OK) == (-1))
             pups_error("[pups_std_init] cannot access (private) patchboard");
       }

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): PSRP patchboard is \"%s\" (default patchboard setting overidden)\n",
                                                           date,appl_name,appl_pid,appl_host,appl_owner,appl_fifo_dir);
          (void)fflush(stderr);
       }
    } 

    #ifdef MAIL_SUPPORT
    /*-------------------------------------------------------------------------*/
    /* Set application MIME message storage/worspace directory. This directory */
    /* is used as workspace when reading mail messages addressed to            */
    /* this process.                                                           */
    /*-------------------------------------------------------------------------*/

    appl_mail_handler = (void *)NULL;

    (void)strlcpy(appl_mdir,     "notset",SSIZE);
    (void)strlcpy(appl_mh_folder,"notset",SSIZE);
    (void)strlcpy(appl_mime_dir,"notset",SSIZE);
    (void)strlcpy(appl_mime_type,"all",SSIZE);

    if((ptr = pups_locate(&init,"mail_dir",argc,args,0)) != NOT_FOUND)
    {  (void)snprintf(appl_mdir,SSIZE,     "%s/Mail/%s.%d.pinbox",appl_home,appl_name,appl_pid);
       (void)snprintf(appl_mh_folder,SSIZE,"%s.%d.pinbox",appl_name,appl_pid);

       if(access(appl_mdir,F_OK) == (-1))
       {  if(mkdir(appl_mdir,0700) == (-1))
             pups_error("[pups_std_init] cannot create mail directory");
       }

       (void)snprintf(appl_mime_dir,SSIZE,"%s.mime",appl_mdir);
       if(access(appl_mime_dir,F_OK) == (-1))
       {  if(mkdir(appl_mime_dir,0700) == (-1))
             pups_error("[pups_std_init] cannot create mime directory");
       }
       
       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): PSRP mail directory is \"%s\"\n",
                                 appl_name,appl_pid,appl_host,appl_owner,appl_mdir);
          (void)fflush(stderr);
       }     
    }
    else
       appl_mailable = FALSE;
    #endif /* MAIL_SUPPORT */


    /*-------------------------------------------------------------*/
    /* Authentication module for protected PSRP server operations. */
    /*-------------------------------------------------------------*/

    if(pups_locate(&init,"pam",argc,args,0) != NOT_FOUND)
    {  if(strccpy(appl_pam_name,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          pups_error("[pups_std_init] expecting PSRP server authentication module name");
    }


    /*-------------------------------*/
    /* Initialise table of children. */
    /*-------------------------------*/

    if((ptr = pups_locate(&init,"chtab",argc,args,0)) != NOT_FOUND)
    {  if((appl_max_child = pups_i_dec(&ptr,argc,args)) == (int32_t)INVALID_ARG)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): expecting number of [PUPS] child table slots\n",
                                                     date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);

          exit(255);
       }
    }
    else if(getenv("PUPS_CHILDREN") != (char *)NULL)
    {  appl_max_child = strtol("PUPS_CHILDREN", (char **)NULL, 10);
       if(pups_get_errno() == ERANGE)
          appl_max_child = MAX_CHILDREN;
    }
    else
       appl_max_child = MAX_CHILDREN;

    (void)pups_init_child_table(appl_max_child);

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): %d slots in PUPS child table\n",
                     date,appl_name,appl_pid,appl_host,appl_owner,appl_max_child);
       (void)fflush(stderr);
    }


    /*--------------------------------------*/
    /* Initialise orifaces (to PUPS DLL's). */
    /*--------------------------------------*/

    #ifdef DLL_SUPPORT
    if((ptr = pups_locate(&init,"ortab",argc,args,0)) != NOT_FOUND)
    {  if((appl_max_orifices = pups_i_dec(&ptr,argc,args)) == (int32_t)INVALID_ARG)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): expecting number of [PUPS] DLL orifice table slots\n",
                                                           date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);

          exit(255);
       }
    }
    else if(getenv("PUPS_ORIFICES") != (char *)NULL)
    {  appl_max_orifices = strtol(getenv("PUPS_ORIFICES"), (char **)NULL, 10);
       if(pups_get_errno() == ERANGE)
          appl_max_orifices = MAX_ORIFICES;
    }
    else
       appl_max_orifices = MAX_ORIFICES;

    ortab_init(appl_max_orifices);

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): %d slots in PUPS DLL orifice table\n",
                      date,appl_name,appl_pid,appl_host,appl_owner,appl_max_orifices);
       (void)fflush(stderr);
    }
    #endif /* DLL_SUPPORT */


    /*---------------------*/
    /* Initialise vtimers. */
    /*---------------------*/

    if((ptr = pups_locate(&init,"vitab",argc,args,0)) != NOT_FOUND)
    {  if((appl_max_vtimers = pups_i_dec(&ptr,argc,args)) == (int32_t)INVALID_ARG)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): expecting number of [PUPS] virtual timer table slots\n",
                                                             date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);

          exit(255);
       }
    }
    else if(getenv("PUPS_VTIMERS") != (char *)NULL)
    {  appl_max_vtimers = strtol(getenv("PUPS_VTIMERS"), (char **)NULL, 10);
       if(pups_get_errno() == ERANGE)
          appl_max_vtimers = MAX_VTIMERS;
    }
    else
       appl_max_vtimers = MAX_VTIMERS;

    initvitimers(appl_max_vtimers);

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): %d slots in PUPS virtual timer table\n",
                         date,appl_name,appl_pid,appl_host,appl_owner,appl_max_vtimers);
       (void)fflush(stderr);
    }


    #ifdef CRIU_SUPPORT
    /*--------------------------------------------*/
    /* Periodically save (process) state via Criu */
    /*--------------------------------------------*/

    if((ptr = pups_locate(&init,"ssave",argc,args,0)) != NOT_FOUND)
    {   

        /*-------------------------------------*/
        /* Get poll time (between state saves) */
        /*-------------------------------------*/

        if((ptr = pups_locate(&init,"sppoll",argc,args,0)) != NOT_FOUND)
        {   int32_t itmp;


            /*-------------------------------------*/
            /* Get poll time (between state saves) */
            /*-------------------------------------*/

            if((itmp = pups_i_dec(&ptr,argc,args)) == (int)INVALID_ARG)
                  pups_error("[pups_std_init] expecting criu (ssave) poll time in seconds (integer >= 0)");
            else if(itmp > 0)
               appl_poll_time = itmp;
            else
               pups_error("[pups_std_init] expecting criu (ssave) poll time in seconds (integer >= 0)");
        }


        /*-----------------------------------------------------------------------*/
        /* Get name of (Criu) directory (holds migratable files and checkpoints) */
        /*-----------------------------------------------------------------------*/

        if((ptr = pups_locate(&init,"cd",argc,args,0)) != NOT_FOUND)
        {  char tmpstr[SSIZE] = "";

            if(strccpy(tmpstr,pups_str_dec(&ptr,argc,args)) == (char *)INVALID_ARG)
               pups_error("[pups_std_init] expecting criu directory name");
            else
               (void)strlcpy(appl_criu_dir,tmpstr,SSIZE); 
        }

        (void)pups_ssave_enable();

        if(appl_verbose == TRUE)
        {  (void)strdate(date);
           (void)fprintf(stderr,"%s %s (%d@%s:%s): criu state saving enabled (poll time: %d, Criu directory: \"%s\")\n",
                                              date,appl_name,appl_pid,appl_host,appl_owner,appl_poll_time,appl_criu_dir);
           (void)fflush(stderr);
        }
    }


    /*----------------------------------*/
    /* Install handler for state saving */
    /*----------------------------------*/

    (void)pups_sighandle(SIGCHECK,"ssave_handler",(void *)ssave_handler,(sigset_t *)NULL);
    #endif /* CRIU_SUPPORT */


    /*-------------------------*/
    /* Start software watchdog */
    /*-------------------------*/

    if((ptr = pups_locate(&init,"softdog",argc,args,0)) != NOT_FOUND)
    {   

        /*----------------------*/
        /* Get watchdog timeout */
        /*----------------------*/

        if((ptr = pups_locate(&init,"timeout",argc,args,0)) != NOT_FOUND)
        {   int32_t itmp;


            /*-------------------------------------*/
            /* Get poll time (between state saves) */
            /*-------------------------------------*/

            if((itmp = pups_i_dec(&ptr,argc,args)) == (int)INVALID_ARG)
                  pups_error("[pups_std_init] expecting criu (ssave) poll time in seconds (integer >= 0)");
            else if(itmp > 0)
               appl_softdog_timeout = itmp;
            else
               pups_error("[pups_std_init] expecting wtchdog timeout in seconds (integer >= 0)");
        }

        (void)pups_start_softdog(appl_softdog_timeout);

        if(appl_verbose == TRUE)
        {  (void)strdate(date);
           (void)fprintf(stderr,"%s %s (%d@%s:%s): software watchdog enabled (pid: %d  timout: %d)\n",
                         date,appl_name,appl_pid,appl_host,appl_owner,appl_softdog_pid,appl_softdog_timeout);
           (void)fflush(stderr);
        }
    }


    /*------------------------------------*/
    /* Start (effective) parent homeostat */
    /*------------------------------------*/

    if(appl_ppid_exit == TRUE)
       (void)pups_setvitimer("default_parent_homeostat",1,VT_CONTINUOUS,1,NULL,(void *)&pups_default_parent_homeostat);


    #ifdef PERSISTENT_HEAP_SUPPORT
    /*-----------------------------*/
    /* Initialise persistent heaps */
    /*-----------------------------*/

    if((ptr = pups_locate(&init,"phtab",argc,args,0)) != NOT_FOUND)
    {  if((appl_max_pheaps = pups_i_dec(&ptr,argc,args)) == (int)INVALID_ARG)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): expecting number of [PUPS] persistent heap table slots\n",
                                                               date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);

          exit(255);
       }
    }
    else if(getenv("PUPS_PHEAPS") != (char *)NULL)
    {  appl_max_pheaps = strtol(getenv("PUPS_PHEAPS"), (char **)NULL, 10);
       if(pups_get_errno() == ERANGE)
          appl_max_pheaps = MAX_PHEAPS;
    }
    else
       appl_max_pheaps = MAX_PHEAPS;

    msm_init(appl_max_pheaps);

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): %d slots in PUPS persistent heap table\n",
                            date,appl_name,appl_pid,appl_host,appl_owner,appl_max_pheaps);
       (void)fflush(stderr);
    }
    #endif /* PERSISTENT_HEAP_SUPPORT */


    /*-----------------------------*/
    /* Initialise PUPS file table. */
    /*-----------------------------*/

    if((ptr = pups_locate(&init,"ftab",argc,args,0)) != NOT_FOUND)
    {  if((appl_max_files = pups_i_dec(&ptr,argc,args)) == (int)INVALID_ARG)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): expecting number of (PUPS) file table slots\n",
                                                    date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);

          exit(255);
       }
    }
    else if(getenv("PUPS_FILES") != (char *)NULL)
    {  appl_max_files = strtol(getenv("PUPS_FILES"), (char **)NULL, 10);
       if(pups_get_errno() == ERANGE)
          appl_max_files = MAX_FILES;
    }
    else
       appl_max_files = MAX_FILES;


    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): %d slots in PUPS file table\n",
                     date,appl_name,appl_pid,appl_host,appl_owner,appl_max_files);
       (void)fflush(stderr);
    }


    /*-------------------------------------------------*/
    /* Disable VT services (mainly used for debugging) */
    /*-------------------------------------------------*/

    if(pups_locate(&init,"no_vt_services",argc,args,0) != NOT_FOUND)
    {  no_vt_services = TRUE;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): VT services disabled\n",
                             date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }
    }


    /*------------------------------------------*/
    /* Start cron services for this application */
    /*------------------------------------------*/

    (void)psrp_crontab_init();


    /*-------------------------------------------------------------*/
    /* We must ALWAYS have ftab slots for stdin, stdout and stderr */
    /*-------------------------------------------------------------*/

    if(appl_max_files < 3)
       appl_max_files = 3;

    if(pups_locate(&init,"stdio_dead",argc,args,0) != NOT_FOUND)
       initftab(appl_max_files,STDIO_DEAD);
    else
       initftab(appl_max_files,STDIO_LIVE);


    #ifdef SSH_SUPPORT
    /*----------------------------------*/
    /* (Simple) migrate (if requested). */
    /*----------------------------------*/

    if((ptr = pups_locate(&init,"on",argc,args,0)) != NOT_FOUND)
    {  _BOOLEAN do_bg       = FALSE;
        char    r_host[SSIZE] = "";

       if(strccpy(r_host,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          pups_error("[pups_std_init] expecting name of host (to migrate to)");

       if(pups_locate(&init,"bg",argc,args,0) != NOT_FOUND)
          do_bg = TRUE;

       if(strcmp(appl_host,r_host) != 0)
       {  if(psrp_cmd_on_host(r_host,do_bg,*argc,args) == (-1))
             pups_error("[pups_std_init] remote start failed");
       }
    }
    #endif /* SSH_SUPPORT */


    /*------------------------------------------------------------*/
    /* Terminal streams associated with a background process      */
    /* are redirected to the datasink to avoid raising SIGTTIN    */
    /* and SIGTTOU                                                */
    /* Amend PUPS file table to reflect that we are in background */
    /*------------------------------------------------------------*/

    if(appl_fgnd == FALSE)
    {  if(stdin_bg == TRUE)
       {  ftab[0].named = TRUE;
          (void)strlcpy(ftab[0].fname,  "/dev/null",SSIZE);
          (void)strlcpy(ftab[0].fshadow,"/dev/null",SSIZE);
       }

       if(stdout_bg == TRUE)
       {  ftab[1].named = TRUE;
          (void)strlcpy(ftab[1].fname,  "/dev/null",SSIZE);
          (void)strlcpy(ftab[1].fshadow,"/dev/null",SSIZE);
       }

       if(stderr_bg == TRUE)
       {  ftab[2].named = TRUE;
          (void)strlcpy(ftab[2].fname,  "/dev/null",SSIZE);
          (void)strlcpy(ftab[2].fshadow,"/dev/null",SSIZE);
       }
    }


    /*-------------------------------------------------------------------------*/
    /* If this process has been relabeled get the execution name of the binary */
    /* that it is running.                                                     */
    /*-------------------------------------------------------------------------*/

    if((ptr = pups_locate(&init,"bin",argc,args,0)) != NOT_FOUND)
    {  char tmp[SSIZE] = "";

       if(strccpy(tmp,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          pups_error("[pups_std_init] expecting name of application binary");

       #ifdef UTILIB_DEBUG
       (void)fprintf(stderr,"UTILIB APPL BIN NAME IN: %s\n",tmp);
       (void)fflush(stderr);
       #endif /* UTILIB_DEBUG */


       /*------------------------------------*/
       /* Abstract binary name from its path */
       /*------------------------------------*/


       if(strrextr(tmp, appl_bin_name, '/') == FALSE)
          (void)strccpy(appl_bin_name,tmp);

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): executing binary %s\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,appl_bin_name);
          (void)fflush(stderr);
       }
    }


    #ifdef UTILIB_DEBUG
    (void)fprintf(stderr,"UTILIB APPL NAME:     %s  [%d]\n",appl_name,appl_pid);
    (void)fprintf(stderr,"UTILIB APPL BIN NAME: %s  [%d]\n",appl_bin_name,appl_pid);
    (void)fflush(stderr);
    #endif /* UTILIB_DEBUG */


    /*-----------------------------------------------------------*/
    /* If appl_verbose switch is set, report progress to stderr. */
    /*-----------------------------------------------------------*/

    if((ptr = pups_locate(&init,"version",argc,args,0)) != NOT_FOUND)
    {  ssize_t i,
               pos;

       char up_appl_name[SSIZE] = "";

       if(strin(appl_bin_name,"/") == TRUE && appl_bin_name != (char *)NULL)
       {  pos = strlen(appl_bin_name) + 1;
          do {    --pos;
             } while(appl_bin_name[pos] != '/');

          if(pos > 0)
             (void)strlcpy(up_appl_name,&appl_bin_name[pos+1],SSIZE);
          else
             (void)strlcpy(up_appl_name,appl_bin_name,SSIZE);    
       }
       else
          (void)strlcpy(up_appl_name,appl_bin_name,SSIZE);

       if(appl_bin_name != (char *)NULL)
          (void)fprintf(stderr,
                  "    %s [PEN %s] version %s [build %.5d (gcc %s: built %s %s)] (C) %s\n",up_appl_name,
                                                                                              appl_name,
                                                                                              version,
                                                                                              appl_vtag,
                                                                                              __VERSION__,
                                                                                              appl_build_time,
                                                                                              appl_build_date,
                                                                                                       author);
       else
          (void)fprintf(stderr,
                        "    %s version %s [build %.5d (gcc %s: built %s %s)] (C) %s\n",up_appl_name,
                                                                                        version,
                                                                                        appl_vtag,
                                                                                        __VERSION__,
                                                                                        appl_build_time,
                                                                                        appl_build_date,
                                                                                                 author);

       #ifdef FLOAT
       (void)fprintf(stderr,"    Floating point representation is single precision (%d bytes)\n",sizeof(FTYPE));
       #else
       (void)fprintf(stderr,"    Floating point representation is double precision (%d bytes)\n",sizeof(FTYPE));
       #endif /* FLOAT */

       #ifndef SINGLE_THREADED
       #ifdef _OPENMP
       (void)fprintf(stderr,"    %d parallel (OMP) threads available\n",appl_omp_threads);
       #else
       (void)fprintf(stderr,"    OMP disabled\n");
       #endif /* _OPENMP       */
       #endif /* SINGLE_THREAD */

       (void)fprintf(stderr,"\n");

        #ifdef SHADOW_SUPPORT
        (void)fprintf(stderr,"    [with shadow support                  ]\n");
        #endif /* SHADOW_SUPPORT */

        #ifdef NIS_SUPPORT
        (void)fprintf(stderr,"    [with NIS support                     ]\n");
        #endif /* NIS_SUPPORT */

        #ifdef SSH_SUPPORT
        (void)fprintf(stderr,"    [with ssh support                     ]\n");
        #endif /* SSH_SUPPORT */

        #ifdef IPV6_SUPPORT
        (void)fprintf(stderr,"    [with IPV6 support                    ]\n");
        #endif /* IPV6_SUPPORT */

        #ifdef DLL_SUPPORT
        (void)fprintf(stderr,"    [with DLL support                     ]\n");
        #endif /* DLL_SUPPORT */

        #ifdef PTHREAD_SUPPORT
        (void)fprintf(stderr,"    [with thread support                  ]\n");
        #endif /* PTHREAD_SUPPORT */

        #ifdef PERSISTENT_HEAP_SUPPORT
        (void)fprintf(stderr,"    [with persistent heap support         ]\n");
        #endif /* PERSISTENT_HEAP_SUPPORT */

        #ifdef BUBBLE_MEMORY_SUPPORT 
        (void)fprintf(stderr,"    [with dynamic bubble memory support   ]\n");
        #endif /* BUBBLE_MEMORY_SUPPORT */

        #ifdef CRIU_SUPPORT
        (void)fprintf(stderr,"    [with state saving support            ]\n");
        #endif /* CRUI_SUPPORT */


       for(i=0; i<strlen(up_appl_name) + 1; ++i)
           up_appl_name[i] = toupper(up_appl_name[i]);

       if(appl_proprietary) 
       {  (void)fprintf(stderr,"\n%s (C) Tumbling Dice\n",up_appl_name);
          (void)fprintf(stderr,"All rights reserved 2025\n\n");
       }
       else
       {  (void)fprintf(stderr,"\n%s is free software, covered by the GNU General Public License, and you are\n",up_appl_name);
          (void)fprintf(stderr,"welcome to change it and/or distribute copies of it under certain conditions.\n");
          (void)fprintf(stderr,"See the GPL and LGPL licences at www.gnu.org for further details\n");
          (void)fprintf(stderr,"%s comes with ABSOLUTELY NO WARRANTY\n\n",up_appl_name);
       }
       (void)fflush(stderr);


       /*-------------------------------------------------------*/
       /* Just in case .psrprc file has set appl_verbose = TRUE */
       /*-------------------------------------------------------*/
 
       appl_verbose = FALSE;
       pups_exit(255);
    }


    /*--------------------------------*/
    /* Display usage for application. */
    /*--------------------------------*/

    if((ptr = pups_locate(&init,"usage",argc,args,0)) != NOT_FOUND)
    {   int32_t  i;
       char up_appl_name[SSIZE] = "";

       (void)fprintf(stderr,
                     "\n\n%s version %s (C) %s (gcc %s: built %s %s)\n",appl_name,
                                                                        version,
                                                                        author,
                                                                        __VERSION__,
                                                                        __TIME__,
                                                                        __DATE__);

       if(open_source == TRUE)
       {  if(appl_bin_name != (char *)NULL && strcmp(appl_bin_name,"") != 0)
          {  int32_t pos;

             pos = strlen(appl_bin_name);
             do {    --pos;
                } while(appl_bin_name[pos] != '/' && pos > (-1));

             if(pos > 0)
                (void)strlcpy(up_appl_name,&appl_bin_name[pos+1],SSIZE);
             else
                (void)strlcpy(up_appl_name,appl_name,SSIZE);
          }
          else
             (void)strlcpy(up_appl_name,appl_bin_name,SSIZE);

          for(i=0; i<strlen(up_appl_name); ++i)
              up_appl_name[i] = toupper(up_appl_name[i]);

          up_appl_name[strlen(appl_name)] = '\0';

          if(appl_proprietary == TRUE)
          {  (void)fprintf(stderr,"\n%s (C) Tumbling Dice\n",up_appl_name);
             (void)fprintf(stderr,"All rights reserved\n\n");
          }
          else
          {  (void)fprintf(stderr,"\n%s is free software, covered by the GNU General Public License, and you are\n",up_appl_name);
             (void)fprintf(stderr,"welcome to change it and/or distribute copies of it under certain conditions.\n");
             (void)fprintf(stderr,"See the GPL and LGPL licences at www.gnu.org for further details\n");
             (void)fprintf(stderr,"%s comes with ABSOLUTELY NO WARRANTY\n\n",up_appl_name);
          }
          (void)fflush(stderr);
       }

       (void)fflush(stderr);


       /*-----------------------*/
       /* usage() never returns */
       /*-----------------------*/

       usage();
    }


    /*---------------------------------------*/
    /* Display segments of this application. */
    /*---------------------------------------*/


    /*----------------------------*/
    /* Shortform slot information */
    /*----------------------------*/

    if((ptr = pups_locate(&init,"slots",argc,args,0)) != NOT_FOUND)
    {  (void)fprintf(stderr,
              "\n\n    %s version %s (C) %s, %s\n",appl_name,
                                                     version,
                                                      author,
                                                        date);
       (void)fflush(stderr);


       /*----------------------------*/
       /* slot_usage() never returns */
       /*----------------------------*/

       slot_usage(0);
    }


    /*---------------------------*/
    /* Longform slot information */
    /*---------------------------*/

    if((ptr = pups_locate(&init,"slotinfo",argc,args,0)) != NOT_FOUND)
    {  (void)fprintf(stderr, "\n\n    %s version %s (C) %s, %s\n",appl_name,
                                                                    version,
                                                                     author,
                                                                       date);
       (void)fflush(stderr);


       /*----------------------------*/
       /* slot_usage() never returns */
       /*----------------------------*/

       slot_usage(2);
    }


    /*------------------------------------------------------------------------*/
    /* Get vtag - this is used by various agents which clean up stale dynamic */
    /* objects.                                                               */
    /*------------------------------------------------------------------------*/

    if(pups_locate(&init,"vtag",argc,args,0) != NOT_FOUND)
    {  (void)fprintf(stderr,"%s: vtag is %d\n",appl_name,appl_vtag);
       (void)fflush(stderr);

       exit(0);
    }

    if(pups_locate(&init,"no_vtag",argc,args,0) != NOT_FOUND)
    {  appl_vtag = NO_VERSION_CONTROL;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);

          (void)fprintf(stderr,"%s %s (%d@%s:%s): version control disabled (vtag: %d)\n",
                                  date,appl_name,appl_pid,appl_host,appl_owner,appl_vtag);
          (void)fflush(stderr);
       }
    }


    /*------------------------------------------------*/
    /* Disable loading of default PSRP resource file. */
    /*------------------------------------------------*/

    if(appl_psrp == TRUE && pups_locate(&init,"psrp_autoload",argc,args,0) != NOT_FOUND)
    {  appl_psrp_load = TRUE;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);

          (void)fprintf(stderr,"%s %s (%d@%s:%s): loading default PSRP resource file (~.%s.psrp) enabled\n",
                                                     date,appl_name,appl_pid,appl_host,appl_owner,appl_name);
          (void)fflush(stderr);
       }
    }


    /*-----------------------------------------------*/
    /* Disable saving of default PSRP resource file. */
    /*-----------------------------------------------*/

    if(appl_psrp == TRUE && pups_locate(&init,"psrp_autosave",argc,args,0) != NOT_FOUND)
    {  appl_psrp_save = TRUE;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);

          (void)fprintf(stderr,"%s %s (%d@%s:%s): saving default PSRP resource file (~.%s.psrp) enabled\n",
                                                    date,appl_name,appl_pid,appl_host,appl_owner,appl_name);
          (void)fflush(stderr);
       }
    }


    /*------------------------------------------------------*/
    /* Tell this process which process group it is to join. */
    /*------------------------------------------------------*/

    if((ptr = pups_locate(&init,"pgrp",argc,args,0)) != NOT_FOUND)
    {  pid_t pg_leaders_pid;

       if((pg_leaders_pid = pups_i_dec(&ptr,argc,args)) == (int)INVALID_ARG)
          pups_error("[pups_std_init] expecting pid of process group leader");

       if(setpgid(0,pg_leaders_pid) < 0)
          pups_error("[pups_std_init] could not join specified process group");
    }


    /*-----------------------------------------------------------------*/
    /* Set application name -- makes sure that channel names for PSRP  */
    /* servers remain conistent.                                       */
    /*-----------------------------------------------------------------*/

    if(appl_psrp == TRUE)
    {  if((ptr = pups_locate(&init,"channel_name",argc,args,0)) != NOT_FOUND)
       {  char tmp_name[SSIZE] = "";

          if(strccpy(tmp_name,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
             pups_error("[pups_std_init] expecting application channel name");

          (void)snprintf(appl_ch_name,SSIZE,"%s#%s",tmp_name,appl_host);
          appl_default_chname = FALSE;
       }
       else
          (void)snprintf(appl_ch_name,SSIZE,"%s#%s",appl_name,appl_host);

       if(appl_verbose == TRUE)
       {  (void)strdate(date);

          (void)fprintf(stderr,"%s %s (%d@%s:%s): application channel name (PSRP) is %s\n",
                                 date,appl_name,appl_pid,appl_host,appl_owner,appl_ch_name);
          (void)fflush(stderr);
       }
    }
    else
    {  if(appl_verbose == TRUE)
       {  (void)strdate(date);

          (void)fprintf(stderr,"%s %s (%d@%s:%s): this application is not PSRP enabled\n",
                                             date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }
    }


    /*------------------------------------------------------------------------*/
    /* Tell remote client that it can resume an interrupted PSRP transaction. */
    /*------------------------------------------------------------------------*/
 
    if((ptr = pups_locate(&init,"psrp_reactivate_client",argc,args,0)) != NOT_FOUND)
    {

       /*--------------------------------------------------------------*/
       /* Note we cannot take the absolute value of chlockdes beacause */
       /* a negative value implies we have no PSRP channel active     */
       /*--------------------------------------------------------------*/

       if((chlockdes = pups_i_dec(&ptr,argc,args)) == (int)INVALID_ARG)
          pups_error("[pups_std_init] expecting PSRP channel lock descriptor");

       psrp_reactivate_client = TRUE;

       if(appl_verbose == TRUE) 
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): Reactivating PSRP clients (segment %d)\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,psrp_client_pid[c_client],psrp_seg_cnt);
          (void)fflush(stderr);
       }
    }


    /*-----------------------------*/
    /* Update server segment count */
    /*-----------------------------*/

    if((ptr = pups_locate(&init,"psrp_segment",argc,args,0)) != NOT_FOUND)
    {  if((psrp_seg_cnt = pups_i_dec(&ptr,argc,args)) == (int)INVALID_ARG || psrp_seg_cnt < 0)
          pups_error("[pups_std_init] expecting PSRP server segment number");

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): segment is %d\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,psrp_seg_cnt);
          (void)fflush(stderr);
       }
    }


    /*-------------*/
    /* Signal mask */
    /*-------------*/

    (void)sigemptyset(&blocked_set);
    (void)sigaddset(&blocked_set,SIGTSTP);
    (void)sigaddset(&blocked_set,SIGCONT);
    (void)sigaddset(&blocked_set,SIGTERM);
    (void)sigaddset(&blocked_set,SIGQUIT);
    (void)sigaddset(&blocked_set,SIGINT);

    if(pups_locate(&init,"kill_pg",argc,args,0) != NOT_FOUND)
    {  appl_kill_pg = TRUE;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): process will not terminate process group at exit\n",
                                                         date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }
    }
 

    /*--------------------------------------------------------------------------*/
    /* Are we going to be a leader of processes? If we are then set up handlers */
    /* to control our subordinates! If we are a foreground process we can never */
    /* assume this exaulted position as the shell has pre-empted us!            */
    /*--------------------------------------------------------------------------*/

    if(pups_locate(&init,"pg_leader",argc,args,0) != NOT_FOUND)
    {  if(appl_fgnd == FALSE)
       {  (void)setsid();

          if(setpgid(0,getpid()) < 0)
             pups_error("[pups_std_init] could not set process group leader");

          (void)pups_sighandle(SIGTERM,"pg_leaders_term_handler",(void *)pg_leaders_term_handler,&blocked_set);
          (void)pups_sighandle(SIGTSTP,"pg_leaders_stop_handler",(void *)pg_leaders_stop_handler,&blocked_set);
          (void)pups_sighandle(SIGCONT,"pg_leaders_cont_handler",(void *)pg_leaders_cont_handler,&blocked_set);

          if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): process group leader\n",
                                date,appl_name,appl_pid,appl_host,appl_owner);
             (void)fflush(stderr);
          }
          appl_pg_leader = TRUE;
       }
    }
    else if(pups_locate(&init,"session_leader",argc,args,0) != NOT_FOUND)
    {  if(appl_fgnd == FALSE)
       {  if(setsid() < 0)
             pups_error("[pups_std_init] could not set session leader");

          (void)pups_sighandle(SIGTERM,"pg_leaders_term_handler",(void *)pg_leaders_term_handler,&blocked_set);
          (void)pups_sighandle(SIGTSTP,"pg_leaders_stop_handler",(void *)pg_leaders_stop_handler,&blocked_set);
          (void)pups_sighandle(SIGCONT,"pg_leaders_cont_handler",(void *)pg_leaders_cont_handler,&blocked_set);

          if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): session (process group) leader\n",
                                          date,appl_name,appl_pid,appl_host,appl_owner);
             (void)fflush(stderr);
          }
          appl_pg_leader = TRUE;
       }
    }
    else
       (void)pups_sighandle(SIGTERM,"pups_exit_handler",(void *)pups_exit_handler, (sigset_t *)&blocked_set);


    (void)pups_sighandle(SIGINT,  "pups_exit_handler",(void *)pups_exit_handler, (sigset_t *)&blocked_set);
    (void)pups_sighandle(SIGQUIT, "pups_exit_handler",(void *)pups_exit_handler, (sigset_t *)&blocked_set);
    (void)pups_sighandle(SIGPIPE, "default",          SIG_DFL,                   (sigset_t *)NULL);
    (void)pups_sighandle(SIGALIVE,"ignore",           SIG_IGN,                   (sigset_t *)NULL);

    #ifdef CRIU_SUPPORT
    (void)pups_sighandle(SIGCHECK,  "ignore",SIG_IGN, (sigset_t *)NULL);
    (void)pups_sighandle(SIGRESTART,"ignore",SIG_IGN, (sigset_t *)NULL);
    #endif /* CRIU_SUPPORT */


    /*------------------------*/
    /* Set working directory. */
    /*------------------------*/

    if((ptr = pups_locate(&init,"cwd",argc,args,0)) != NOT_FOUND)
    {  char cwd[SSIZE]     = "",
            cwdpath[SSIZE] = "",
            relpath[SSIZE] = "";

       if(strccpy(cwd,pups_str_dec(&ptr,argc,args)) != (char *)NULL)
       {

          /*----------------------------------*/
          /* Check to see if path is absolute */
          /*----------------------------------*/

          if(cwd[0] == '/')
             (void)strlcpy(cwdpath,cwd,SSIZE);
          else if(cwd[0] == '.')
          {  if(cwd[1] == '.')
                pups_error("[pups_std_init] cannot have \"..\" in cwd path"); 

             (void)getcwd(relpath,SSIZE);
             (void)snprintf(cwdpath,SSIZE,"%s/%s",relpath,&cwd[2]);
          }
          else
          {  (void)getcwd(relpath,SSIZE);
             (void)snprintf(cwdpath,SSIZE,"%s/%s",relpath,cwd);
          } 

          if(chdir(cwdpath) != (-1))
          {  (void)strlcpy(appl_cwd,cwdpath,SSIZE);

             if(appl_verbose == TRUE)
             {  (void)strdate(date);
                (void)fprintf(stderr,"%s %s (%d@%s:%s): set current working directory to \"%s\"\n",
                                       date,appl_name,appl_pid,appl_owner,appl_host,appl_cwd);
                (void)fflush(stderr);
             }
          }
          else
          {  (void)getcwd(appl_cwd,SSIZE);
             if(appl_verbose == TRUE)
             {  (void)strdate(date);
                (void)fprintf(stderr,"%s %s (%d@%s:%s): current working directory is \"%s\" (could not change it to \"%s\")\n",
                                                                     date,appl_name,appl_pid,appl_owner,appl_host,appl_cwd,cwd);
                (void)fflush(stderr);
             }    
          }
       }
    }
    else
    {  (void)getcwd(appl_cwd,SSIZE);

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): current working directory is \"%s\" (default)\n",
                                             date,appl_name,appl_pid,appl_owner,appl_host,appl_cwd);
          (void)fflush(stderr);
       }    
    }


    /*-------------------------------------------------------------------------*/
    /* Connect stdio to user specified files (or FIFO's). This permits PUPS    */
    /* compliant routines to use redirected FIFO's for I/O (very useful of the */
    /* application is an active process object, or is part of a dynamically    */
    /* reconfigurable pipeline). Note standard UNIX shells sh,csh etc are      */
    /* not clever enough to redirect to FIFO's or sockets.                     */
    /*-------------------------------------------------------------------------*/
    /*----------------------------*/
    /* Redirect stdin (from file) */
    /*----------------------------*/

    if((ptr = pups_locate(&init,"in",argc,args,0)) != NOT_FOUND)
       in_state = DEAD;
    else
    {  if((ptr = pups_locate(&init,"inalive",argc,args,0)) != NOT_FOUND)
          in_state = LIVE;
    }


    if(in_state == LIVE || in_state == DEAD)
    {   int32_t in_des,
                f_index;

       char     in_name[SSIZE] = "";
       _BOOLEAN file_creator = FALSE;

       if(strccpy(in_name,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          pups_error("[pups_std_init] expecting file name for stdin redirection");

       if(access(in_name,F_OK) == (-1)) 
       {  file_creator = TRUE; 
          if(pups_creat(in_name,0644) == (-1))
             pups_error("[pups_std_init] failed to creat file (for stdin redirection)");
       }
  
       (void)pups_close(0); 
       in_des  = pups_open(in_name,2,LIVE);
       f_index = pups_get_ftab_index(in_des); 

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_lock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       (void)snprintf(ftab[f_index].fname,SSIZE,"%s",in_name);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       if(file_creator == TRUE) 
       {  pups_creator(in_des); 
  
          if(in_state == LIVE) 
             pups_fd_alive(in_des,"default_fd_homeostat: stdin",&pups_default_fd_homeostat);
       }

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): stdin (%d) redirected to %s\n",
                     date,appl_name,appl_pid,appl_host,appl_owner,in_des,in_name);
          (void)fflush(stderr);
       }
    }


    /*----------------------------*/
    /* Redirect stdin (from FIFO) */
    /*----------------------------*/

    if((ptr = pups_locate(&init,"pin",argc,args,0)) != NOT_FOUND)
       pin_state = DEAD; 
    else 
    {  if((ptr = pups_locate(&init,"pinalive",argc,args,0)) != NOT_FOUND) 
          pin_state = LIVE; 
    } 
 

    /*----------------------------*/
    /* Redirect stdin (from FIFO) */
    /*----------------------------*/

    if(pin_state == LIVE || pin_state == DEAD)
    {   int32_t in_des,
                f_index; 
 
       char     in_name[SSIZE] = "";
       _BOOLEAN fifo_creator   = FALSE;
 
       if(strccpy(in_name,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          pups_error("[pups_std_init] expecting FIFO name for stdin redirection");
 
       if(access(in_name,F_OK) == (-1))
       {  fifo_creator = TRUE;
          if(mkfifo(in_name,0600) == (-1))
             pups_error("[pups_std_init] failed to create FIFO (for stdin redirection)");
       }
 
       (void)pups_close(0);
       in_des = pups_open(in_name,2,LIVE);
 
       f_index = pups_get_ftab_index(in_des);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_lock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       (void)snprintf(ftab[f_index].fname,SSIZE,"%s",in_name);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       if(fifo_creator == TRUE)
       {  pups_creator(in_des);

          if(pin_state == LIVE) 
             pups_fd_alive(in_des,"default_fd_homeostat: stdin",&pups_default_fd_homeostat);
       }
 
       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): stdin (%d) redirected to %s\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,in_des,in_name);
          (void)fflush(stderr);
       }
    }


    /*---------------------------*/
    /* Redirect stdout (to file) */
    /*---------------------------*/

    if((ptr = pups_locate(&init,"out",argc,args,0)) != NOT_FOUND)
       out_state = DEAD; 
    else 
    {  if((ptr = pups_locate(&init,"outalive",argc,args,0)) != NOT_FOUND) 
          out_state = LIVE; 
    } 
 
    if(out_state == LIVE || out_state == DEAD)
    {   int32_t out_des,
                f_index;

       char out_name[SSIZE]  = "";

       _BOOLEAN file_creator = FALSE;
 
       if(strccpy(out_name,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          pups_error("[pups_std_init] expecting file name for stdout redirection");

       if(access(out_name,F_OK | W_OK) == (-1))
       {  file_creator = TRUE;
          if(pups_creat(out_name,0644) == (-1))
             pups_error("[pups_std_init] failed to create file (for stdout redirection)");
       }
 
       (void)pups_close(1);
       out_des = pups_open(out_name,2,LIVE);
       f_index = pups_get_ftab_index(out_des);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_lock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       (void)snprintf(ftab[f_index].fname,SSIZE,"%s",out_name);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       if(file_creator == TRUE)
       {  pups_creator(out_des);
 
          if(out_state == LIVE) 
             pups_fd_alive(out_des,"default_fd_homeostat: stdout",&pups_default_fd_homeostat);
       }

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): stdout (%d) redirected to %s\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,out_des,out_name);
          (void)fflush(stderr);
       }   
    }


    /*---------------------------*/
    /* Redirect stdout (to FIFO) */
    /*---------------------------*/

    if((ptr = pups_locate(&init,"pout",argc,args,0)) != NOT_FOUND)
       pout_state = DEAD; 
    else 
    {  if((ptr = pups_locate(&init,"poutalive",argc,args,0)) != NOT_FOUND) 
          pout_state = LIVE; 
    } 
 
    if(pout_state == LIVE || pout_state == DEAD)
    {   int32_t out_des,
                f_index;
 
       char     out_name[SSIZE] = "";
       _BOOLEAN fifo_creator    = FALSE;
 
       if(strccpy(out_name,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          pups_error("[pups_std_init] expecting FIFO name for stdout redirection");
 
       if(access(out_name,F_OK | W_OK) == (-1))
       {  fifo_creator = TRUE;
          if(mkfifo(out_name,0600) == (-1))
             pups_error("[pups_std_init] failed to create FIFO (for stdout redirection)");
       }
 
       (void)pups_close(1);
       out_des = pups_open(out_name,2,LIVE);
 
       f_index = pups_get_ftab_index(out_des);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_lock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       (void)snprintf(ftab[f_index].fname,SSIZE,"%s",out_name);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       if(fifo_creator == TRUE)
       {  pups_creator(out_des);

          if(pout_state == LIVE) 
             pups_fd_alive(out_des,"default_fd_homeostat: stdout",&pups_default_fd_homeostat);
       }
 
       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): stdout (%d) redirected to %s\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,out_des,out_name);
          (void)fflush(stderr);
       }
    }


    /*---------------------------*/ 
    /* Redirect stderr (to file) */
    /*---------------------------*/ 

    if((ptr = pups_locate(&init,"err",argc,args,0)) != NOT_FOUND)
       err_state = DEAD; 
    else 
    {  if((ptr = pups_locate(&init,"erralive",argc,args,0)) != NOT_FOUND) 
          err_state = LIVE; 
    } 
 
    if(err_state == LIVE || err_state == DEAD)
    {   int32_t err_des,
                f_index;

       char err_name[SSIZE]  = "";

       _BOOLEAN file_creator = FALSE;
 
       if(strccpy(err_name,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          pups_error("[pups_std_init] expecting file name for stderr redirection");
 
       if(access(err_name,F_OK) == (-1))
       {  file_creator = TRUE;
          if(pups_creat(err_name,0644) == (-1))
             pups_error("[pups_std_init] failed to create file (for stderr redirection)");
       }

       (void)pups_close(2);
       err_des = pups_open(err_name,2,LIVE);
       f_index = pups_get_ftab_index(err_des);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_lock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       (void)snprintf(ftab[f_index].fname,SSIZE,"%s",err_name);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       if(file_creator == TRUE)
       {  pups_creator(err_des);

          if(err_state == LIVE)
             pups_fd_alive(err_des,"default_fd_homeostat: stderr",&pups_default_fd_homeostat);
       }

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): stderr (%d) redirected to %s\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,err_des,err_name);
          (void)fflush(stderr);
       }   
    }


    /*---------------------------*/
    /* Redirect stderr (to FIFO) */
    /*---------------------------*/

    if((ptr = pups_locate(&init,"perr",argc,args,0)) != NOT_FOUND)
       perr_state = DEAD; 
    else 
    {  if((ptr = pups_locate(&init,"perralive",argc,args,0)) != NOT_FOUND) 
          perr_state = LIVE; 

    } 
 
    if(perr_state == LIVE || perr_state == DEAD)
    {   int32_t err_des,
                f_index;

       char     err_name[SSIZE] = "";
       _BOOLEAN fifo_creator    = FALSE;

       if(strccpy(err_name,pups_str_dec(&ptr,argc,args)) == (char *)NULL)
          pups_error("[std_init] expecting FIFO name for stderr redirection");

       if(access(err_name,F_OK) == (-1))
       {  fifo_creator = TRUE;
          if(mkfifo(err_name,0600) == (-1))
             pups_error("[pups_std_init] failed to created stderr FIFO");
       }
 
       (void)pups_close(2); 
       err_des = pups_open(err_name,2,LIVE);
 
       f_index = pups_get_ftab_index(err_des);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_lock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       (void)snprintf(ftab[f_index].fname,SSIZE,"%s",err_name);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       if(fifo_creator == TRUE)
       {  pups_creator(err_des);

          if(perr_state == LIVE)
             pups_fd_alive(err_des,"default_fd_homeostat: stderr",&pups_default_fd_homeostat);
       }
 
       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): stderr (%d) redirected to %s\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,err_des,err_name);
          (void)fflush(stderr);
       }   
    }
 

    /*--------------------------*/
    /* Generate command string. */
    /*--------------------------*/

    (void)strlcpy(appl_cmd_str,appl_name,SSIZE);
    (void)strlcat(appl_cmd_str," ",      SSIZE);

    for(i=0; i<(*argc) - 1; ++i)
    {  (void)strlcat(appl_cmd_str,args[i],SSIZE);
       (void)strlcat(appl_cmd_str," ",SSIZE);
    }


    /*-----------------------------------*/
    /* Set the niceness of this process. */
    /*-----------------------------------*/

    if((ptr = pups_locate(&init,"nice",argc,args,0)) != NOT_FOUND)
    {  if((appl_nice_lvl = pups_i_dec(&ptr,argc,args)) == INVALID_ARG)
          pups_error("[std_init] expecting process niceness parameter");
    }

    (void)nice(appl_nice_lvl);
    pups_set_errno(OK);
}




/*----------------------------------------*/
/* Set memory allocate (extended) options */
/*----------------------------------------*/

_PUBLIC void pups_set_alloc_opt(const int32_t alloc_opt)

{
    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_set_alloc_opt] attempt by non root thread to perform PUPS/P3 global utility operation");


    appl_alloc_opt = alloc_opt;
}




/*--------------------------------*/
/* Extended error checking malloc */
/*--------------------------------*/

_PUBLIC void *pups_malloc(const psize_t size)

{   void *ptr = (void *)NULL;

    if(size == 0)
    {  pups_set_errno(EINVAL);
       return(NULL);
    }

    if(appl_alloc_opt == MALLOC_HOMEOSTATIC)
    {

       /*----------------------------------------*/
       /* Make sure memory is actually allocated */
       /* before returning to caller             */
       /*----------------------------------------*/

       do {    pupshold(ALL_PUPS_SIGS);
               ptr = (void *)malloc(size);
               pupsrelse(ALL_PUPS_SIGS);

               (void)pups_usleep(100);
          } while(ptr == (void *)NULL);
    }
    else
    {  pupshold(ALL_PUPS_SIGS);
       ptr = (void *)malloc(size);
       pupsrelse(ALL_PUPS_SIGS);

       if(ptr == NULL && size != 0)
          pups_error("[pups_malloc] failed to allocate core");
    }

    pups_set_errno(OK);
    return((void *)ptr);
}




/*---------------------------------*/
/* Extended error checking realloc */
/*---------------------------------*/

_PUBLIC void *pups_realloc(void *ptr, const psize_t size)

{   pups_set_errno(OK);
    
    if(ptr == (void *)NULL && size != 0)
    {  ptr = (void *)pups_malloc(size);
       return(ptr);
    }
    else if(ptr != NULL && size == 0)
       return(pups_free((void *)ptr));
    else if(ptr != NULL && size > 0)
    {  if(appl_alloc_opt == MALLOC_HOMEOSTATIC)
       {  

          /*----------------------------------------*/
          /* Make sure memory is actually allocated */
          /* before returning to caller             */
          /*----------------------------------------*/

          do {    pupshold(ALL_PUPS_SIGS);
                  ptr = (void *)realloc(ptr,size);
                  pupsrelse(ALL_PUPS_SIGS);

                  (void)pups_usleep(100);
             } while(ptr == (void *)NULL);
       }
       else
       {  pupshold(ALL_PUPS_SIGS);
          ptr = (void *)realloc(ptr,size);
          pupsrelse(ALL_PUPS_SIGS);

          if(ptr == NULL)
             pups_error("[pups_realloc] failed to allocate core");
       }

       return((void *)ptr);
    }
}



/*--------------------------------*/
/* Extended error checking calloc */
/*--------------------------------*/

_PUBLIC void *pups_calloc(const pindex_t nel, const psize_t size)

{   void *ptr = (void *)NULL;

    if(nel <= 0 || size <= 0)
    {  pups_set_errno(EINVAL);
       return((void *)NULL);
    }
    else
       pups_set_errno(OK);

    if(appl_alloc_opt == MALLOC_HOMEOSTATIC)
    {

       /*----------------------------------------*/
       /* Make sure memory is actually allocated */
       /* before returning to caller             */
       /*----------------------------------------*/

       do {    pupshold(ALL_PUPS_SIGS);
               ptr = (void *)calloc(nel,size);
               pupsrelse(ALL_PUPS_SIGS);

               (void)pups_usleep(100);
          } while(ptr == (void *)NULL);
    }
    else
    {  pupshold(ALL_PUPS_SIGS);
       ptr = (void *)calloc(nel,size);
       pupsrelse(ALL_PUPS_SIGS);

       if(ptr == NULL)
          pups_error("[pups_calloc] failed to allocate core");
    }

    return((void *)ptr);
}



/*------------------------------*/
/* Extended error checking free */
/*------------------------------*/

_PUBLIC void *pups_free(const void *ptr)

{   if(ptr != (const void *)NULL)
    {  pupshold(ALL_PUPS_SIGS);
       (void)free((void *)ptr);
       pupsrelse(ALL_PUPS_SIGS);

       ptr = NULL;
    }

    pups_set_errno(OK);
    return((void *)ptr);
}




/*------------------------------*/
/* Extended error checking read */
/*------------------------------*/

_PUBLIC int32_t pups_read(const des_t f_id, void *buf, const psize_t n_bytes)

{   
    if(buf == (const _BYTE *)NULL || n_bytes <= 0)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);

    if((read(f_id,buf,n_bytes)) != n_bytes)
       pups_error("[xread] failed to read all of data from stream");

    return(n_bytes);
}



/*------------------------------*/
/* Extended eror checking write */
/*------------------------------*/

_PUBLIC int32_t pups_write(const des_t fd, const void *buf, const psize_t n_bytes)

{   psize_t written       = 0L,
            total_written = 0L;

    _BYTE *bbuf = (_BYTE *)buf;

    if(buf == (const _BYTE *)NULL || n_bytes <= 0)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);


    /*----------------------*/
    /* Write chunks of data */
    /*----------------------*/

    while(total_written < n_bytes)
    {    written = write(fd,&bbuf[total_written],(n_bytes - total_written));

         if(written == (-1) && pups_get_errno() != EINTR)
            return(-1);
         else
            total_written += written;
    }
       
    return(total_written);
}




/*------------------------*/
/* Execute command string */ 
/*------------------------*/

_PUBLIC int32_t pups_execls(const char *command)

{    int32_t ret,
             j,
             i = 0;

    char     *cmd_list[ARGS] = { [0 ... ARGS-1] = (char *)NULL };  // List of command arguments 
    _BOOLEAN looper;

    if(command == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*--------------------------*/
    /* Could expand ^ into xrsh */
    /*--------------------------*/

    if(command == (char *)NULL || strin(command,"^") == TRUE)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*---------------------------------------------------------------*/
    /* If we are part of a command pipeline start a shell to run the */
    /* pipeline.                                                     */
    /*---------------------------------------------------------------*/

    if(strin(command,"|") == TRUE)
    {  execlp(shell,shell,"-c",command,(char *)0);

       pups_set_errno(ENOEXEC);
       return(-1);
    }


    /*-----------------------*/
    /* Build command vector. */
    /*-----------------------*/

    (void)strext(' ',(char *)NULL,(char *)NULL);
    do {   cmd_list[i] = pups_malloc(SSIZE);
           looper = strext(' ',cmd_list[i],command);

           if(looper != FALSE)
           {

              #ifdef UTILIB_DEBUG
              (void)fprintf(stderr,"UTILIB EXECA[%d]: \"%s\"\n",i,cmd_list[i]);
              (void)fflush(stderr);
              #endif /* UTILIB_DEBUG */

              ++i;
           }
           else
              (void)pups_free((void *)cmd_list[i]);

       } while(looper == TRUE);
       cmd_list[i] = (char *)0;


    /*-----------------------------------------------------------*/
    /* Execute command: if execv is successful this won't return */
    /*-----------------------------------------------------------*/

    ret = execvp(cmd_list[0],cmd_list);

    for(j=0; j<i; ++j)
        (void)pups_free(cmd_list[j]); 

    pups_set_errno(ENOEXEC);;
    return(ret);
}




/*------------------------------------------------------------------*/
/*    Set  discretionary lock. This routine assumes that            */
/*    making a link to a file is atomic. This means                 */
/*    that only one process can make a link to a file at any        */
/*    time.                                                         */
/*                                                                  */
/*    This routine blocks the caller until lock is acquired. If the */
/*    lock cannot be acquired after a reasonable number of attempts */
/*    the calling process is exited                                 */
/*------------------------------------------------------------------*/


/*------------------------------*/
/* Lock names,types` and counts */
/*------------------------------*/

_PRIVATE int32_t         lock_cnt      [PUPS_MAX_LOCKS]        = { [0 ... PUPS_MAX_LOCKS-1] = 0 };
_PRIVATE char            lock_name_list[PUPS_MAX_LOCKS][SSIZE] = { [0 ... PUPS_MAX_LOCKS-1] = { "notset"}};
_PRIVATE char            lock_type_list[PUPS_MAX_LOCKS][SSIZE] = { [0 ... PUPS_MAX_LOCKS-1] = { "notset"}};
_PRIVATE _BOOLEAN        locks_active                          = FALSE;



/*---------------------------------*/
/* Get next free (link) lock index */
/*---------------------------------*/

_PRIVATE int32_t get_lock_index(char *name)

{   uint32_t i;
             int32_t  free_index = 0;


    /*---------------------------------------*/
    /* Initialise lock list if this is first */
    /* call to function                      */
    /*---------------------------------------*/

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(locks_active == FALSE)
    {  for(i=0; i<PUPS_MAX_LOCKS; ++i)
          (void)strlcpy(lock_name_list[i],"notset",SSIZE);
       locks_active = TRUE;
    }


    /*--------------------------------------------*/
    /* search for object (to see if it is locked) */
    /*--------------------------------------------*/

    for(i=0; i<PUPS_MAX_LOCKS; ++i)
    {  if(strcmp(lock_name_list[i],name) == 0)
       {

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&lock_mutex);
          #endif /* PTHREAD_SUPPORT */

          return((int32_t)i);
       }
       else if(strcmp(lock_name_list[i],"notset") != 0)
          ++free_index;
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    return(free_index);
} 




/*------------------------------------*/
/* Show (link) locks currently in use */
/*------------------------------------*/

_PUBLIC int32_t pups_show_link_file_locks(const FILE *stream)

{   uint32_t i,
             locks = 0;

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_link_file_locks] attempt by non root thread to perform PUPS/P3 global utility operation");

    (void)fprintf(stream,"\nLink lock table\n");
    (void)fprintf(stream,"===============\n\n");

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(locks_active == TRUE) 
    {  for(i=0; i<PUPS_MAX_LOCKS; ++i)
       {  if(strcmp(lock_name_list[i],"notset") != 0)
          {  

             /*----------------*/
             /* Read lock held */
             /*----------------*/

             if(strcmp(lock_type_list[i],"write") == 0)
                (void)fprintf(stream,"    %04d [slot %04d] read lock held on file \"%-48s\" (count is %04d)\n",
                                                                         locks,i,lock_name_list[i],lock_cnt[i]);


             /*-----------------*/
             /* Write lock held */
             /*-----------------*/

             else
                (void)fprintf(stream,"    %04d [slot %04d] write lock held on file \"%-48s\" (count is %04d)\n",
                                                                          locks,i,lock_name_list[i],lock_cnt[i]);

             (void)fflush(stream);

             ++locks;
          }
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(locks == 0)
       (void)fprintf(stream,"\n\n    No locks held (%04d concurrent locks available)\n\n",PUPS_MAX_LOCKS);
    else if(locks == 1)
       (void)fprintf(stream,"\n\n    %04d lock held (%04d concurrent locks available)\n\n", locks,PUPS_MAX_LOCKS - 1);
    else
       (void)fprintf(stream,"    %04d locks held (%04d concurrent locks available)\n\n",locks,PUPS_MAX_LOCKS - locks);


    return(0);
}




/*----------------------------------*/
/* Set read or write link file lock */
/*----------------------------------*/

_PUBLIC int32_t pups_get_rdwr_link_file_lock(const int32_t lock_type, const int32_t max_trys, const char *file_name)

{    int32_t ret,
             lock_index,
             trys = 0;

    pid_t owner_pid;

    char  lock_name[SSIZE]      = "",
          rd_lock_name[SSIZE]   = "",
          lock_id[SSIZE]        = "",
          strdum[SSIZE]         = "",

          #ifdef SSH_SUPPORT
          owner_host[SSIZE]     = "",
          owner_ssh_port[SSIZE] = "",
          #endif /* SSH_SUPPORT */

          lock_owner[SSIZE]     = "",
          lid_name[SSIZE]       = "";

    struct stat buf;

    DIR           *dirp      = (DIR *)NULL;
    struct dirent *next_item = (struct dirent *)NULL;

    if(max_trys   == 0                                                      ||
       (lock_type != EXLOCK && lock_type != RDLOCK && lock_type != WRLOCK)  ||
       file_name  == (const char *)NULL                                      )
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);


    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*-----------------------------------------------------------*/
    /* If we are on a read only filesystem then we don't need to */
    /* lock files but tell caller that it has attempted to set a */
    /* write lock on a read only file system via global errno    */
    /*-----------------------------------------------------------*/

    if(pups_is_on_isofs(file_name) == TRUE)
    {  

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&lock_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(EROFS);
       return(0);
    }

    if(file_name == (char *)NULL || access(file_name,F_OK | R_OK | W_OK) == (-1))
    {  if(access(file_name,F_OK | R_OK | W_OK) == (-1) && appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): file \"%s\" does not exist (cannot lock it!))\n",
                                            date,appl_name,appl_pid,appl_host,appl_owner,file_name);
          (void)fflush(stderr);
       }

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&lock_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(EBADF);
       return(-1);
    }

    if((lock_index = get_lock_index(file_name)) == (-1))
       pups_error("[pups_get_rdwr_link_file_lock] too many locks");
   
    ++lock_cnt[lock_index];

    if(lock_cnt[lock_index] > 1)
    {  if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): file \"%s\" lock count incremented to %d\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,file_name,lock_cnt[lock_index]);
          (void)fflush(stderr);
       }

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&lock_mutex);
       #endif /* PTHREAD_SUPPORT */

       return(0);
    }


    /*---------------------*/
    /* Try to acquire lock */
    /*---------------------*/

    (void)snprintf(lock_name,SSIZE,   "%s.lock",  file_name);
    (void)snprintf(rd_lock_name,SSIZE,"%s.rdlock",file_name);
    (void)snprintf(lid_name,SSIZE,    "%s.lid",   file_name);


    /*-----------------------------------------------------------*/
    /* Check to see if the process which owns the lock is still  */
    /* alive and well. Note there are conditions (when owner_pid */
    /* has been re-used when we may get a deadlock here)         */
    /*-----------------------------------------------------------*/

    dirp = opendir(".");
    while((next_item = readdir(dirp)) != (struct dirent *)NULL)
    {

        /*------------------------------------------------*/
        /* Is this the lid which corresponds to our lock? */ 
        /*------------------------------------------------*/

        if(strncmp(next_item->d_name,lid_name,strlen(lid_name)) == 0)
        {

           /*--------------------------------------------*/
           /* Strip file i.d. information from lock i.d. */
           /*--------------------------------------------*/

           (void)strccpy(lock_owner,next_item->d_name);
           (void)mchrep(' ',".",lock_owner);


           #ifdef SSH_SUPPORT
                                              /*---------------------------------------------------*/
                                              /* name  lid          host       ssh_port        pid */
                                              /*---------------------------------------------------*/

           if(sscanf(lock_owner,"%s%s%s%s%s%d",strdum,strdum,strdum,owner_host,owner_ssh_port,&owner_pid) == 6)
           #else
                                              /*------------------------------*/
                                              /* name  lid           pid      */
                                              /*------------------------------*/

           if(sscanf(lock_owner,"%s%s%s%s%s%d",strdum,strdum,strdum,&owner_pid) == 4)
           #endif /* SSH_SUPPORT */
           {

              /*----------------------------------------*/
              /* Is this lock owned by a local process? */
              /*----------------------------------------*/

              if(strncmp(appl_host,owner_host,strlen(owner_host)) == 0)
              {  if(kill(owner_pid,SIGALIVE) == (-1))
                 {

                    /*----------------------*/
                    /* This is a stale I.D. */
                    /*----------------------*/


                                                           /*------*/
                    (void)pups_unlink(next_item->d_name);  /* Lock */
                    (void)pups_unlink(lock_name);          /* Lid  */
                                                           /*------*/

                    if(appl_verbose == TRUE)
                    {  (void)strdate(date);
                       (void)fprintf(stderr,"%s %s (%d@%s:%s): stale (local) lock id \"%s\" removed (owner has died)\n",
                                                         date,appl_name,appl_pid,appl_host,appl_owner,next_item->d_name);
                       (void)fflush(stderr);
                    }
                 }
              }


              #ifdef SSH_SUPPORT
              /*----------------*/
              /* Remote process */
              /*----------------*/

              else
              {  char pidname[SSIZE] = "";

                 (void)snprintf(pidname,SSIZE,"%d",owner_pid);
                 if(pups_rkill(owner_host,owner_ssh_port,appl_owner,pidname,SIGTERM) == (-1))
                 {  

                    /*----------------------*/
                    /* This is a stale I.D. */
                    /*----------------------*/


                                                           /*------*/
                    (void)pups_unlink(next_item->d_name);  /* Lock */
                    (void)pups_unlink(lock_name);          /* Lid  */
                                                           /*------*/

                    if(appl_verbose == TRUE)
                    {  (void)strdate(date);
                       (void)fprintf(stderr,"%s %s (%d@%s:%s): stale lock id \"%s\" removed (owner has died)\n",
                                                 date,appl_name,appl_pid,appl_host,appl_owner,next_item->d_name);
                       (void)fflush(stderr);
                    }
                 }
              }
              #endif /* SSH_SUPPORT */

           }
       }
    }
    (void)closedir(dirp);


    /*--------------------------------------------------------*/
    /* A lock which only has a link count of two is stale and */
    /* should be removed                                      */
    /*--------------------------------------------------------*/

    (void)stat(lock_name,&buf);
    if(access(lock_name,F_OK | R_OK | W_OK) != (-1) && buf.st_nlink < 3)
    {  (void)unlink(lock_name);
       (void)unlink(rd_lock_name);
       (void)unlink(lock_id);

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): file \"%s\" stale lock removed\n",
                             date,appl_name,appl_pid,appl_host,appl_owner,file_name);
          (void)fflush(stderr);
       }
    }

    if(max_trys != TRYLOCK)
    {  if(lock_type == RDLOCK) 
       {  while((ret = link(file_name,lock_name)) == (-1))
          {    if(max_trys != WAIT_FOREVER)
	          ++trys;

               if(trys > max_trys && max_trys != WAIT_FOREVER)
               {  if(appl_verbose == TRUE)
                  {  (void)strdate(date);
                     (void)fprintf(stderr,"%s %s (%d@%s:%s): file \"%s\" failed to acquire lock (in %d attempts)\n",
                                                    date,appl_name,appl_pid,appl_host,appl_owner,file_name,max_trys);
                     (void)fflush(stderr);
                  }

                  #ifdef PTHREAD_SUPPORT
                  (void)pthread_mutex_lock(&lock_mutex);
                  #endif /* PTHREAD_SUPPORT */

                  --lock_cnt[lock_index];

                  #ifdef PTHREAD_SUPPORT
                  (void)pthread_mutex_unlock(&lock_mutex);
                  #endif /* PTHREAD_SUPPORT */

                  pups_set_errno(EBUSY);
                  return(-1);
               }

               (void)pups_usleep(100);
          }
       }
       else       
       {  while(access(rd_lock_name,F_OK | R_OK | W_OK) != (-1) || link(file_name,lock_name) == (-1))
          {    if(max_trys != WAIT_FOREVER)
                  ++trys;

               if(trys > max_trys && max_trys != WAIT_FOREVER)
               {  if(appl_verbose == TRUE)
                  {  (void)strdate(date);
                     (void)fprintf(stderr,"%s %s (%d@%s:%s): file \"%s\" failed to acquire lock (in %d attempts)\n",
                                                    date,appl_name,appl_pid,appl_host,appl_owner,file_name,max_trys);
                     (void)fflush(stderr);
                  }

                  #ifdef PTHREAD_SUPPORT
                  (void)pthread_mutex_lock(&lock_mutex);
                  #endif /* PTHREAD_SUPPORT */

                  --lock_cnt[lock_index];

                  #ifdef PTHREAD_SUPPORT
                  (void)pthread_mutex_unlock(&lock_mutex);
                  #endif /* PTHREAD_SUPPORT */

                  pups_set_errno(EBUSY);
                  return(-1);
               }

               (void)pups_usleep(100);
          }
       }
    }


    /*------------------*/
    /* Protect the lock */
    /*------------------*/

    else
    {


       if(lock_type == RDLOCK)
       {  if(link(file_name,rd_lock_name) == (-1)) 
          {  

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_lock(&lock_mutex);
             #endif /* PTHREAD_SUPPORT */

             --lock_cnt[lock_index];

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&lock_mutex);
             #endif /* PTHREAD_SUPPORT */

             pups_set_errno(EBUSY);
	     return(-1);
          }
       }


       /*------------------------------------------------------------------------*/
       /* We need to check if we still have readers before releasing a read lock */ 
       /*------------------------------------------------------------------------*/

       else
       {  

          if(access(rd_lock_name,F_OK | R_OK | W_OK) != (-1) || link(file_name,lock_name) == (-1))
          {  
             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_lock(&lock_mutex);
             #endif /* PTHREAD_SUPPORT */

             --lock_cnt[lock_index];

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&lock_mutex);
             #endif /* PTHREAD_SUPPORT */

             pups_set_errno(EBUSY);
	     return(-1);
          }
       }
    }

    if(appl_verbose == TRUE && test_mode == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): file \"%s\" lock count incremented to %d [lock acquired]\n",
                               date,appl_name,appl_pid,appl_host,appl_owner,file_name,lock_cnt[lock_index]);
       (void)fflush(stderr);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*---------------------------------------------*/
    /* New object -- put it in the lock name table */ 
    /*---------------------------------------------*/

    (void)strlcpy(lock_name_list[lock_index],file_name,SSIZE);


    /*-----------*/
    /* Read lock */
    /*-----------*/

    if(lock_type == RDLOCK)
       (void)strlcpy(lock_type_list[lock_index],"read",SSIZE);


    /*------------*/
    /* Write lock */
    /*------------*/

    else
       (void)strlcpy(lock_type_list[lock_index],"write",SSIZE);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    #ifdef SSH_SUPPORT
    (void)snprintf(lock_id,SSIZE,"%s.lid.%s.%s.%s.%d.tmp",file_name,appl_name,appl_host,ssh_remote_port,appl_pid);
    #else
    (void)snprintf(lock_id,SSIZE,"%s.lid.%s.%d.tmp",file_name,appl_name,appl_pid);
    #endif /* SSH_SUPPORT */


    /*--------------*/
    /* Acquire lock */
    /*--------------*/

    (void)link(file_name,lock_id);
    return(0);
}




/*----------------------------------------*/
/* Set a write (exclusive) link file lock */
/*----------------------------------------*/

_PUBLIC int32_t pups_get_link_file_lock(const uint32_t max_trys, const char *file_name)

{    int32_t ret;

    if(max_trys == 0 || file_name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    ret = pups_get_rdwr_link_file_lock(WRLOCK,max_trys,file_name);
    return(ret);
}




/*------------------------*/
/* Release link file lock */
/*------------------------*/

_PUBLIC int32_t pups_release_link_file_lock(const char *file_name)

{   char lock_name[SSIZE] = "",
         lock_id[SSIZE]   = "";

     int32_t lock_index;

    if(file_name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);


    /*-------------------------------------------------------------*/
    /* If we are on a read only filesystem then we don't need to   */
    /* lock files but tell caller that we attempted to set a write */
    /* lock on a read only file system via global errno            */
    /*-------------------------------------------------------------*/

    if(pups_is_on_isofs(file_name) == TRUE)
    {  pups_set_errno(EROFS);
       return(0);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);

    #ifdef SSH_SUPPORT
    (void)snprintf(lock_id,SSIZE,"%s.lid.%s.%s.%s.%d.tmp",file_name,appl_name,appl_host,ssh_remote_port,appl_pid);
    #else
    (void)snprintf(lock_id,SSIZE,"%s.lid.%s.%s.%d.tmp",file_name,appl_name,appl_host,appl_pid);
    #endif /* SSH_SUPPORT */

    if(access(lock_id,F_OK | R_OK | W_OK) == (-1))
    {  if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): not currently holding lock on \"%s\" (so cannot release it!)\n",
                                                           date,appl_name,appl_pid,appl_host,appl_owner,file_name);
          (void)fflush(stderr);
       } 
       
       pups_set_errno(EBADF);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&lock_mutex);
       #endif /* PTHREAD_SUPPORT */

       return(-1);
    }

    if((lock_index = get_lock_index(file_name)) == (-1))
       pups_error("[pups_release_link)file_lock] lock count inconsistency");

    --lock_cnt[lock_index];

    if(lock_cnt[lock_index] > 0)
    {  if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): file \"%s\" lock count decremented to %d\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,file_name,lock_cnt[lock_index]);
          (void)fflush(stderr);
       } 

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&lock_mutex);
       #endif /* PTHREAD_SUPPORT */

       return(0);
    }


    /*-------------------------*/
    /* Free lock table entries */
    /*-------------------------*/

    (void)strlcpy(lock_name_list[lock_index],"notset",SSIZE);
    (void)strlcpy(lock_type_list[lock_index],"notset",SSIZE);


    /*-------------------*/
    /* Remove write lock */
    /*-------------------*/

    (void)snprintf(lock_name,SSIZE,"%s.lock",file_name);
    (void)unlink(lock_name);


    /*------------------*/
    /* Remove read lock */
    /*------------------*/

    (void)snprintf(lock_name,SSIZE,"%s.rdlock",file_name);
    (void)unlink(lock_name);


    /*------------------------*/
    /* Remove lock i.d. (LID) */
    /*------------------------*/

    (void)unlink(lock_id);

    if(appl_verbose == TRUE && test_mode == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): file \"%s\" lock count decremented to %d [lock released]\n",
                               date,appl_name,appl_pid,appl_host,appl_owner,file_name,lock_cnt[lock_index]);
       (void)fflush(stderr);
    } 

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    return(0);
}




/*-----------------------------*/
/* Release all link file locks */
/*-----------------------------*/

_PUBLIC int32_t pups_release_all_link_file_locks(void)

{   uint32_t i;
    int32_t  removed         = 0;

    char next_lock_id[SSIZE] = "",
         next_lock[SSIZE]    = "";

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*--------------------------------*/
    /* Remove all locks in lock table */
    /*--------------------------------*/

    for(i=0; i<PUPS_MAX_LOCKS; ++i)
    {  if(strcmp(lock_name_list[i],"notset") != 0)
       {  

          /*-----------------------------*/
          /* Remove next lock i.d. (LID) */
          /*-----------------------------*/

          #ifdef SSH_SUPPORT
          (void)snprintf(next_lock_id,SSIZE,"%s.lid.%s.%s.%s.%d.tmp",lock_name_list[i],appl_name,appl_host,ssh_remote_port,appl_pid);
          #else
          (void)snprintf(next_lock_id,SSIZE,"%s.lid.%s.%s.%d.tmp",   lock_name_list[i],appl_name,appl_host,appl_pid);
          #endif /* SSH_SUPPORT */


          /*------------------------------------------*/
          /* Error - failed to remove lock i.d. (LID) */
          /*------------------------------------------*/

          if(unlink(next_lock_id) == (-1))
          {
             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_lock(&lock_mutex);
             #endif /* PTHREAD_SUPPORT */

             return(-1);
          }
      

          /*------------------*/
          /* Remove next lock */
          /*------------------*/
 
          (void)snprintf(next_lock,SSIZE,"%s.lock",lock_name_list[i]);
          if(unlink(next_lock) != (-1))
             ++removed;


          /*-------------------------------*/
          /* Error - failed to remove lock */
          /*-------------------------------*/

          else

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&lock_mutex);
             #endif /* PTHREAD_SUPPORT */

             return(-1); 


          /*---------------------------*/
          /* Remove lock table entries */
          /*---------------------------*/

          (void)strlcpy(lock_name_list[i],"notset",SSIZE);
          (void)strlcpy(lock_type_list[i],"notset",SSIZE);
       }
    }


    /*-------------------------------------------*/
    /* No locks (should) be active at this point */
    /*-------------------------------------------*/

    locks_active = FALSE;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(removed);
}




/*------------------------------------------------*/
/* Enable obituary for pipestream (child) process */
/*------------------------------------------------*/

_PUBLIC  int32_t pups_pipestream_obituary_enable(const des_t fdes)

{   uint32_t i;
     int32_t f_index;

    f_index = pups_get_ftab_index(fdes);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(ftab[f_index].fifo_pid == (-1) && ftab[f_index].rd_pid == (-1))
    {  

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(EINVAL);
       return(-1);
    }

    for(i=0; i<n_children; ++i)
    {  if(chtab[i].pid == ftab[f_index].fifo_pid || chtab[i].pid == ftab[i].rd_pid)
       {  chtab[i].obituary = TRUE;


          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return(0);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




/*-------------------------------------------------*/ 
/* Disable obituary for pipestream (child) process */
/*-------------------------------------------------*/ 
 
_PUBLIC int32_t pups_pipestream_obituary_disable(const des_t fdes) 
 
{   uint32_t i;
    int32_t  f_index;
 
    f_index = pups_get_ftab_index(fdes);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(ftab[f_index].fifo_pid == (-1) && ftab[f_index].rd_pid == (-1))
    {  

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(EINVAL);
       return(-1);
    }
 
    for(i=0; i<n_children; ++i)
    {  if(chtab[i].pid == ftab[f_index].fifo_pid || chtab[i].pid == ftab[i].rd_pid)
       {  chtab[i].obituary = FALSE;

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return(0);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH); 
    return(-1);
} 




/*-----------------------------------------------*/
/* Extended fork (which records forked children) */
/*-----------------------------------------------*/

_PUBLIC pid_t pups_fork(const _BOOLEAN fork_wait, const _BOOLEAN obituary)

{   uint32_t i;
    pid_t    pid;

    if((fork_wait != FALSE && fork_wait != TRUE) ||
       (obituary  != FALSE && obituary  != TRUE)  )
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&pups_fork_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*-------------------------------------*/
    /* Search for free slot in child table */
    /*-------------------------------------*/

    for(i=0; i<appl_max_child; ++i)
    {  if(chtab[i].pid == (-1))
       {  

          /*---------------------------------------------------*/
          /* If fork_wait is TRUE parent will wait until there */
          /* are sufficient resources to fork its child        */ 
          /*---------------------------------------------------*/

          if(fork_wait == TRUE)
          {  while((pid = fork()) == (-1))
                  (void)pups_usleep(100);
          }
          else
             pid = fork(); 


          /*---------------------*/
          /* Parent side of fork */
          /*---------------------*/

          if(pid > 0)
          {  chtab[i].pid      = pid;
             chtab[i].obituary = obituary;
             ++n_children;

            #ifdef PTHREAD_SUPPORT
            (void)pthread_mutex_unlock(&pups_fork_mutex);
            #endif /* PTHREAD_SUPPORT */

             return(pid);
          }


          /*--------------------*/
          /* Child side of fork */
          /*--------------------*/

          else
             return(0);
       }
    }

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): child table full (max %d entries)\n",
                        date,appl_name,appl_pid,appl_host,appl_owner,appl_max_child);
       (void)fflush(stderr);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&pups_fork_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(EAGAIN);
    return(-1);
}




/*------------------------*/
/* Initialise child table */
/*------------------------*/

_PUBLIC void pups_init_child_table(const uint32_t max_children)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_init_child_table] attempt by non root thread to perform PUPS/P3 global utility operation");

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&chtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    chtab = (chtab_type *)pups_realloc((void *)chtab,max_children*sizeof(chtab_type));
    for(i=0; i<max_children; ++i)
    {  chtab[i].pid      = (-1);
       chtab[i].obituary = FALSE;
       (void)strlcpy(chtab[i].name,"",SSIZE);
    }

    (void)pups_sighandle(SIGCHLD,"child_handler",(void *)&chld_handler, (sigset_t *)NULL);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&chtab_mutex);
    #endif /* PTHREAD_SUPPORT */
}




/*------------------------*/
/* Clear child table slot */
/*------------------------*/

_PUBLIC int32_t pups_clear_chtab_slot(const _BOOLEAN destroy, const uint32_t chld_index)

{   if(chld_index >= appl_max_child || (destroy != FALSE && destroy != TRUE))
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&chtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    chtab[chld_index].pid      = (-1);
    chtab[chld_index].obituary = FALSE;

    if(destroy == FALSE)
       (void)strlcpy(chtab[chld_index].name,"",SSIZE);
    else
       (void)strlcpy(chtab[chld_index].name,"",SSIZE);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&chtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}




/*---------------------*/
/* Display child table */
/*---------------------*/

_PUBLIC void pups_show_children(const FILE *stream)

{   uint32_t i,
             children = 0;

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_children] attempt by non root thread to perform PUPS/P3 global utility operation");

    (void)fprintf(stream,"\n\n    Active children\n");
    (void)fprintf(stream,"    ===============\n\n");    
    (void)fflush(stream);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&chtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_child; ++i)
    {  if(chtab[i].pid != (-1))     /*----------------------------------------------------------*/
       {  char name[SSIZE] = "";    /* Could be a pipeline so we need to reserve space for this */
                                    /*----------------------------------------------------------*/

          if(chtab[i].name == (char *)NULL)
             (void)strlcpy(name,"<unnamed>",SSIZE);
          else
             (void)strlcpy(name,chtab[i].name,SSIZE); 

          if(chtab[i].obituary == TRUE)
             (void)fprintf(stream,"    %04d: child pid %09d (%-32s) [obituary]\n",i,chtab[i].pid,name);
          else
             (void)fprintf(stream,"    %04d: child pid %08d (%-32s)\n",i,chtab[i].pid,name);

          (void)fflush(stream);

          ++children;
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&chtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(children > 1)
       (void)fprintf(stream,"\n\n    %04d active children (%04d slots free)\n\n",children,appl_max_child - children);
    else if(children == 1)
       (void)fprintf(stream,"\n\n    %04d active child (%04d slots free)\n\n",1,appl_max_child - 1);
    else
       (void)fprintf(stream,"    no active children (%04d slots free)\n\n",appl_max_child);
    (void)fflush(stream);

    pups_set_errno(OK);
}




/*----------------*/
/* Set child name */
/*----------------*/

_PUBLIC _BOOLEAN pups_set_child_name(const pid_t pid, const char *name)

{   uint32_t i;

    if(pid <= 0 || name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(FALSE);
    }
    else
       pups_set_errno(OK);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&pups_fork_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<MAX_CHILDREN; ++i)
    {  if(chtab[i].pid == pid)
       {  (void)strlcpy(chtab[i].name,name,SSIZE);
     
          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&pups_fork_mutex);
          #endif /* PTHREAD_SUPPORT */

          return(TRUE);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&pups_fork_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(FALSE);
}




/*-----------------------------------------------------------------*/
/* Routine to handle SIGCHLD (removes child info from child table) */
/*-----------------------------------------------------------------*/

_PRIVATE void chld_handler(const int32_t signum)

{   uint32_t i;
    pid_t    pid;
    int32_t  status;

    while((pid = waitpid((-1),&status,WNOHANG)) > 0)
    {    
         #ifdef PTHREAD_SUPPORT
         (void)pthread_mutex_lock(&pups_fork_mutex);
         #endif /* PTHREAD_SUPPORT */

         for(i=0; i<MAX_CHILDREN; ++i)
         {  if(chtab[i].pid == pid) 
            {  if(appl_verbose == TRUE)
               {  if(chtab[i].obituary == TRUE)
                  {  (void)strdate(date);
                     (void)fprintf(stderr,"%s %s(%d@%s): [chld_handler] child process %d (%s) has exited\n",
                                                        date,appl_name,getpid(),appl_host,pid,chtab[i].name);

                     if(n_children > 1)
                        (void)fprintf(stderr,"%s %s(%d@%s): [chld_handler] clearing child table slot %d (%d children)\n",
                                                                          date,appl_name,getpid(),appl_host,i,n_children);
                     else
                        (void)fprintf(stderr,"%s %s(%d@%s): [chld_handler] clearing child table slot %d (1 child)\n",
                                                                                 date,appl_name,getpid(),appl_host,i);
                     (void)fflush(stderr);
                  }
               }

               chtab[i].pid = (-1);
               (void)strlcpy(chtab[i].name,"",SSIZE);

               --n_children;
               i = MAX_CHILDREN + 1;
            }
         }

         #ifdef PTHREAD_SUPPORT
         (void)pthread_mutex_unlock(&pups_fork_mutex);
         #endif /* PTHREAD_SUPPORT */
     }

     return;
}




/*-----------------------------------------------------*/
/* Install handler for PUPS automatic child management */
/*-----------------------------------------------------*/

_PUBLIC void pups_auto_child(void)

{   pups_sighandle(SIGCHLD,"child_handler",(void *)chld_handler, (sigset_t *)NULL);
}




/*---------------------------------------------------*/
/* Remove handler for PUPS automatic child managment */
/*---------------------------------------------------*/

_PUBLIC void pups_noauto_child(void)

{   pups_sighandle(SIGCHLD,"ignored",SIG_DFL, (sigset_t *)NULL);
}




/*----------------------------------------------------*/
/* Wait function compatable with PUPS child managment */
/*----------------------------------------------------*/

_PUBLIC int32_t pupswait(_BOOLEAN wait_verbose, int32_t *status)

{   uint32_t i;
    pid_t    pid;


    /*--------------*/
    /* Sanity check */
    /*--------------*/

    if(status == (int32_t *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*------------------------*/
    /* Wait for child to exit */
    /*------------------------*/

    pups_noauto_child();
    do {   pid = waitpid((-1),status,WNOHANG);

           if(pid == 0)
             (void)pups_usleep(100);
           else
           {  pups_auto_child(); 
              pups_set_errno(ECHILD);
              return(-1);
           }

        } while(pid == 0);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&pups_fork_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*--------------------*/
    /* Update child table */
    /*--------------------*/

    for(i=0; i<MAX_CHILDREN; ++i)
    {   if(chtab[i].pid == pid)
        {  if(wait_verbose == TRUE)
           {  if(chtab[i].obituary == TRUE)
              {  (void)strdate(date);
                 (void)fprintf(stderr,"%s %s(%d@%s): [pupswait] child process %d (%s) has exited\n",
                                                date,appl_name,appl_pid,appl_host,pid,chtab[i].name);

                 if(n_children == 1)
                    (void)fprintf(stderr,"%s %s(%d@%s): [pupswait] clearing child slot %d (1 child)\n",
                                                                   date,appl_name,appl_pid,appl_host,i);
                 else
                    (void)fprintf(stderr,"%s %s(%d@%s): [pupswait] clearing child slot %d (%d children)\n",
                                                            date,appl_name,appl_pid,appl_host,i,n_children);
              }
              (void)fflush(stderr);
           }

           chtab[i].pid = (-1);

           (void)strlcpy(chtab[i].name,"",SSIZE);
           --n_children;

           pups_auto_child();

           #ifdef PTHREAD_SUPPORT
           (void)pthread_mutex_unlock(&pups_fork_mutex);
           #endif /* PTHREAD_SUPPORT */

           pups_set_errno(OK);
           return(pid);
        }
    }

    pups_auto_child();

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&pups_fork_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(EEXIST);
    return(-1);
}




/*-------------------------------------------------------*/
/* Waitpid function compatable with PUPS child managment */
/*-------------------------------------------------------*/

_PUBLIC int32_t pupswaitpid(const _BOOLEAN waitpid_verbose, const pid_t wait_pid, int32_t *status)

{   uint32_t i;
    pid_t    pid;


    /*--------------*/
    /* Sanity check */
    /*--------------*/

    if(wait_pid <= 0 || status == (int32_t *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*------------------------*/
    /* Wait for child to exit */
    /*------------------------*/

    pups_noauto_child();
    do {   pid = waitpid(wait_pid,status,WNOHANG);

           if(pid == 0)
             (void)pups_usleep(100);
           else
           {  pups_auto_child();
              pups_set_errno(ECHILD);

              return(-1);
           }

        } while(pid == 0);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&pups_fork_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*--------------------*/
    /* Update child table */
    /*--------------------*/

    for(i=0; i<MAX_CHILDREN; ++i)
    {  if(chtab[i].pid == pid)
       {  if(waitpid_verbose == TRUE)
          {  if(chtab[i].obituary == TRUE)
             {  (void)strdate(date);
                (void)fprintf(stderr,"%s %s(%d@%s): [pupswaitpid] child process %d (%s) has exited\n",
                                                  date,appl_name,getpid(),appl_host,pid,chtab[i].name);

                (void)fprintf(stderr,"%s %s(%d@%s): [pupswaitpid] clearing child slot %d (%d children)\n",
                                                           date,appl_name,getpid(),appl_host,i,n_children);

                (void)fflush(stderr);
             }
          }

          chtab[i].pid = (-1);
          (void)strlcpy(chtab[i].name,"",SSIZE);

          --n_children;

          pups_auto_child();

         #ifdef PTHREAD_SUPPORT
         (void)pthread_mutex_unlock(&pups_fork_mutex);
         #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK); 
          return(pid);
       }
    }

    pups_auto_child();

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&pups_fork_mutex);
    #endif /* PTHREAD_SUPPORT */


    pups_set_errno(EEXIST);
    return(-1);
}



/*---------------------------*/
/* Allocate dynamic 2D array */
/*---------------------------*/

_PUBLIC void **pups_aalloc(pindex_t rows, pindex_t cols, psize_t size)

{   uint32_t i;

    void *array_mem = (void *) NULL;
    void **array    = (void **)NULL;    


    /*-------------------------------------------*/
    /* First allocate memory for the whole array */
    /*-------------------------------------------*/

    array_mem = (void *)pups_malloc(rows*cols*size);


    /*------------------------------------*/
    /* Now row allocate the pointer array */
    /*------------------------------------*/

    array = (void **)pups_malloc(rows*sizeof(void **));


    /*---------------------------------------------------------*/
    /* Now allocate the columns addressed by the pointer array */
    /*---------------------------------------------------------*/

    for(i=0; i<rows; ++i)
    {  array[i]  = (void *)array_mem;
       array_mem = (void *)((uint64_t)array_mem + (uint64_t)(cols*size));
    } 

    return(array);
}




/*-----------------------*/
/* Free dynamic 2D array */
/*-----------------------*/

_PUBLIC void **pups_afree(pindex_t rows, void **array)

{    if(array != (void **)NULL)
        (void)pups_free((void *)array[0]);

     return(pups_free((void *)array));
}



/*--------------------------------------------------------*/
/*  Sort an array of floating point values using heapsort */
/*--------------------------------------------------------*/

_PUBLIC void pups_sort(int32_t n, FTYPE ra[], FTYPE rb[])

{   int32_t l,
            j,
            ir,
            i;

    FTYPE   rrb,
            rra;

    l = (n >> 1) + 1;

    ir = n;
    for(;;)
    {   if(l > 0)
        {  rra = ra[--l];
           rrb = rb[l];
        }
        else
        {  rra    = ra[ir];
           rrb    = rb[0];
           rb[ir] = rb[0];

           if(--ir == 0)
           {  ra[0] = rra;
              rb[0] = rrb;
              return;
           }
        }

        i = 1;
        j = l << 1;

        while(j <= ir)
        {   if(j < ir && ra[j] < ra[j+1])
               ++j;

            if(rra < ra[j])
            {   ra[i] = ra[j];
                rb[i] = rb[j];
                j     += (i + j);
            }
            else
               j = ir + 1;
        }

        ra[i] = rra;
        ra[i] = rrb;
    }
}




/*------------------------------------*/
/* Check stdio descriptor redirection */
/*------------------------------------*/

_PUBLIC void pups_check_redirection(const des_t des)

{    if(isatty(des) == 1)
     {  switch(des)
        {   case 0:  pups_error("[check_redirection] stdin not redirected");

            case 1:  pups_error("[check_redirection] stdout not redirected");

            case 2:  pups_error("[check_redirection] stderr not redirected");

            default: pups_error("[check_redirection] tty device not redirected");
        }
     }
}
 



/*-------------------*/
/* Set process state */
/*-------------------*/

_PUBLIC void pups_set_state(const char *state)

{   if(state == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }
    else
       pups_set_errno(OK); 

    (void)strlcpy(appl_state,state,SSIZE);
}




/*-------------------*/
/* Get process state */
/*-------------------*/

_PUBLIC void pups_get_state(const char *state)

{   if(state == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }
    else
       pups_set_errno(OK); 

    (void)strlcpy(state,appl_state,SSIZE);
}




/*-----------------------*/
/* Display process state */
/*-----------------------*/

_PUBLIC void pups_show_state(void)

{   FILE *info_stream = (FILE *)NULL,
         *io_stream   = (FILE *)NULL;

    char tty_dev_name[SSIZE]  = "",
         tty_info_file[SSIZE] = "";


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_state] attempt by non root thread to perform PUPS/P3 utility operation");


    /*------------------------------------------------------*/
    /* Get the name of the I/O terminal that is to be used. */
    /*------------------------------------------------------*/

    snprintf(tty_info_file,SSIZE,"/tmp/info.tmp.%d",getpid());
    info_stream = pups_fopen(tty_info_file,"r",LIVE);
    fscanf(info_stream,"%s",tty_dev_name);
    (void)pups_fclose(info_stream);

    io_stream = pups_fopen(tty_dev_name,"r+",LIVE);

    (void)fprintf(io_stream,"    Application %s [pid %d]@%s\n",appl_name,
                                                                getpid(), 
                                                               appl_host);

    (void)fprintf(io_stream,"\n    Application state: %s\n",appl_state);
    (void)fflush(io_stream);

    pups_fclose(io_stream);
}





/*------------------------------------*/
/* Register process entrance function */
/*------------------------------------*/

_PUBLIC int32_t max_entrance_funcs     = MAX_FUNCS;
_PUBLIC int32_t n_entrance_funcs       = 0;
_PUBLIC int32_t n_entrance_funcs_alloc = 0;

_PUBLIC char *pups_entrance_f_name[MAX_FUNCS];
_PUBLIC char *pups_entrance_arg[MAX_FUNCS];
_PUBLIC void (*pups_entrance_f[MAX_FUNCS])(char *);

_PUBLIC int32_t pups_register_entrance_f(const char *f_name, 
                                         const void *entrance_f, 
                                         const char *arg_str)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("pups_register_entrance_f] attempt by non root thread to perform PUPS/P3 utility operation");


    if(f_name     == (const char *)NULL  ||
       entrance_f == (const void *)NULL   ) 
    {  pups_set_errno(EINVAL);
       return(PUPS_ERROR);
    }


    /*--------------------------------------------------------------*/
    /* Check to see if the entrance function table is full and that */
    /* we have a function name                                      */
    /*--------------------------------------------------------------*/

    if(n_entrance_funcs == max_entrance_funcs)
    {  pups_set_errno(ERANGE);
       return(PUPS_ERROR);
    }


    /*------------------------------------*/
    /* Find a free entrance function slot */
    /*------------------------------------*/

    for(i=0; i<n_entrance_funcs_alloc; ++i)
    {   if((void *)pups_entrance_f[i] == (void *)NULL)
        {  pups_entrance_f[i] = entrance_f; 

           (void)strlcpy(pups_entrance_f_name[i],f_name,SSIZE);
           if(arg_str != (char *)NULL)
             (void)strlcpy(pups_entrance_arg[i],arg_str,SSIZE);

           if(appl_verbose == TRUE)
           {  strdate(date);
              (void)fprintf(stderr,
                      "%s %s (%d@%s:%s): PUPS entrance function %s (%016lx) registered at slot %d\n",
                                                                                                date,
                                                                                           appl_name,
                                                                                            getpid(),
                                                                                           appl_host,
                                                                                          appl_owner,
                                                                             pups_entrance_f_name[i], 
                                                                                  pups_entrance_f[i],
                                                                                                   i);
              (void)fflush(stderr);
           }

           ++n_entrance_funcs;

           pups_set_errno(OK);
           return(PUPS_OK);
        }
    }


    /*--------------------------------------------------------*/
    /* Allocate next free slot and register entrance function */
    /*--------------------------------------------------------*/

    pups_entrance_f_name[n_entrance_funcs_alloc] = (char *)pups_malloc(SSIZE);
    pups_entrance_f[n_entrance_funcs_alloc]      = entrance_f;

    (void)strlcpy(pups_entrance_f_name[n_entrance_funcs_alloc],f_name,SSIZE);

    if(arg_str != (char *)NULL)
    {  pups_entrance_arg[n_entrance_funcs_alloc]    = (char *)pups_malloc(SSIZE);
       (void)strlcpy(pups_entrance_arg[n_entrance_funcs_alloc],arg_str,SSIZE);
    }

    if(appl_verbose == TRUE)
    {  strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): PUPS exit function \"%-32s\" (at %016lx virtual) registered at slot %d\n",
                                                                                                                    date,
                                                                                                               appl_name,
                                                                                                                getpid(),
                                                                                                               appl_host,
                                                                                                              appl_owner,
                                                                            pups_entrance_f_name[n_entrance_funcs_alloc], 
                                                                       (uint64_t)pups_entrance_f[n_entrance_funcs_alloc],
                                                                                              n_entrance_funcs_alloc + 1);
       (void)fflush(stderr);
    }

    ++n_entrance_funcs;
    ++n_entrance_funcs_alloc;

    pups_set_errno(OK);
    return(PUPS_OK);
}



/*--------------------------------------*/
/* Deregister process entrance function */
/*--------------------------------------*/

_PUBLIC int32_t pups_deregister_entrance_f(const void *func)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_deregister_entrance_f] attempt by non root thread to perform PUPS/P3 utility operation");

    if(func   == (const void *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    for(i=0; i<n_entrance_funcs_alloc; ++i)
    {  if((void *)pups_entrance_f[i] == func)
       {  if(appl_verbose == TRUE) 
          {  strdate(date);
             (void)fprintf(stderr, "%s %s (%d@%s:%s): PUPS entrance function \"%032s\" (at %016lx virtual) at slot %d deregistered\n",
                                                                                                                                 date,
                                                                                                                            appl_name,
                                                                                                                             getpid(),
                                                                                                                            appl_host,
                                                                                                                           appl_owner,
                                                                                                              pups_entrance_f_name[i],
                                                                                                                   pups_entrance_f[i],
                                                                                                                                    i);
             (void)fflush(stderr);
          }
      
          pups_entrance_f[i] = (void *)NULL;
          (void)strlcpy(pups_entrance_f_name[i],"",SSIZE);

          if(pups_entrance_arg[i] != (char *)NULL)
            (void)strlcpy(pups_entrance_arg[i],"",SSIZE);

          --n_entrance_funcs;

          pups_set_errno(OK);
          return(PUPS_OK);
       }
    }

    pups_set_errno(ESRCH);
    return(PUPS_ERROR);
}




/*------------------------------------*/
/* Display process entrance functions */
/*------------------------------------*/

_PUBLIC void pups_show_entrance_f(const FILE *stream)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_entrance_f] attempt by non root thread to perform PUPS/P3 utility operation");

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }

    if(n_entrance_funcs == 0)
       (void)fprintf(stream,"\n\n    No entrance functions registered (%d slots free0\n",n_entrance_funcs_alloc);
    else
    {  (void)fprintf(stream,"\n\n    Entrance functions registered\n");
       (void)fprintf(stream,"    =============================\n\n");
       (void)fflush(stream);
   

       for(i=0; i<n_entrance_funcs_alloc; ++i)
       {  if((void *)pups_entrance_f[i] != (void *)NULL)
          {  (void)fprintf(stream,"    %04d: \"%-32s\" (at %016lx virtual)\n",i,
                                                        pups_entrance_f_name[i],
                                                   (uint64_t)pups_entrance_f[i]);
             (void)fflush(stream);
          }
       }

       if(n_entrance_funcs > 1)
          (void)fprintf(stream,"\n\n    %04d entrance functions registered (%04d slots free)\n\n",n_entrance_funcs,n_entrance_funcs_alloc - n_entrance_funcs);
       else if(n_entrance_funcs == 1)
          (void)fprintf(stream,"\n\n    %04d entrance function registered (%04d slots free)\n\n",1,n_entrance_funcs_alloc - 1);

       (void)fflush(stream);
    }

    pups_set_errno(OK);
}




/*--------------------------------------*/
/* Flag unparsed command line arguments */
/*--------------------------------------*/

_PUBLIC void pups_t_arg_errs(const _BOOLEAN argd[], const char *args[])

{    uint32_t i;
    _BOOLEAN  parse_failed = FALSE;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_t_arg_errs] attempt by non root thread to perform PUPS/P3 utility operation");

    if(argd == (const _BOOLEAN **)NULL || args == (const char **)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }


    /*--------------------------------------------------------------*/
    /* Before we see if all arguments are correctly decoded run all */
    /* entrance functions                                           */
    /*--------------------------------------------------------------*/

    for(i=0; i<n_entrance_funcs_alloc; ++i)
    {  if((void *)pups_entrance_f[i] != (void *)NULL)
       {  (void)strdate(date);
          if(appl_verbose == TRUE)
          {  (void)fprintf(stderr,"%s %s (%d@%s:%s): executing entrance function \"%-32s\" (slot %d at %016lx virtual)\n",
                                                                             date,appl_name,appl_pid,appl_host,appl_owner,
                                                                                                  pups_entrance_f_name[i],
                                                                                                                        i,
                                                                                            (uint64_t)pups_entrance_f[i]);
             (void)fflush(stderr);
          }

          (*pups_entrance_f[i])(pups_entrance_arg[i]);
       }
    }

    for(i=0; i<t_args; ++i)
    {   if(argd[i] == FALSE && strcmp(args[i],"") != 0)
        {  if(parse_failed == FALSE)
           {  (void)fprintf(stderr,"\n\n");
              parse_failed = TRUE;
           }

           (void)strdate(date);
           (void)fprintf(stderr,"%s %s (%d@%s:%s): %sERROR%s failed to parse: %s[%d]\n",
                         date,appl_name,appl_pid,appl_host,appl_owner,boldOn,boldOff,args[i],i);
           (void)fflush(stderr);
        }
    }

    if(appl_verbose == TRUE)
      (void)pups_release_fd_lock(2);

    if(parse_failed == TRUE)
       pups_exit(255);

    pups_set_errno(OK);
}




/*--------------------------------*/
/* Register process exit function */
/*--------------------------------*/

_PUBLIC int32_t max_exit_funcs     = MAX_FUNCS;
_PUBLIC int32_t n_exit_funcs       = 0;
_PUBLIC int32_t n_exit_funcs_alloc = 0;

_PUBLIC char *pups_exit_f_name[MAX_FUNCS];
_PUBLIC char *pups_exit_arg[MAX_FUNCS];
_PUBLIC void (*pups_exit_f[MAX_FUNCS])(char *);

_PUBLIC int32_t pups_register_exit_f(const char *f_name, 
                                     const void *exit_f, 
                                     const char *arg_str)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_register_entrance_f] attempt by non root thread to perform PUPS/P3 utility operation");

    if(f_name  == (const char *)NULL  ||
       exit_f  == (const void *)NULL   ) 
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*----------------------------------------------------------*/
    /* Check to see if the exit function table is full and that */
    /* we have a function name                                  */
    /*----------------------------------------------------------*/

    if(n_exit_funcs == max_exit_funcs)
       return(PUPS_ERROR);


    /*--------------------------------*/
    /* Find a free exit function slot */
    /*--------------------------------*/

    for(i=0; i<n_exit_funcs_alloc; ++i)
    {   if((void *)pups_exit_f[i] == (void *)NULL)
        {  pups_exit_f[i] = exit_f; 

           (void)strlcpy(pups_exit_f_name[i],f_name,SSIZE);
           if(arg_str != (char *)NULL)
             (void)strlcpy(pups_exit_arg[i],arg_str,SSIZE);

           if(appl_verbose == TRUE)
           {  strdate(date);
              (void)fprintf(stderr,
                      "%s %s (%d@%s:%s): PUPS exit function %s (%016lx) registered at slot %d\n",
                                                                                            date,
                                                                                       appl_name,
                                                                                        getpid(),
                                                                                       appl_host,
                                                                                      appl_owner,
                                                                             pups_exit_f_name[i], 
                                                                                  pups_exit_f[i],
                                                                                               i);
              (void)fflush(stderr);
           }

           ++n_exit_funcs;

           pups_set_errno(OK);
           return(PUPS_OK);
        }
    }


    /*----------------------------------------------------*/
    /* Allocate next free slot and register exit function */
    /*----------------------------------------------------*/

    pups_exit_f_name[n_exit_funcs_alloc] = (char *)pups_malloc(SSIZE);
    pups_exit_f[n_exit_funcs_alloc]      = exit_f;

    (void)strlcpy(pups_exit_f_name[n_exit_funcs_alloc],f_name,SSIZE);

    if(arg_str != (char *)NULL)
    {  pups_exit_arg[n_exit_funcs_alloc]    = (char *)pups_malloc(SSIZE);
       (void)strlcpy(pups_exit_arg[n_exit_funcs_alloc],arg_str,SSIZE);
    }

    if(appl_verbose == TRUE)
    {  strdate(date);
       (void)fprintf(stderr,
               "%s %s (%d@%s:%s): PUPS exit function \"%-32s\" (at %016lx virtual) registered at slot %d\n",
                                                                                                       date,
                                                                                                  appl_name,
                                                                                                   getpid(),
                                                                                                  appl_host,
                                                                                                 appl_owner,
                                                                       pups_exit_f_name[n_exit_funcs_alloc], 
                                                                  (uint64_t)pups_exit_f[n_exit_funcs_alloc],
                                                                                     n_exit_funcs_alloc + 1);
       (void)fflush(stderr);
    }

    ++n_exit_funcs;
    ++n_exit_funcs_alloc;

    pups_set_errno(OK);
    return(PUPS_OK);
}



/*----------------------------------*/
/* Deregister process exit function */
/*----------------------------------*/

_PUBLIC int32_t pups_deregister_exit_f(const void *func)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_deregister_entrance_f] attempt by non root thread to perform PUPS/P3 utility operation");

    if(func   == (const void *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    for(i=0; i<n_exit_funcs_alloc; ++i)
    {  if((void *)pups_exit_f[i] == func)
       {  if(appl_verbose == TRUE) 
          {  strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): PUPS exit function \"%-32s\" (at %016lx virtual) at slot %d deregistered\n",
                                                                                                                            date,
                                                                                                                       appl_name,
                                                                                                                        getpid(),
                                                                                                                       appl_host,
                                                                                                                      appl_owner,
                                                                                                             pups_exit_f_name[i],
                                                                                                        (uint64_t)pups_exit_f[i],
                                                                                                                               i);
             (void)fflush(stderr);
          }
      
          pups_exit_f[i] = (void *)NULL;
          (void)strlcpy(pups_exit_f_name[i],"",SSIZE);

          if(pups_exit_arg[i] != (char *)NULL)
             (void)strlcpy(pups_exit_arg[i],"",SSIZE);

          --n_exit_funcs;

          pups_set_errno(OK);
          return(PUPS_OK);
       }
    }

    pups_set_errno(OK);
    return(PUPS_ERROR);
}




/*--------------------------------*/
/* Display process exit functions */
/*--------------------------------*/

_PUBLIC void pups_show_exit_f(const FILE *stream)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_exit_f] attempt by non root thread to perform PUPS/P3 utility operation");

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }


    /*--------------------------------------------------------*/
    /* Check that we actually have a valid stream to write to */
    /*--------------------------------------------------------*/

    if(stream == (FILE *)NULL)
       return;

    if(n_exit_funcs == 0)
       (void)fprintf(stream,"\n\n    No exit functions registered (%04d slots free)\n\n",n_exit_funcs_alloc);
    else
    {  (void)fprintf(stream,"\n\n    Exit functions registered\n");
       (void)fprintf(stream,"    =========================\n\n");
       (void)fflush(stream);


       for(i=0; i<n_exit_funcs_alloc; ++i)
       {  if((void *)pups_exit_f[i] != (void *)NULL)
          {  (void)fprintf(stream,"    %04d: \"%-32s\" (at %016lx virtual)\n",i,
                                                            pups_exit_f_name[i],
                                                       (uint64_t)pups_exit_f[i]);
             (void)fflush(stream);
          }
       }

       if(n_exit_funcs > 1)
          (void)fprintf(stream,"\n\n    %04d exit functions registered (%04d slots free)\n\n",n_exit_funcs,n_exit_funcs_alloc - n_exit_funcs);
       else if(n_exit_funcs == 1)
          (void)fprintf(stream,"\n\n    %04d exit function registered (%04d slots free)\n\n",1,n_exit_funcs_alloc - 1);

       (void)fflush(stream);
    }

    pups_set_errno(OK);
}




/*---------------------------------*/
/* Register process abort function */
/*---------------------------------*/

_PUBLIC int32_t max_abort_funcs     = MAX_FUNCS;
_PUBLIC int32_t n_abort_funcs       = 0;
_PUBLIC int32_t n_abort_funcs_alloc = 0;

_PUBLIC void (*pups_abort_f[MAX_FUNCS])(char *);
_PUBLIC char *pups_abort_f_name[MAX_FUNCS];
_PUBLIC char *pups_abort_arg[MAX_FUNCS];


_PUBLIC int32_t pups_register_abort_f(const char *f_name,
                                      const void  *abort_f,
                                      const char *arg_str)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_register_abort_f] attempt by non root thread to perform PUPS/P3 utility operation");

    if(f_name   == (const char *)NULL  ||
       abort_f  == (const void *)NULL   ) 
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*----------------------------------------------------------*/
    /* Check to see if the exit function table is full and that */
    /* we have a function name                                  */
    /*----------------------------------------------------------*/

    if(n_abort_funcs == max_abort_funcs || f_name == (char *)NULL)
    {  pups_set_errno(ERANGE);
       return(PUPS_ERROR);
    }


    /*---------------------------------*/
    /* Find a free abort function slot */
    /*---------------------------------*/

    for(i=0; i<n_abort_funcs_alloc; ++i)
    {   if((void *)pups_abort_f[i] == (void *)NULL)
        {  pups_abort_f[i] = abort_f;

           (void)strlcpy(pups_abort_f_name[i],f_name,SSIZE);
           if(arg_str != (char *)NULL)
             (void)strlcpy(pups_abort_arg[i],arg_str,SSIZE);

           if(appl_verbose == TRUE)
           {  (void)strdate(date);
              (void)fprintf(stderr,"%s %s (%d@%s:%s): PUPS abort function %s (%018x) registered at slot %d\n",
                                                                                                         date,
                                                                                                    appl_name,
                                                                                                     appl_pid,
                                                                                                    appl_host,
                                                                                                   appl_owner,
                                                                                         pups_abort_f_name[i],
                                                                                              pups_abort_f[i],
                                                                                                            i);
              (void)fflush(stderr);
           }

           ++n_abort_funcs;

           pups_set_errno(OK);
           return(PUPS_OK);
        }
    }


    /*-----------------------------------------------------*/
    /* Allocate next free slot and register abort function */
    /*-----------------------------------------------------*/

    pups_abort_f_name[n_abort_funcs_alloc] = (char *)pups_malloc(SSIZE);
    pups_abort_f[n_abort_funcs_alloc]      = abort_f;

    (void)strlcpy(pups_abort_f_name[n_abort_funcs_alloc],f_name,SSIZE);

    if(arg_str != (char *)NULL)
    {  pups_abort_arg[n_abort_funcs_alloc]    = (char *)pups_malloc(SSIZE);
       (void)strlcpy(pups_abort_arg[n_abort_funcs_alloc],arg_str,SSIZE);
    }

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): PUPS abort function \"%-32s\" (at %016lx virtual) registered at slot %d\n",
                                                                             date,appl_name,appl_pid,appl_host,appl_owner,
                                                                                   pups_abort_f_name[n_abort_funcs_alloc],
                                                                              (uint64_t)pups_abort_f[n_abort_funcs_alloc],
                                                                                                  n_abort_funcs_alloc + 1);
       (void)fflush(stderr);
    }

    ++n_abort_funcs;
    ++n_abort_funcs_alloc;

    pups_set_errno(OK);
    return(PUPS_OK);
}




/*-----------------------------------*/
/* Deregister process abort function */
/*-----------------------------------*/

_PUBLIC  int32_t pups_deregister_abort_f(const void *func)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_deregister_abort_f] attempt by non root thread to perform PUPS/P3 utility operation");

    if(func   == (const void *)NULL   )
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    for(i=0; i<n_abort_funcs_alloc; ++i)
    {  if((void *)pups_abort_f[i] == func)
       {  if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): PUPS abort function \"%-32s\" (at %018x virtual) at slot %d deregistered\n",
                                                                                                                            date,
                                                                                                                       appl_name,
                                                                                                                        appl_pid,
                                                                                                                       appl_host,
                                                                                                                      appl_owner,
                                                                                                            pups_abort_f_name[i],
                                                                                                       (uint64_t)pups_abort_f[i],
                                                                                                                               i);
             (void)fflush(stderr);
          }

          pups_abort_f[i] = (void *)NULL;
          (void)strlcpy(pups_abort_f_name[i],"",SSIZE);

          if(pups_abort_arg[i] != (char *)NULL)
             (void)strlcpy(pups_abort_arg[i],"",SSIZE);

          --n_abort_funcs;

          pups_set_errno(OK);
          return(PUPS_OK);
       }
    }

    pups_set_errno(ESRCH);
    return(PUPS_ERROR);
}




/*---------------------------------*/
/* Display process abort functions */
/*---------------------------------*/

_PUBLIC void pups_show_abort_f(const FILE *stream)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[show_pups_abort_f] attempt by non root thread to perform PUPS/P3 global utility operation");

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }

    if(n_abort_funcs == 0)
       (void)fprintf(stream,"\n\n    No abort functions registered (%04d slots free)\n\n",n_abort_funcs_alloc);
    else
    {  (void)fprintf(stream,"\n\n    Abort functions registered\n");
       (void)fprintf(stream,"    ==========================\n\n");
       (void)fflush(stream);

       for(i=0; i<n_abort_funcs_alloc; ++i)
       {  if((void *)pups_abort_f[i] != (void *)NULL)
          {  (void)fprintf(stream,"    %04d: \"%-32s\" (at %016lx virtual)\n",i,
                                                           pups_abort_f_name[i],
                                                      (uint64_t)pups_abort_f[i]);
             (void)fflush(stream);
          }
       }

       if(n_abort_funcs > 1)
          (void)fprintf(stream,"\n\n    %04d abort functions registered (%04d slots free)\n\n",n_abort_funcs,n_abort_funcs_alloc - n_abort_funcs);
       else if(n_abort_funcs == 1)
          (void)fprintf(stream,"\n\n    %04d abort function registered (%04d slots free)\n\n",1,n_abort_funcs_alloc - 1);

       (void)fflush(stream);
    }

    pups_set_errno(OK);
}



/*--------------------*/
/* PUPS exit function */
/*--------------------*/

_PUBLIC  int32_t pups_exit(const int32_t exit_code)

{   uint32_t i;
    int32_t  lfl_cnt                   = 0;

    char     tunnel_f_name   [SSIZE]       = "",
             psrp_channel_lockname[SSIZE]  = "";

    sigset_t set,
             old_set;;


    /*--------------------------------------------*/
    /* Disable signal handling via hold/relse API */
    /*--------------------------------------------*/

    in_pups_exit = TRUE;


    /*---------------------------------------*/
    /* Clear alarm for virtual timer systems */
    /* and block signals                     */
    /*---------------------------------------*/

    sigfillset(&set);
    (void)pups_sigprocmask(SIG_BLOCK,&set,&old_set);


    #ifdef PTHREAD_SUPPORT
    int32_t retval;


    /*------------------------------------*/
    /* If we have been called by a thread */
    /* terminate that thread and not the  */
    /* entire PUPS?P3 server              */
    /*------------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
    {  if(appl_verbose == TRUE)
       {  pthread_t tid;
          char      tfuncname[SSIZE] = "";

          tid = pthread_self();
          (void)pupsthread_tid2tfuncname(tid,tfuncname);
          (void)strdate(date);

          (void)fprintf(stderr,"%s %s (%d@%s:%s): thread %016lx (%s) exiting\n",
                                    date,appl_name,appl_pid,appl_host,appl_owner,
                                                         (uint64_t)tid,tfuncname);
          (void)fflush(stderr);
       }
       (void)pupsthread_exit(&retval);
    }
    else
       (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*----------------------------------------*/
    /* If we are being monitored by a softdog */
    /* instance termimnate it                 */
    /*----------------------------------------*/

    if (appl_softdog_enabled == TRUE)
    {  (void)kill(appl_softdog_pid,SIGTERM);

       (void)fprintf(stderr,"\n%s %s (%d@%s:%s): softdog terminated\n\n",date,appl_name,appl_pid,appl_host,appl_owner);
       (void)fflush(stderr);
    }


    #ifdef CRIU_SUPPORT
    /*-----------------------------------------------------*/
    /* Remove (Criu) saved state (if state saving enabled) */
    /*-----------------------------------------------------*/

    if(appl_ssave == TRUE)
       pups_ssave_disable();
    #endif /* CRIU_SUPPORT */


    /*----------------------------------------*/
    /* Execute registered PUPS exit functions */
    /*----------------------------------------*/

    for(i=0; i<n_exit_funcs_alloc; ++i)
    {  if((void *)pups_exit_f[i] != (void *)NULL)
       {  if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): executing exit function \"%-32s\" (slot %d at %016lx virtual)\n",
                                                                                                                 date,
                                                                                                            appl_name,
                                                                                                   appl_pid,appl_host,
                                                                                                           appl_owner,
                                                                                                  pups_exit_f_name[i],
                                                                                                                    i,
                                                                                             (uint64_t)pups_exit_f[i]);
             (void)fflush(stderr);
          }

          (*pups_exit_f[i])(pups_exit_arg[i]);
       }
    }


    /*---------------------------------------*/
    /* If we are in PSRP mode clear channels */
    /*---------------------------------------*/
    /*--------------------------------------------------------------*/
    /* Workaround bug in RedHat 6.0/egcs-1.1.2 which cause problems */
    /* with data transmission down FIFOS's                          */
    /*--------------------------------------------------------------*/

    (void)pups_usleep(100);


    /*-------------------------*/
    /* Clear psrp channel lock */
    /*-------------------------*/

    (void)snprintf(psrp_channel_lockname,SSIZE,"/tmp/psrp.%d.lock",appl_pid);
    (void)unlink(psrp_channel_lockname);


    /*----------------------------------*/
    /* CLear all active link file locks */
    /*----------------------------------*/

    lfl_cnt = pups_release_all_link_file_locks();

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       if(lfl_cnt > 0)
          (void)fprintf(stderr,"\n%s %s (%d@%s:%s): %d link file locks released\n\n",date,appl_name,appl_pid,appl_host,appl_owner,lfl_cnt);
       else
          (void)fprintf(stderr,"\n%s %s (%d@%s:%s): no link file locks to release\n\n",date,appl_name,appl_pid,appl_host,appl_owner);

       (void)fflush(stderr);
    }

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"\n%s %s (%d@%s:%s): executing standard exit functions\n\n",date,appl_name,appl_pid,appl_host,appl_owner);
       (void)fflush(stderr);
    }


    /*------------------------*/
    /* Detach all fast caches */
    /*------------------------*/

    cache_exit();


    #ifdef PERSISTENT_HEAP_SUPPORT
    /*---------------------------------------------------------*/
    /* If we have persistent heaps attached -- detach them now */
    /*---------------------------------------------------------*/

    msm_exit(0);
    #endif /* PERSISTENT_HEAP_SUPPORT */


    /*-----------------------------*/
    /* Close all (non stdio) files */
    /*-----------------------------*/

    (void)pups_closeall();

    vt_no_reset = TRUE;
    for(i=0; i<appl_max_vtimers; ++i)
    {  if(vttab[i].priority > 0)
          (void)pups_clearvitimer(vttab[i].name);
    }


    /*----------------------------------------------------------------------------------*/
    /* Search file table removing all FIFO's  and lockposts created by this application */
    /*----------------------------------------------------------------------------------*/

    if(appl_verbose == TRUE)
    {  (void)fprintf(stderr,"\n\n");
       (void)fflush(stderr);
    }


    /*-------------------------------*/
    /* Remove shadow files and links */
    /*-------------------------------*/

    for(i=0; i<appl_max_files; ++i)
    {

       /*--------------------------------------------*/
       /* Only remove files we have actually created */
       /*--------------------------------------------*/


       if(i < 3 || ftab[i].creator == TRUE)
       {  struct stat buf;


          /*-------------------------------*/
          /* Is this a named file or FIFO? */
          /*-------------------------------*/


          if(i < 3 || ftab[i].named == TRUE)
          {

             /*-------------------------------*/
             /* Is next file a FIFO or a pty? */
             /*-------------------------------*/

             (void)fstat(ftab[i].fdes,&buf);
             if(isatty(ftab[i].fdes) == 1 || S_ISFIFO(buf.st_mode) || strcmp(ftab[i].fshadow,"/dev/tty") == 0)
             {  char fname[SSIZE] = "";

                (void)sscanf(ftab[i].fname,"%s (%s",fname,fname);
                fname[strlen(fname)-1] ='\0';

                if(appl_verbose == TRUE )
                {  (void)strdate(date);


                   /*----------------------------*/
                   /* File is a lockpost symlink */
                   /*----------------------------*/

                   if(strcmp(ftab[i].fshadow,"/dev/tty") == 0)
                      (void)fprintf(stderr,"%s %s (%d@%s:%s): removing lockpost TTY symlink (%s)\n",
                                                 date,appl_name,appl_pid,appl_host,appl_owner,fname);


                   /*---------------*/
                   /* File is a PTY */
                   /*---------------*/

                   else if(isatty(ftab[i].fdes) == 1)
                      (void)fprintf(stderr,"%s %s (%d@%s:%s): removing PTY (%s)\n",
                                    date,appl_name,appl_pid,appl_host,appl_owner,fname);


                   /*----------------*/ 
                   /* File is a FIFO */
                   /*----------------*/ 

                   else
                      (void)fprintf(stderr,"%s %s (%d@%s:%s): removing FIFO (%s)\n",
                                    date,appl_name,appl_pid,appl_host,appl_owner,fname);

                   (void)fflush(stderr);
                }


                /*-----------------*/
                /* Remove the file */
                /*-----------------*/

                (void)unlink(ftab[i].fname);


                /*--------------------*/
                /* Remove shadow file */
                /*--------------------*/

                (void)unlink(ftab[i].fshadow);
             }


             /*--------------*/
             /* Regular file */
             /*--------------*/

             else
             {

                /*-----------------*/
                /* Remove the file */
                /*-----------------*/

                (void)unlink(ftab[i].fname);


                /*--------------------*/
                /* Remove shadow file */
                /*--------------------*/

                (void)unlink(ftab[i].fshadow);
             }
          }
       }
    }

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"\n%s %s (%d@%s:%s): %d application specific exit function(s) to execute\n\n",
                                                date,appl_name,appl_pid,appl_host,appl_owner,n_exit_funcs);
       (void)fflush(stderr);
    }


    /*-------------------*/
    /* Remove file table */
    /*-------------------*/

    ftab = (ftab_type *)pups_free((void *)ftab);


    /*--------------------*/
    /* Remove child table */
    /*--------------------*/

    chtab = (chtab_type *)pups_free((void *)chtab);


    #ifdef MAIL_SUPPORT
    /*------------------------*/
    /* Remove process mailbox */
    /*------------------------*/

    if(appl_mailable == TRUE)
    {  char rm_command[SSIZE] = "";

       (void)unlink(appl_mdir);
       (void)unlink(appl_mime_dir);

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): process mailbox removed\n",
                                date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }
    }
    #endif /* MAIL_SUPPORT */


    /*--------------------------------------------------------------*/
    /* If we are part of a pipeline and we have a problem terminate */
    /* the pipeline unless connected to a terminal                  */
    /*--------------------------------------------------------------*/

    if(appl_kill_pg == TRUE && exit_code != 0 && (isatty(0) == 0 || isatty(1) == 0))
    {   if(appl_verbose == TRUE)
        {  (void)strdate(date);
           (void)fprintf(stderr,"%s %s (%d@%s:%s): pipeline %d terminated (exit code %d for filter %s)\n",
                               date,appl_name,appl_pid,appl_host,appl_owner,getpgrp(),exit_code,appl_name);
           (void)fflush(stderr);
        }

        (void)kill(0,SIGTERM);
    }


    /*---------------------------------------*/
    /* Overlay xact command to seal pipeline */
    /*---------------------------------------*/

    if(exit_code == PUPS_PIPE_DETACH)
    {  if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%sd %s (%d@%s:%s): overlaying process with xcat filter (to seal pipeline)\n",
                                                                date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }

       (void)pups_execls("xcat");

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%sd %s (%d@%s:%s): failed to overlay xcat filter (and seal pipeline)\n",
                                                           date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }

       _exit(255);
    }


    /*---------------------------------------------------------------------*/
    /* If this is a tunnel process remove (stale) tunnel endpoints in /tmp */
    /* Remove body tunnel endpoint                                         */
    /*---------------------------------------------------------------------*/

    if(appl_tunnel_path != (char *)NULL && access(appl_tunnel_path,F_OK | R_OK | W_OK) != (-1))
    {  char tunnel_context_dir[SSIZE] = "";

       (void)unlink(appl_tunnel_path);


       /*--------------------------------*/
       /* Remove context tunnel endpoint */
       /*--------------------------------*/

       (void)snprintf(tunnel_context_dir,SSIZE,"%s.ckpt.dir",appl_tunnel_path);
       if(access(tunnel_context_dir,F_OK | R_OK | W_OK) != (-1))
       {  char rm_command[SSIZE] = "";

          (void)snprintf(rm_command,SSIZE,"rm -rf %s",tunnel_context_dir);
          (void)system(rm_command);
       }
    }


    /*------------------------------*/
    /* Restore controlling terminal */
    /*------------------------------*/

    if(getuid() == 0)
      (void)system("mktty");


    /*-----------------------------------*/
    /* Restart process if it is resident */
    /*-----------------------------------*/

    if(appl_resident == TRUE)
       longjmp(appl_resident_restart,1);



    /*--------------------------------------*/
    /* Restore signal handlers if deferred) */
    /*--------------------------------------*/

    else if(exit_code == PUPS_DEFER_EXIT)
    {  (void)pups_sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);
       (void)pups_malarm(vitimer_quantum);

       return(0);
    }


    /*-------------*/
    /* Really exit */
    /*-------------*/

    else
       _exit(exit_code);
}





/*---------------------------------------------------------------------------*/
/* Build a table of filesystem objects in the current directory matching key */
/*---------------------------------------------------------------------------*/

_PUBLIC char **pups_get_directory_entries(const char *dir_name,
                                          const char *key,
                                          int32_t    *key_entries,
                                          int32_t    *max_entries)

{   DIR           *dirp      = (DIR *)NULL;
    struct dirent *next_item = (struct dirent *)NULL;
    struct stat   buf;
    char          ** entries = (char **)NULL;

    if(dir_name    == (const char *)NULL  ||
       key         == (const char *)NULL  ||
       key_entries == (int32_t    *)NULL  ||
       max_entries == (int32_t    *)NULL   )
    {  pups_set_errno(EINVAL);
       return((char **)NULL);
    }


    /*-------------------------------------------------------------------------------*/
    /* Scan directory for leaves (normal files) stalks (other directories within the */
    /* current directory being scanned are simply ignored).                          */
    /*-------------------------------------------------------------------------------*/

    if((dirp = opendir(dir_name)) == (DIR *)NULL)
    {  pups_set_errno(EEXIST);
       return((char **)NULL);
    }

    (void)rewinddir(dirp);

    *key_entries = 0;
    *max_entries = ALLOC_QUANTUM;
    entries      = (char **)pups_calloc((*max_entries),sizeof(char *));

    while((next_item = readdir(dirp)) != (struct dirent *)NULL)
    {  

         if(strin(next_item->d_name,key) == TRUE)
         {  char pathname[SSIZE] = "";

            (void)snprintf(pathname,SSIZE,"%s/%s",dir_name,next_item->d_name); 
            (void)stat(pathname,&buf);

            if(buf.st_mode & S_IFREG || buf.st_mode & S_IFIFO || buf.st_mode & S_IFLNK)
            {  if((*key_entries)%ALLOC_QUANTUM == 0 && (*key_entries) > 0)
               {  (*max_entries) += ALLOC_QUANTUM;
                  entries        =  (char **)pups_realloc((void *)entries,(*max_entries)*sizeof(char *));
               }

               entries[(*key_entries)] = (char *)pups_malloc(SSIZE); 
               (void)strlcpy(entries[(*key_entries)],next_item->d_name,SSIZE);
               ++(*key_entries);
            }
         }
    }

    (void)closedir(dirp);

    pups_set_errno(OK);
    return(entries);
}



/*------------------------------------------------------------------------------------*/
/* Build a table of filesystem object in the current driectory matching multiple keys */
/*------------------------------------------------------------------------------------*/

_PRIVATE _BOOLEAN multimatch(char *target, int32_t keylist, const char **keys)

{   uint32_t i;

    if(target == (char *)NULL || keylist <= 0 || keys == (char **)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);

    for(i=0; i<keylist; ++i)
    {  if(strin(target,keys[i]) == FALSE)
          return(FALSE);
    }

    return(TRUE);
}


_PUBLIC char **pups_get_multikeyed_directory_entries(const char      *dir_name,
                                                     const  int32_t  keys,
                                                     const char      **keylist,
                                                     int32_t         *key_entries,
                                                     int32_t         *max_entries)

{   DIR           *dirp      = (DIR *)NULL;
    struct dirent *next_item = (struct dirent *)NULL;
    struct stat   buf;
    char          ** entries = (char **)NULL;

    if(dir_name    == (const char  *)NULL   ||
       keylist     == (const char **)NULL   ||
       key_entries == (int32_t     *)NULL   ||
       max_entries == (int32_t     *)NULL    )
    {  pups_set_errno(EINVAL);
       return((char **)NULL);
    }


    /*-------------------------------------------------------------------------------*/
    /* Scan directory for leaves (normal files) stalks (other directories within the */
    /* current directory being scanned are simply ignored).                          */
    /*-------------------------------------------------------------------------------*/

    if((dirp = opendir(dir_name)) == (DIR *)NULL)
    {  pups_set_errno(EEXIST);
       return((char **)NULL);
    }

    (void)rewinddir(dirp);

    *key_entries = 0;
    *max_entries = ALLOC_QUANTUM;
    entries = (char **)pups_calloc((*max_entries),sizeof(char *));

    while((next_item = readdir(dirp)) != (struct dirent *)NULL)
    {    if(multimatch(next_item->d_name,keys,keylist) == TRUE)
         {  char pathname[SSIZE] = "";

            (void)snprintf(pathname,SSIZE,"%s/%s",dir_name,next_item->d_name);
            (void)stat(pathname,&buf);

            if(buf.st_mode & S_IFREG || buf.st_mode & S_IFIFO || buf.st_mode & S_IFLNK)
            {  if((*key_entries)%ALLOC_QUANTUM == 0 && (*key_entries) > 0)
               {  (*max_entries) += ALLOC_QUANTUM;
                  entries        =  (char **)pups_realloc((void *)entries,(*max_entries)*sizeof(char *));
               }

               entries[(*key_entries)] = (char *)pups_malloc(SSIZE);
               (void)strlcpy(entries[(*key_entries)],next_item->d_name,SSIZE);
               ++(*key_entries);
            }
         }
    }

    closedir(dirp);

    pups_set_errno(OK);
    return(entries);
}




/*-----------------------------------------------------------*/
/* Extended read which restarts after interrupted system call */
/*------------------------------------------------------------*/

_PUBLIC ssize_t pups_sread(const des_t fildes, char *data, const size_t size)

{   int32_t ret;

    ssize_t eff_size,
            bytes_read = 0;

    if(data == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    eff_size = size;
    do {   ret = read(fildes,(char *)&data[bytes_read],eff_size);

           if(ret > 0 && errno != EINTR)
           {  bytes_read += ret;
              eff_size   -= ret;
           }

       } while(errno == EINTR);

    pups_set_errno(OK);
    return(bytes_read);
}




/*-------------------------------------------------------*/
/* Extended which restarts after interrupted system call */
/*-------------------------------------------------------*/
    
_PUBLIC ssize_t pups_swrite(const des_t fildes, const char *data, const size_t size)
 
{   int32_t ret;

    size_t  eff_size,
            bytes_written = 0;
 
    if(data == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    eff_size = size;
    do {   ret = write(fildes,(char *)&data[bytes_written],eff_size);
 
           if(ret > 0 && errno != EINTR)
           {  bytes_written += ret;
              eff_size -= bytes_written;
           }
 
       } while(errno == EINTR);

    pups_set_errno(OK); 
    return(bytes_written);    
}




/*-------------------------------------------*/
/* Broadcast SIGTERM signal to process group */
/*-------------------------------------------*/

_PRIVATE int32_t pg_leaders_term_handler(const int32_t signum)
 
{   _IMMORTAL _BOOLEAN entered = FALSE;

    void (*func)(void);

    if(entered == FALSE)
    {  entered = TRUE;

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): %s's process group terminated\n",
                            date,appl_name,appl_pid,appl_host,appl_owner,appl_name);
          (void)fflush(stderr);
       }
    }
    else

       /*--------------------------------------------------------------*/
       /* We get here as a result of sending TERM to our process group */
       /* of which we are (the leading!) member                        */
       /*--------------------------------------------------------------*/

       _exit(255 + signum);
 
    (void)kill(0,SIGTERM);


    /*-----------------------------*/
    /* Evict process (if resident) */
    /*-----------------------------*/

    if(appl_resident == TRUE)
    {  appl_resident = FALSE;
       (void)unlink(appl_argfifo);
    }

    pups_exit(255 + signum);
}
 
    
 
/*-------------------------------------------*/
/* Broadcast SIGSTOP signal to process group */
/*-------------------------------------------*/

_PRIVATE int32_t pg_leaders_stop_handler(const  int32_t signum)

{   _IMMORTAL _BOOLEAN entered = FALSE; 

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): process group stopped\n",
                           date,appl_name,appl_pid,appl_host,appl_owner);
       (void)fflush(stderr);
    }
                 
    if(entered == FALSE)
    {  (void)fprintf(stderr,"*** %s's process group stopped\n",appl_name);
       (void)fflush(stderr);


       /*----------------------------------------------------------*/
       /* Distribute signal to process group. Note that we have to */
       /* make sure that we are aware that the next SIGTSTP we get */
       /* is self generated                                        */
       /*----------------------------------------------------------*/

       entered = TRUE;
       (void)kill(0,SIGTSTP);
    }
    else
    {

       /*------------------------------------------------------------------*/
       /* This is the SIGTSTP we have propagated. We must signal ourselves */
       /* at this point                                                    */ 
       /*------------------------------------------------------------------*/

       entered = FALSE;
       (void)kill(appl_pid,SIGSTOP);
    }

    return(0);
}





/*-------------------------------------------*/
/* Broadcast SIGCONT signal to process group */
/*-------------------------------------------*/
    
_PRIVATE int32_t pg_leaders_cont_handler(const int32_t signum)                                              

{   _IMMORTAL _BOOLEAN continuing = FALSE;   


    /*------------------------------------------*/
    /* Make sure we don't catch our own signals */
    /*------------------------------------------*/

    if(continuing == TRUE)        
    {  continuing = FALSE;
       (void)pups_sighandle(SIGCONT,"pg_leaders_cont_handler",(void *)pg_leaders_cont_handler, (sigset_t *)NULL);
       return(-1);
    }                             

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): process group restarted\n",
                             date,appl_name,appl_pid,appl_host,appl_owner);
       (void)fflush(stderr);               
    }

    continuing = TRUE;
    appl_wait  = FALSE;

    (void)kill(0,SIGCONT);
    return(0);
}




/*---------------------------------------------------------------------*/
/* Handler for SIGCONT (for processes other than process group leader) */
/*---------------------------------------------------------------------*/

_PRIVATE int32_t pups_cont_handler(const int32_t signum)

{   appl_wait  = FALSE;
    return(psrp_cont_handler(signum));
}




/*-----------------*/
/* Process SIGTERM */
/*-----------------*/

_PRIVATE int32_t pups_exit_handler(const int32_t signum)

{   if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr, "%s %s (%d@%s:%s): in exit handler\n",
                      date,appl_name,appl_pid,appl_host,appl_owner); 
       (void)fflush(stderr);
    }


    /*-----------------------------*/
    /* Evict process (if resident) */
    /*-----------------------------*/

    if(appl_resident == TRUE)
    {  appl_resident = FALSE;
       (void)unlink(appl_argfifo);
    }

    pups_exit(signum);
}




/*-----------------------------------*/
/* Set re-entry point for SIGRESTART */
/*-----------------------------------*/


_PUBLIC int32_t pups_restart_enable(void)

{   int32_t ret;

    if(pups_abort_restart == FALSE)
    {  pups_abort_restart = TRUE;
       (void)pups_sighandle(SIGRESTART,"pups_restart_handler",(void *)&pups_restart_handler,(sigset_t *)NULL);
    }

    ret = sigsetjmp(pups_restart_buf,1);
    return(ret);
}




/*-------------------------------------*/
/* Reset re-entry point for SIGRESTART */
/*-------------------------------------*/

_PUBLIC int32_t pups_restart_disable(void)

{   pups_set_errno(OK);

    if(pups_abort_restart == TRUE)
    {  pups_abort_restart = FALSE;
       (void)pups_sighandle(SIGRESTART,"default",SIG_DFL,(sigset_t *)NULL);

        return(0);
    }

    pups_set_errno(EINVAL);
    return(-1);
}




/*------------------------*/
/* Handler for SIGRESTART */
/*------------------------*/

_PRIVATE int32_t pups_restart_handler(const int32_t signum)

{   siglongjmp(pups_restart_buf,1);
}




/*------------------------------------------*/
/* Search (shell) path directories for item */
/*------------------------------------------*/

_PUBLIC char *pups_search_path(const char *pathtype, const char *item)

{   int32_t ret;

    char cwd[SSIZE]       = "",
         next_path[SSIZE] = "",
         path_list[SSIZE] = "";

    _IMMORTAL char item_path[SSIZE] = "";


    if(pathtype == (const char *)NULL || item == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return((char *)NULL);
    }
    
    #ifdef UTILIB_DEBUG
    (void)fprintf(stderr,"UTILIB ITEM %s\n",item);
    (void)fflush(stderr);
    #endif /* UTILIB_DEBUG */


    /*-------------------------------------------*/
    /* Do we have absolute path to current item? */
    /*-------------------------------------------*/

    if(item[0] == '/' && access(item,F_OK | X_OK) != (-1))
    {

       #ifdef UTILIB_DEBUG
       (void)fprintf(stderr,"UTILIB %s: CWD RETURN PATH ABSOLUTE %s\n",item,item);
       (void)fflush(stderr);
       #endif /* UTILIB_DEBUG */

       pups_set_errno(OK);
       return(item);
    }


    /*------------------------------------------*/
    /* Is the command in the current directory? */
    /*------------------------------------------*/

    (void)getcwd(cwd,SSIZE);

    if(strncmp(item,"./",2) == 0)
       (void)snprintf(item_path,SSIZE,"%s/%s",cwd,(char *)&item[2]);
    else
       (void)snprintf(item_path,SSIZE,"%s/%s",cwd,item);


    #ifdef UTILIB_DEBUG
    (void)fprintf(stderr,"UTILIB ITEM PATH %s (access %d)\n",item_path,access(item_path,F_OK | X_OK));
    (void)fflush(stderr);
    #endif /* UTILIB_DEBUG */

    if(access(item_path,F_OK | X_OK) != (-1))
    {

       #ifdef UTILIB_DEBUG
       (void)fprintf(stderr,"UTILIB %s: CWD RETURN PATH LOCAL %s\n",item,item_path);
       (void)fflush(stderr);
       #endif /* UTILIB_DEBUG */

       pups_set_errno(OK);
       return(item_path);
    }


    /*---------------------------------------------------*/
    /* If pathtype is not a valid environmental variable */
    /* return immediately                                */
    /*---------------------------------------------------*/

    if(strccpy(path_list,(char *)getenv(pathtype)) == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return((char *)NULL);
    }

    (void)strext(':',(char *)NULL,(char *)NULL);


    do {    ret = strext(':',next_path,path_list);
            (void)snprintf(item_path,SSIZE,"%s/%s",next_path,item);

            #ifdef UTILIB_DEBUG
            (void)fprintf(stderr,"UTILIB PATH %s ACC: %d\n",item_path,access(item_path,F_OK | X_OK));
            (void)fflush(stderr);
            #endif /* UTILIB_DEBUG */

            if(access(item_path,F_OK | X_OK) != (-1))
            {  (void)strext(':',(char *)NULL,(char *)NULL);

               #ifdef UTILIB_DEBUG
               (void)fprintf(stderr,"UTILIB %s: RETURN PATH %s\n",item,item_path);
               (void)fflush(stderr);
               #endif /* UTILIB_DEBUG */

               pups_set_errno(OK);
               return(item_path);
           }
       } while(ret != END_STRING);

    #ifdef UTILIB_DEBUG
    (void)fprintf(stderr,"UTILIB %s: RETURN PATH NULL\n",item);
    (void)fflush(stderr);
    #endif /* UTILIB_DEBUG */

    pups_set_errno(ENOENT);
    return((char *)NULL);
}




/*------------------------------------------------------*/
/* Set process execution name (in kernel process table) */ 
/*------------------------------------------------------*/

_PUBLIC void pups_set_pen(char *argv[], const char *ben_name, const char *pen_name)

{   pid_t   pid;
    int32_t trys = 0;

    char exec_pathname   [SSIZE] = "",
         symlink_pathname[SSIZE] = "";

    if(argv     == (const char **)NULL  ||
       ben_name == (const char *) NULL  ||
       pen_name == (const char *) NULL   )
    {  pups_set_errno(EINVAL);
       return;
    }


    /*-------------------*/
    /* Get absolute path */
    /*-------------------*/

    if(strccpy(exec_pathname,pups_search_path("PATH",ben_name)) == (char *)NULL)
       pups_error("[set_pen] failed to resolve ben path");


    /*-------------------------------------------*/
    /* Make sure process calls itself by its pen */
    /* not its ben                               */
    /*-------------------------------------------*/


    (void)snprintf(symlink_pathname,SSIZE,"/tmp/%s",pen_name);


    /*------------------*/
    /* Try to make link */
    /*------------------*/

    while (symlink(exec_pathname,symlink_pathname) == (-1))
    {   if (trys == MAX_TRYS)
           pups_error("[pups_set_pen] symlink failed");
        else
        {  ++trys;
           (void)pups_usleep(100);
        }
    }


    #ifdef UTILIB_DEBUG
    (void)fprintf(stderr,"UTILIB PEN EXEC PATH %s\n",exec_pathname);
    (void)fflush(stderr);

    uint32_t i;
    for(i=0; i<255; ++i)
    {  if(argv[i] == (char *)NULL)
          break;
       else
       {  (void)fprintf(stderr,"UTILIB argv[%d]: %s\n",i,argv[i]);
          (void)fflush(stderr);
       }
    }
    #endif /* UTILIB_DEBUG */


    /*--------------------------------------*/
    /* Child side of fork wait for a second */
    /* and then remove (pen) symlink        */
    /*--------------------------------------*/

    if (fork() == 0)
    {
       /*-------------------------------*/
       /* Wait for parent to change its */
       /* name then delete symlink      */
       /*-------------------------------*/

       while(psrp_pname_to_pid(pen_name) != getppid())
	    pups_usleep(1000);

       (void)unlink(symlink_pathname);
       _exit(0);
    }


    /*----------------------------------*/
    /* Re-exec PRSP server woth new pen */
    /*----------------------------------*/

    (void)execv(symlink_pathname,argv);
    pups_error("[pups_set_pen] exec failed");
}




/*----------------------------------------------------------*/
/* Pread (pipe read) routine which blocks correctly on pipe */
/*----------------------------------------------------------*/

_PUBLIC ssize_t pups_pipe_read(const des_t fd, void *buf, const size_t count)
       
{   size_t cnt,
           r;                                         

    if(buf == (void *)NULL || fd < 0 || count <= 0)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    cnt = count;
    while (cnt > 0)                                                             
    {   

        if((r = read(fd,buf,cnt)) == 0)
           break;                          
 
        buf  = (void *)((long)buf + r);
        cnt -= r;                   
    }                                             

    pups_set_errno(OK);
    return(count - cnt);
}




/*-------------------------------*/
/* Block signal (PUPS compliant) */
/*-------------------------------*/

_PRIVATE _BOOLEAN in_pupshold = FALSE;

_PRIVATE sigset_t set,
                  old_set;

_PUBLIC int32_t pupsighold(const int32_t signum, const _BOOLEAN defer)

{   if(signum < 1 || signum > MAX_SIGS || (defer != FALSE && defer != TRUE))
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*-------------------------------------------------------------*/ 
    /* Signal handling is disabled if called from within pups_exit */
    /* psrp_handler or chan handler                                */
    /*-------------------------------------------------------------*/ 

    if(in_pups_exit == TRUE || in_vt_handler == TRUE || in_psrp_handler == TRUE || in_chan_handler == TRUE)
    {  pups_set_errno(OK);
       return(0);
    }

    if(in_pupshold == FALSE)
       (void)sigfillset(&set);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(pupsighold_cnt[signum] < 0)
       pups_error("[pupsighold] pupsighold/pupsigrelse count mismatch");

    if(pupsighold_cnt[signum] > 0)
    {  ++pupsighold_cnt[signum];
       (void)pups_sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&sigtab_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(OK);
       return(0);
    }
    else
       pupsighold_cnt[signum] = 1;

    if(defer == FALSE)
    {  sigset_t pending_set;

       (void)sigpending(&set);


       /*-----------------------------------------------------------------*/
       /* If this signal is pending we must guarantee its delivery before */
       /* sighold exits                                                   */
       /*-----------------------------------------------------------------*/

       if(defer == FALSE && sigismember(&set,signum) && sigtab[signum - 1].haddr != (uint64_t)SIG_IGN)
       {  

          /*----------------------------------------------------------------*/
          /* Unmask only the target signal (signum) which may be pending    */
          /* and awaits its delivery. We also need to unmask SIGABRT (which */
          /*  handles PSRP client-server interrupts, SIGARLM which is used  */
          /* by the homeostasis sub-systems, SIGTERM (which terminates the  */
          /* server in an orderly fashion, and SIGCONT (which is used by    */
          /* PSRP clients to test if the PSRP server is alive               */
          /*----------------------------------------------------------------*/

          (void)sigdelset  (&set,signum);

          if(appl_verbose == TRUE)
          {  (void)strdate(date);
	     if(signum < NSIG)
                (void)fprintf(stderr,"%s %s (%d@%s:%s): awaiting delivery of pending signal %d (%s)\n",
                                   date,appl_name,appl_pid,appl_host,appl_owner,signum,signame[signum]);
	     else
		fprintf(stderr,"%s %s (%d@%s:%s): awaiting delivery of pending signal %d\n",
				        date,appl_name,appl_pid,appl_host,appl_owner,signum);

             (void)fflush(stderr);
          }

          (void)sigsuspend(&set);

          if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): delivered\n",date,appl_name,appl_pid,appl_host,appl_owner);
             (void)fflush(stderr);
          }
       }
    }


    /*------------------------------------------------------*/
    /* Check to see if target signal (signum) is pending    */
    /* we must block it to prevent it being serviced before */
    /* sigsuspend is executed                               */
    /*------------------------------------------------------*/

    (void)sigdelset(&set,signum); 
    (void)pups_sigprocmask(SIG_SETMASK,&set,(sigset_t *)NULL);


    /*---------------------------------------*/
    /* Make sure that PUPS sigtab is updated */
    /*---------------------------------------*/

    (void)sigdelset(&sigtab[signum].sa_mask,signum); 

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}




/*-----------------------------------*/
/* Block signal set (PUPS compliant) */
/*-----------------------------------*/

_PUBLIC void pupshold(const int32_t sigs_to_hold)

{   sigset_t set;

    if(sigs_to_hold != PSRP_SIGS && sigs_to_hold != ALL_PUPS_SIGS && sigs_to_hold != ALL_SIGS)
    {  pups_set_errno(EINVAL);
       return;
    }


    /*-------------------------------------------------------------*/ 
    /* Signal handling is disabled if called from within pups_exit */
    /* psrp_handler or chan handler                                */
    /*-------------------------------------------------------------*/ 

    if(in_pups_exit == TRUE || in_vt_handler == TRUE || in_psrp_handler == TRUE || in_chan_handler == TRUE)
    {  pups_set_errno(OK);
       return; 
    }

    (void)sigfillset(&set);
    (void)pups_sigprocmask(SIG_BLOCK,&set,&old_set);
    in_pupshold = TRUE;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(pupshold_cnt < 0)
       pups_error("[pupshold] puphold/pupsrelse count mismatch"); 

    if(pupshold_cnt > 0)
    {  ++pupshold_cnt;
       (void)pups_sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);

       in_pupshold = FALSE;

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&sigtab_mutex);
       #endif /* PTHREAD_SUPPORT */

       return;
    }
    else
       pupshold_cnt = 1;

    (void)pups_sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);


    /*---------------------*/
    /* All PUPS/P3 signals */
    /*---------------------*/

    if(sigs_to_hold == ALL_PUPS_SIGS)
    {  (void)pupsighold(SIGALRM,  TRUE);
       (void)pupsighold(SIGCHLD,  TRUE);
       (void)pupsighold(SIGINIT,  TRUE);
       (void)pupsighold(SIGCHAN,  TRUE);
       (void)pupsighold(SIGPSRP,  TRUE);
       (void)pupsighold(SIGCLIENT,TRUE);
    }


    /*-----------------------*/
    /* PSRP protocol signals */
    /*-----------------------*/

    else if(sigs_to_hold == PSRP_SIGS || ignore_pups_signals == TRUE)
    {  (void)pupsighold(SIGINIT,  TRUE);
       (void)pupsighold(SIGCHAN,  TRUE);
       (void)pupsighold(SIGPSRP,  TRUE);
       (void)pupsighold(SIGCLIENT,TRUE);
    }

    in_pupshold = FALSE;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
}




/*-------------------------------------*/
/* unblock signal set (PUPS compliant) */
/*-------------------------------------*/

_PUBLIC void pupsrelse(const int32_t sigs_to_relse)

{   sigset_t set;

    if(sigs_to_relse != PSRP_SIGS && sigs_to_relse != ALL_PUPS_SIGS)
    {  pups_set_errno(EINVAL);
       return;
    }


    /*----------------------------------------------------------------*/ 
    /* If we handling a virtual timer or pupsighold() has been called */
    /* recursively and pupsighold_cnt > 0  don't unblock signal       */
    /*----------------------------------------------------------------*/

    if(in_pups_exit == TRUE || in_vt_handler == TRUE || in_psrp_handler == TRUE || in_chan_handler == TRUE)
    {  pups_set_errno(OK);
       return;
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    (void)sigfillset(&set);
    (void)pups_sigprocmask(SIG_SETMASK,&set,&old_set); 

    if(pupshold_cnt < 0)
       pups_error("[pupsrelse] pupshold/pupsrelse count mismatch");

    if(pupshold_cnt > 1)
    {  --pupshold_cnt;
       (void)pups_sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&sigtab_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(OK);
       return; 
    }
    else
       pupshold_cnt = 0;
    (void)pups_sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);

    if(sigs_to_relse == ALL_PUPS_SIGS || ignore_pups_signals == FALSE)
    {  (void)pupsigrelse(SIGINIT);
       (void)pupsigrelse(SIGCHAN);
       (void)pupsigrelse(SIGPSRP);
       (void)pupsigrelse(SIGCLIENT);
    }

    if(sigs_to_relse == ALL_PUPS_SIGS)
    {  (void)pupsigrelse(SIGALRM);
       (void)pupsigrelse(SIGCHLD);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
}




/*---------------------------------*/
/* Unblock signal (PUPS compliant) */
/*---------------------------------*/

_PUBLIC int32_t pupsigrelse(const int32_t signum)

{   sigset_t set;

    if(signum < 1 || signum > MAX_SIGS)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*-------------------------------------------------------------*/ 
    /* Signal handling is disabled if called from within pups_exit */
    /* psrp_handler or chan handler                                */
    /*-------------------------------------------------------------*/ 

    if(in_pups_exit == TRUE || in_vt_handler == TRUE || in_psrp_handler == TRUE || in_chan_handler == TRUE)
    {  pups_set_errno(OK);
       return(0);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    (void)sigfillset(&set);
    (void)pups_sigprocmask(SIG_SETMASK,&set,&old_set); 
    if(pupsighold_cnt[signum] < 0) 
    {  char errstr[SSIZE] = "";

       (void)snprintf(errstr,SSIZE,"[pupsigrelse] pupsighold/pupsigrelse count mismatch [%s:%d]: %d",signame[signum],signum,pupsighold_cnt[signum]);
       pups_error(errstr);
    }

    if(pupsighold_cnt[signum] > 1)
    {  --pupsighold_cnt[signum];
       (void)pups_sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&sigtab_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(OK);
       return(0);
    }
    else
       pupsighold_cnt[signum] = 0;
    (void)pups_sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);


    /*--------------------*/
    /* Unblock the signal */
    /*--------------------*/

    (void)sigemptyset(&set);
    (void)sigaddset(&set,signum);
    (void)pups_sigprocmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);


    /*----------------------------------*/
    /* Make sure PUPS sigtab is updated */
    /*----------------------------------*/

    (void)sigdelset(&sigtab[signum].sa_mask,signum);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}



/*-------------------------------------*/
/* PUPS compliant POSIX signal handler */
/*-------------------------------------*/

_PUBLIC int32_t pups_sighandle(const int32_t signum, const char *handler_name, const void *handler, const sigset_t *handler_mask)

{   int32_t ret;
    struct  sigaction action;

    if(signum       <  1                       ||
       signum       >  MAX_SIGS                ||
       handler_name == (const char *)    NULL   ) 
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(handler_mask == (const sigset_t *)NULL)
       (void)sigemptyset(&action.sa_mask);
    else
       action.sa_mask = *handler_mask;

    action.sa_handler       = handler;
    action.sa_flags         = 0; //SA_RESTART;

    sigtab[signum].haddr    = (uint64_t)handler;
    sigtab[signum].sa_mask  = action.sa_mask;
    sigtab[signum].sa_flags = action.sa_flags;

    if(handler_name != (char *)NULL)
      (void)strlcpy(sigtab[signum].hname,handler_name,SSIZE);

    ret = sigaction(signum,&action,(struct sigaction *)NULL);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(ret);
}



/*------------------------------*/
/* Save PUPS signal table entry */
/*------------------------------*/

_PUBLIC void pups_sigtabsave(const int32_t signum, sigtab_type *sigtab_entry)

{   
    if(signum < 1 || signum > MAX_SIGS || sigtab_entry == (const sigtab_type *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }
    else
       pups_set_errno(OK);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    *sigtab_entry = sigtab[signum - 1];

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */
}




/*-----------------------------------------------------------------*/
/* Restore PUPS signal table entry (and re-install signal handler) */
/*-----------------------------------------------------------------*/

_PUBLIC void pups_sigtabrestore(const int32_t signum, const sigtab_type *sigtab_entry)

{   if(signum < 1 || signum > MAX_SIGS || sigtab_entry == (const sigtab_type *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }
    else
       pups_set_errno(OK);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    (void)pups_sighandle(signum,sigtab_entry->hname,(void *)sigtab_entry->haddr,&sigtab_entry->sa_mask);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
}



/*---------------------------------------------------------------------*/
/* PUPS compliant process suspension (until specified signal recieved) */
/*---------------------------------------------------------------------*/

_PUBLIC int32_t pups_signalpause(const int32_t signum)

{   int32_t  ret;
    sigset_t set;

    if(signum < 1 || signum > MAX_SIGS)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    (void)sigfillset(&set);


    /*---------------------*/
    /* Signalm to wait for */
    /*---------------------*/

    (void)sigdelset(&set,signum);


    /*-------------------------------------------------*/
    /* Make sure standard PUPS signals are not blocked */
    /*-------------------------------------------------*/

    (void)sigdelset(&set,SIGINT);
    (void)sigdelset(&set,SIGTERM);
    (void)sigdelset(&set,SIGALIVE);
    (void)sigdelset(&set,SIGALRM);
    (void)sigdelset(&set,SIGCONT);

    if((ret = sigsuspend(&set)) == (-1))
       return(ret);

    (void)pups_malarm(vitimer_quantum);

    pups_set_errno(OK);
    return(ret);
}




/*---------------------------*/
/* PUPS compliant sigpending */
/*---------------------------*/

_PUBLIC _BOOLEAN pups_signalpending(const int32_t signum)

{   sigset_t set;

    if(signum < 1 || signum > MAX_SIGS)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       pups_set_errno(OK);

    (void)sigpending(&set);
    if(sigismember(&set,signum))
       return(TRUE);
    else
       return(FALSE);
}




/*---------------------------------*/
/* Initialise process signal table */
/*---------------------------------*/

_PRIVATE void initsigstatus(void)

{   uint32_t i;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=1; i<MAX_SIGS; ++i)
    {  if(i+1 == SIGTTIN   ||
          i+1 == SIGTTOU   ||
          i+1 == SIGIO     ||
          i+1 == SIGWINCH  ||
          i+1 == SIGURG    ||
          i+1 == SIGCONT   ||
          i+1 == SIGCHLD    )
       {  sigtab[i].haddr    = (uint64_t)SIG_IGN;
          (void)strlcpy(sigtab[i].hname,"ignored",SSIZE);
       }
       else
       {  sigtab[i].haddr    = (uint64_t)SIG_DFL;
          (void)strlcpy(sigtab[i].hname,"default",SSIZE);
       }

       sigtab[i].sa_flags = 0;
       (void)sigemptyset(&sigtab[i].sa_mask);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */
}



/*------------------------------*/
/* Display process signal table */
/*------------------------------*/

_PUBLIC void pups_show_sigstatus(const FILE *stream)

{   uint32_t i,
             handlers_installed = 0;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_sigstatus] attempt by non root thread to perform PUPS/P3 global utility operation");

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    (void)fprintf(stream,"\n\n    Signal handler status\n");
    (void)fprintf(stream,"    =====================\n\n");
    (void)fflush(stream);

    for(i=1; i<=MAX_SIGS; ++i)
    {  if(sigtab[i].haddr == (uint64_t)SIG_IGN)
       {  (void)fprintf(stream,"    %-16s (%02d): ignored\n",signame[i],i);
          (void)fflush(stream);

          ++handlers_installed;
       }
       else
       {  if(sigtab[i].haddr != (uint64_t)SIG_DFL)
          {  (void)fprintf(stream,
                     "    %-16s (%02d): re-entrant handler at %016lx virtual",
                              signame[i],i,(uint64_t)sigtab[i].haddr);

             if(sigtab[i].hname != (char *)NULL)
                (void)fprintf(stream," (%s)\n",sigtab[i].hname);
             else
                (void)fprintf(stream,"\n");

             (void)fflush(stream);

             ++handlers_installed;
          }
       }
    } 

    if(handlers_installed == 0)
       (void)fprintf(stream,"    Default signal handling (for all signals)\n\n");
    else if(handlers_installed == 1)
       (void)fprintf(stream,"\n\n    %04d signals, non default handler installed\n\n",MAX_SIGS);
    else
       (void)fprintf(stream,"\n\n    %04d signals, %04d non default handlers installed\n\n",MAX_SIGS,handlers_installed);
    (void)fflush(stream);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
}




/*------------------------------------------------------------*/
/* Display state of process signal mask (and signals pending) */
/*------------------------------------------------------------*/

_PUBLIC void pups_show_sigmaskstatus(const FILE *stream)

{   uint32_t i,
             b_cnt = 0,
             p_cnt = 0; 

    char blockstatus[SSIZE] = "",
         pendstatus[SSIZE]  = "";

    sigset_t set,
             old_set,
             pending_set;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_sigmaskstatus] attempt by non root thread to perform PUPS/P3 global utility operation");

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }


    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    (void)fprintf(stream,"\n\n    Signal mask status\n");
    (void)fprintf(stream,"    ==================\n\n");
    (void)fflush(stream);

    (void)sigemptyset(&set);
    (void)pups_sigprocmask(SIG_SETMASK,&set,&old_set);
    (void)sigpending(&pending_set);

    for(i=1; i<=MAX_SIGS; ++i)
    {  if(sigismember(&old_set,i))
       {  (void)strlcpy(blockstatus,"blocked",SSIZE);
          ++b_cnt;
       }
       else
          (void)strlcpy(blockstatus,"active",SSIZE);

       if(sigismember(&pending_set,i))
       {  (void)strlcpy(pendstatus,"pending",SSIZE);
          ++p_cnt;
       }
       else
          (void)strlcpy(pendstatus,"",SSIZE);

       (void)fprintf(stream,"    %-16s (%02d): %-16s  %-16s\n",signame[i],i+1,blockstatus,pendstatus);
       (void)fflush(stream);
    }

    (void)fprintf(stream,"\n\n    Blocked signals   : %04d (%d active)\n",b_cnt,MAX_SIGS - b_cnt);
    (void)fprintf(stream,"    Pending signals   : %04d\n\n",p_cnt);
    (void)fflush(stream);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
}




/*--------------------------------------*/
/* Convert signal name to signal number */
/*--------------------------------------*/

_PUBLIC int32_t pups_signametosigno(const char *name)

{   uint32_t i;
    char     tmpstr[SSIZE] = "";

    if(name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    else
       (void)strlcpy(tmpstr,name,SSIZE);

    for(i=0; i<strlen(tmpstr); ++i)
       tmpstr[i] = toupper(tmpstr[i]);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=1; i<=MAX_SIGS; ++i)
    {  if(strncmp(tmpstr,"SIG",3) == 0)
       {  if(signame[i] != (char *)NULL && strin(signame[i],&tmpstr[3]) == TRUE)
          {

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&sigtab_mutex);
             #endif /* PTHREAD_SUPPORT */

             pups_set_errno(OK);
             return((int32_t)i);
          }
       }
       else
       {  if(signame[i] != (char *)NULL && strin(signame[i],tmpstr) == TRUE)
          {

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&sigtab_mutex);
             #endif /* PTHREAD_SUPPORT */

             pups_set_errno(OK);
             return((int32_t)i);
          }
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ERANGE);
    return(-1);
}




/*--------------------------------------*/
/* Convert signal number to signal name */
/*--------------------------------------*/

_PUBLIC char *pups_signotosigname(const int32_t signum, char *name)

{    if(name == (char *)NULL || signum < 1 || signum >MAX_SIGS)
     {  pups_set_errno(EINVAL);
        return((char *)NULL);
     }
 
     #ifdef PTHREAD_SUPPORT
     (void)pthread_mutex_lock(&sigtab_mutex);
     #endif /* PTHREAD_SUPPORT */

     (void)strlcpy(name,signame[signum],SSIZE);

     #ifdef PTHREAD_SUPPORT
     (void)pthread_mutex_unlock(&sigtab_mutex);
     #endif /* PTHREAD_SUPPORT */

     pups_set_errno(OK);
     return(name);
}           




/*----------------------------------*/
/* Display detailed state of signal */
/*----------------------------------*/

_PUBLIC int32_t pups_show_siglstatus(const int32_t signum, const FILE *stream)

{   int32_t  i;
    _BOOLEAN maskbits_set = FALSE;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_sigstatus] attempt by non root thread to perform PUPS/P3 utility operation");

    if(stream == (FILE *)NULL || signum < 1 || signum > MAX_SIGS)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(sigtab[signum].haddr == (uint64_t)SIG_DFL)
    {  (void)fprintf(stream,"\n    %-16s [%04d]: default action\n\n",signame[signum],signum);
       (void)fflush(stream);

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&sigtab_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(OK);
       return(0);
    }
    else
    {  if(sigtab[signum-1].haddr == (uint64_t)SIG_IGN)
       {  (void)fprintf(stream,"\n    %-16s [%04d]: ignored\n\n",signame[signum],signum);
          (void)fflush(stream);

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&sigtab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return(0);
       }
    }

    (void)fprintf(stream,"\n\n    %-16s [%04d] handler information\n\n",signame[signum],signum);
    (void)fflush(stream);

    (void)fprintf(stream,"    Handler \"%-32s\" installed at %016lx virtual\n",sigtab[signum].hname,
                                                                     (uint64_t)sigtab[signum].haddr);
    (void)fflush(stream);

    if(sigtab[signum].sa_flags & SA_RESTART)
       (void)fprintf(stream,"    Handler will restart interrupted system calls on exit (BSD semantics)\n");
    else
       (void)fprintf(stream,"    Handler will NOT restart interrupted system calls on exit (POSIX semantics)\n");
    (void)fflush(stream);
 
    if(sigtab[signum].sa_flags & SA_RESETHAND)
       (void)fprintf(stream,"    Handler will be de-installed on exit\n\n");
    else
       (void)fprintf(stream,"    Handler will stay installed on exit\n\n");
    (void)fflush(stream);

    for(i=1; i<=MAX_SIGS; ++i)
    {  if(sigismember(&sigtab[signum].sa_mask,i+1) == 1)
       {  (void)fprintf(stream,"    Signal %-16s [%04d] is blocked while handler for %-16s [%04d] is running\n",
                                                                        signame[i+1],i+1,signame[signum],signum);
          (void)fflush(stream);

          maskbits_set = TRUE;
       }
    }

    if(maskbits_set == TRUE)
    {  (void)fprintf(stream,"\n\n");
       (void)fflush(stream);
    }
    else
    {  (void)fprintf(stream,"    Handler for %-16s [%04d] does not block any other signals while running\n\n",
                                                                                       signame[signum],signum);
       (void)fflush(stream);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&sigtab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}




/*--------------------------------------------------------*/
/* Execute non-local goto (as exit from a signal handler) */
/*--------------------------------------------------------*/

_PUBLIC int32_t pups_sigvector(const int32_t signum, const sigjmp_buf *vector)

{   if(signum > 0 && signum <= MAX_SIGS && vector != (sigjmp_buf *)NULL && jump_vector == TRUE)
    {  

       /*-----------------------------------------------*/
       /* Make sure that virtual timer gets rescheduled */
       /*-----------------------------------------------*/

       in_vt_handler = FALSE;

       (void)usleep(100);
       siglongjmp(*vector,signum);
    }

    pups_set_errno(EINVAL);
    return(-1);
}




/*----------------------------------------------------------*/
/* Check descriptor for data and/or exceptions using select */
/*----------------------------------------------------------*/

_PUBLIC int32_t pups_monitor(const des_t fdes, int32_t secs, const int32_t usecs)

{   fd_set read_set,
           write_set,
           excep_set;

    struct timeval timeout;

    if(fdes < 0 || fdes >= appl_max_files)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&excep_set);

    FD_SET(fdes,&read_set);
    FD_SET(fdes,&write_set);
    FD_SET(fdes,&excep_set);

    timeout.tv_sec  = secs;
    timeout.tv_usec = usecs;

    if(select(MAX_SIGS,&read_set,&write_set,&excep_set,&timeout) == (-1))
       return(-1);

    pups_set_errno(OK);
    if(FD_ISSET(fdes,&read_set))
       return(READ_DATA);

    if(FD_ISSET(fdes,&write_set))
       return(WRITE_DATA);

    if(FD_ISSET(fdes,&write_set))
       return(EXCEPTION_RAISED);

    return(TIMED_OUT);
}




/*----------------------------*/
/* Return current date string */
/*----------------------------*/

_PUBLIC void strdate(char *date)

{   time_t tval;

    if(date == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&date_mutex);
    #endif /* PHTREAD_SUPPORT */

    (void)pupshold(ALL_PUPS_SIGS);

    (void)time(&tval);
    (void)strlcpy(date,ctime(&tval),SSIZE);
    date[strlen(date) - 1] = '\0';

    (void)pupsrelse(ALL_PUPS_SIGS);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&date_mutex);
    #endif /* PHTREAD_SUPPORT */

    pups_set_errno(OK);
}




/*-------------------------------------------*/
/* Handler for vitual interval timer timeout */
/*-------------------------------------------*/
/*-----------------------------------*/
/* Initialise PUPS virtual timer set */
/*-----------------------------------*/

_PRIVATE void initvitimers(int32_t appl_max_vtimers)

{   uint32_t i;


    /*------------------------------------*/
    /* Allocate timer datastructure array */
    /*------------------------------------*/

    vttab = (vttab_type *)pups_calloc(appl_max_vtimers,sizeof(vttab_type));

    for(i=0; i<appl_max_vtimers; ++i)
    {  vttab[i].priority        = 0;
       vttab[i].mode            = VT_NONE;
       vttab[i].prescaler       = 0;
       vttab[i].interval_time   = 0;
       vttab[i].handler         = NULL;

       (void)strlcpy(vttab[i].name,        "",SSIZE);
       (void)strlcpy(vttab[i].handler_args,"",SSIZE);
    }

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): PUPS virtual timer system  int32_tialised (%d timers available)\n",
                                               date,appl_name,appl_pid,appl_host,appl_owner,appl_max_vtimers);
       (void)fflush(stderr);
    }
}




/*-------------------------------------------------------------------------------------------*/
/* Handler for PUPS virtual timer interrupt -- note if the handler for a given virtual timer */
/* does not return control to the signal handler all subsequent timers will have to wait a   */
/* further clock cycle before they are serviced                                              */
/*-------------------------------------------------------------------------------------------*/

_PRIVATE int32_t pups_vt_handler(const int32_t signum)

{   sigset_t   set;
    int32_t    i;
    time_t     tdum;
    char       handler_args[SSIZE];
    void       (*handler)(void *, char *args) = NULL;
    vttab_type tinfo;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_vt_handler] attempt by non root thread to perform PUPS/P3 utility operation");

    if(signum < 1 || signum > MAX_SIGS)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if(in_vt_handler == TRUE)
    {  pups_set_errno(OK);
       return(0);
    }

    in_vt_handler = TRUE;
    for(i=start_index; i<appl_max_vtimers; ++i)
    {  if(vttab[i].priority > 0)
       {  --vttab[i].prescaler;

          if(vttab[i].prescaler == 0) 
          {  if(vttab[i].mode == VT_ONESHOT)
             {  --active_v_timers;
                tinfo = vttab[i];

                vttab[i].priority      = 0;  
                vttab[i].mode          = VT_NONE;
                vttab[i].interval_time = 0;

                if(vttab[i].handler_args != (char *)NULL)
                   (void)strlcpy(handler_args,vttab[i].handler_args,SSIZE);

                handler                  = vttab[i].handler;
                vttab[i].handler         = NULL;

                (void)strlcpy(vttab[i].name,        "",SSIZE); 
                (void)strlcpy(vttab[i].handler_args,"",SSIZE);
             }
             else
             {  if(vttab[i].handler_args != (char *)NULL)
                   (void)strlcpy(handler_args,vttab[i].handler_args,SSIZE);

                vttab[i].prescaler       = vttab[i].interval_time;
                tinfo                    = vttab[i];
                handler                  = vttab[i].handler;
             }


             /*-------------------------------------------------*/
             /* Execute the handler - note we have to re-enable */ 
             /* SIGALRM when handler terminates                 */
             /*-------------------------------------------------*/

             if(i < appl_max_vtimers -  1)
                start_index = i;

             (*handler)((void *)&tinfo,handler_args);
          }
       }
    }

    start_index   = 0;

    if(active_v_timers > 0)
      (void)pups_malarm(vitimer_quantum);

    in_vt_handler = FALSE;

    pups_set_errno(OK);
    return(0);
}




/*---------------------------------------------------*/
/* Sort function for virtual interval timer priority */
/*---------------------------------------------------*/

_PRIVATE int32_t sort_priority(vttab_type *el_1, vttab_type *el_2)

{   if(el_1->priority > el_2->priority)
       return(-1);

    if(el_2->priority > el_1->priority)
       return(1);

    return(0);
}




/*------------------------------------------------------------------------------*/
/* Restart virtual timer system (this routine is usually called as a precaution */
/* after using dubious library functions e.g. CURSES which may silently reset   */
/* signal handlers in an undocumented fashion)                                  */
/* -----------------------------------------------------------------------------*/

_PUBLIC int32_t pups_vitrestart(void)

{

    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_vitrestart] attempt by non root thread to perform PUPS/P3 utility operation");

    if((in_setvitimer == TRUE  && active_v_timers == 0)    ||
       (in_setvitimer == FALSE && active_v_timers >  0)     )
    {  sigset_t vt_set;


       /*------------------------------------------------*/
       /* Initialise interval timer if this is the first */
       /* virtual timer activated                        */
       /* vt_set is the set of signals masked when the   */
       /* vt_handler function is running                 */
       /*------------------------------------------------*/

       (void)sigfillset(&vt_set);

       (void)sigdelset(&vt_set,SIGALRM);
       (void)sigdelset(&vt_set,SIGINT);
       (void)sigdelset(&vt_set,SIGTERM);
       (void)sigdelset(&vt_set,SIGCONT);

       (void)pups_sighandle(SIGALRM,"vt_handler",(void *)pups_vt_handler, &vt_set);

       in_vt_handler = FALSE;
       (void)pups_malarm(vitimer_quantum);
 
       pups_set_errno(OK);
       return(0);
    }
 
    pups_set_errno(EINVAL);
    return(-1);
}




/*----------------------------*/
/* Set virtual interval timer */
/*----------------------------*/
                                                              /*---------------------------------*/
_PUBLIC int32_t pups_setvitimer(const char     *tname      ,  /* Timer payload name              */
                                const  int32_t priority    ,  /* Priority of handler             */
                                const  int32_t mode        ,  /* Mode of timer                   */
                                const time_t  interval     ,  /* Timeout interval (microseconds) */
                                const char    *handler_args,  /* Handler arguments               */
                                const void    *handler     )  /* Handler called on timeout       */
                                                              /*---------------------------------*/

{   int32_t  i,
             t_index;

    time_t   tdum;
    sigset_t set;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_setvitimer] attempt by non root thread to perform PUPS/P3 global utility operation");

    if(tname        == (const char *)NULL  ||
       priority     <  0                   ||
       interval     <  0                   ||
       handler      == (const void *)NULL   )
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*----------------------------------------------------------*/
    /* If we have disabled VT services do not process this call */
    /*----------------------------------------------------------*/

    if(no_vt_services == TRUE)
    {  pups_set_errno(EACCES);
       return(-1);
    }

    if(priority < 0)
       pups_error("[setvitimer] invalid priority");



    /*-------------------------------------------*/
    /* Ignore timer signals while reseting timer */
    /*-------------------------------------------*/

    (void)pups_malarm(0);
    (void)sigemptyset(&set);
    (void)sigaddset(&set,SIGALRM);
    (void)pups_sigprocmask(SIG_BLOCK,&set,(sigset_t *)NULL);


    /*--------------------------------------------*/
    /* Get free timer slot and check to make sure */
    /* this timer is unique                       */
    /*--------------------------------------------*/

    for(i=0; i<appl_max_vtimers; ++i)
    {   if(vttab[i].name != (char *)NULL && strcmp(vttab[i].name,tname) == 0)
        {  (void)pups_sigprocmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);
           (void)pups_malarm(vitimer_quantum);

           pups_set_errno(EEXIST);
           return(-1);
        }

        if((void *)vttab[i].handler == (void *)NULL)
        {  t_index = i;
           goto free_timer;
        }
    }

    (void)pups_sigprocmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);
    (void)pups_malarm(vitimer_quantum);

    pups_set_errno(ENOSPC);
    return(-1);

free_timer:

    if(mode != VT_ONESHOT && mode != VT_CONTINUOUS)
    {  (void)pups_sigprocmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);
       (void)pups_malarm(vitimer_quantum);

       pups_set_errno(EINVAL);
       return(-1);
    }

    if(interval <= 0)
    {  (void)pups_sigprocmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);
       (void)pups_malarm(vitimer_quantum);

       pups_set_errno(ERANGE);
       return(-1);
    }


    /*------------------------------------------------*/
    /* Set up signal handler for virtual timer system */
    /*------------------------------------------------*/

    in_setvitimer = TRUE;
    (void)pups_vitrestart();
    in_setvitimer = FALSE;


    /*----------------------*/
    /* Set up virtual timer */
    /*----------------------*/

    if(vttab[t_index].priority == 0)
       ++active_v_timers;

    vttab[t_index].priority       = priority;


    /*-------------------------------------------------------------*/
    /* A negative interval means that we want the payload function */
    /* run immediately                                             */
    /*-------------------------------------------------------------*/

    if(interval > 0 && mode == VT_CONTINUOUS)
       vttab[t_index].prescaler   =  interval;
    else if(mode != VT_CONTINUOUS)
    {  (void)pups_sigprocmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);
       (void)pups_malarm(vitimer_quantum);

       pups_set_errno(EINVAL);
       return(-1);
    }

    vttab[t_index].interval_time  =  interval;
    vttab[t_index].mode           = mode;
    vttab[t_index].handler        = handler;

    if(handler_args != (char *)NULL)
       (void)strlcpy(vttab[t_index].handler_args,handler_args,SSIZE);

    (void)strlcpy(vttab[t_index].name,tname,SSIZE);


    /*-----------------------------------------------*/
    /* Order the timer structure by handler priority */
    /*-----------------------------------------------*/

    (void)qsort((void *)vttab,appl_max_vtimers,sizeof(vttab_type),(void *)sort_priority);


    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       if(mode == VT_CONTINUOUS)
          (void)fprintf(stderr,"%s %s (%d@%s:%s): PUPS virtual timer %d (%s) continuous timer, priority %d (interval %5.2F secs)\n",
                                                                                                                               date,
                                                                                                                          appl_name,
                                                                                                                           appl_pid,
                                                                                                                          appl_host,
                                                                                                                         appl_owner,
                                                                                                                            t_index,
                                                                                                                              tname,
                                                                                                                           priority,
                                                                                                           (FTYPE)interval / 1000.0);
       else
          (void)fprintf(stderr,"%s %s (%d@%s:%s): PUPS virtual timer %d (%s) one shot timer, priority %d (interval %5.2F secs)\n",
                                                                                                                             date,
                                                                                                                        appl_name,
                                                                                                                         appl_pid,
                                                                                                                        appl_host,
                                                                                                                       appl_owner,
                                                                                                                          t_index,
                                                                                                                            tname,
                                                                                                                         priority,
                                                                                                          (FTYPE)interval /1000.0);

       (void)fflush(stderr);
    }


    /*-----------------------------------------------------*/
    /* Re-enable interrupts from underlying hardware timer */
    /*-----------------------------------------------------*/

    (void)pups_sigprocmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);
    (void)pups_malarm(vitimer_quantum);

    pups_set_errno(OK);
    return(t_index);
}




/*------------------------------*/
/* Clear virtual interval timer */
/*------------------------------*/

_PUBLIC int32_t pups_clearvitimer(const char *tname)

{   uint32_t i;
    sigset_t set;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[clearvitimer] attempt by non root thread to perform PUPS/P3 global utility operation");

    if(tname == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    (void)pups_malarm(0);
    (void)sigemptyset(&set);
    (void)sigaddset(&set,SIGALRM);
    (void)pups_sigprocmask(SIG_BLOCK,&set,(sigset_t *)NULL);

    for(i=0; i<appl_max_vtimers; ++i)
    {  if(vttab[i].name != (char *)NULL && strcmp(vttab[i].name,tname) == 0)
       {  if(appl_verbose == TRUE)
          {  char date[SSIZE] = "";

             (void)strdate(date);

             if (strcmp(vttab[i].name,"") == 0)
                (void)fprintf(stderr,"%s %s (%d@%s:%s): PUPS virtual timer %d cleared\n",
                              date,appl_name,appl_pid,appl_host,appl_owner,i,vttab[i].name);
             else
                (void)fprintf(stderr,"%s %s (%d@%s:%s): PUPS virtual timer %d (%s) cleared\n",
                                 date,appl_name,appl_pid,appl_host,appl_owner,i,vttab[i].name);

             (void)fflush(stderr);
          }


          vttab[i].mode            = 0;
          vttab[i].priority        = 0;
          vttab[i].prescaler       = 0;
          vttab[i].interval_time   = 0;
          vttab[i].handler         = NULL;

          (void)strlcpy(vttab[i].name,        "",SSIZE);
          (void)strlcpy(vttab[i].handler_args,"",SSIZE);

          --active_v_timers;

          if(vt_no_reset == FALSE)
          {  

             /*---------------*/
             /* Active timers */
             /*---------------*/

             if(active_v_timers > 0)
             { 

               /*------------------------------------------------------------*/
               /* Note we have to re-enable the alarm after we have finished */
               /* clearing timers in case it expired in the critical section */
               /*------------------------------------------------------------*/

               (void)pups_malarm(vitimer_quantum);
               (void)pups_malarm(vitimer_quantum);
               (void)pups_sigprocmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);
             }


             /*------------------*/
             /* No active timers */
             /*------------------*/

             else
             {  (void)pups_malarm(0);
                (void)pups_sighandle(SIGALRM,"default",SIG_DFL, (sigset_t *)NULL);
                (void)pups_sigprocmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);
             }
          }
     
          pups_set_errno(OK);
          return(0);
       }
    }

    pups_set_errno(ESRCH);
    return(-1);
}

 


/*--------------------------------------*/
/* Display virtual interval timer table */                      
/*--------------------------------------*/

_PUBLIC void pups_show_vitimers(const FILE *stream)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_vitimers] attempt by non root thread to perform PUPS/P3 global utility operation");

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }

    if(active_v_timers == 0)
       (void)fprintf(stream,"\n\n    No virtual timers active (%d slots free)\n",appl_max_vtimers);
    else
    {  (void)fprintf(stream,"\n\n    Virtual timers active\n");
       (void)fprintf(stream,"    =====================\n\n");
       (void)fflush(stream);

        (void)fprintf(stream,"    Virtual timer quantum is %7.4F seconds\n\n",(FTYPE)vitimer_quantum*1.e-7);
        (void)fflush(stream);

        for(i=0; i<appl_max_vtimers; ++i)
        {  if(vttab[i].priority > 0)
           {  if(vttab[i].mode == VT_CONTINUOUS)
                 (void)fprintf(stream,
                               "    %04d: \"%-48s\" priority %02d, continuous mode, interval time %7.4F secs, prescaler %04d, (handler at %016lx virtual)\n",i,
                                                                                                                                                 vttab[i].name,
                                                                                                                                             vttab[i].priority,
                                                                                                                        (FTYPE)vttab[i].interval_time / 1000.0,
                                                                                                                                            vttab[i].prescaler,
                                                                                                                                    (uint64_t)vttab[i].handler);
              else
                (void)fprintf(stream,
                              "    %04d: \"%-48s\" priority %02d, oneshot mode, interval time %7.4F secs, prescaler %04d, (handler at %016lx virtual)\n",i,
                                                                                                                                             vttab[i].name,
                                                                                                                                         vttab[i].priority,
			                                                                                            (FTYPE)vttab[i].interval_time / 1000.0,
                                                                                                                                        vttab[i].prescaler,
                                                                                                                                (uint64_t)vttab[i].handler);
              (void)fflush(stream);
           }
        }

        if(active_v_timers > 1)
           (void)fprintf(stream,"\n\n    %04d virtual timers in use (%04d slots free)\n\n",active_v_timers,appl_max_vtimers - active_v_timers);
        else if(active_v_timers == 1)
           (void)fprintf(stream,"\n\n    %04d virtual timers registered (%04d slots free)\n\n",1,appl_max_vtimers - 1);
        else
           (void)fprintf(stream,"    No virtual timers in use ((%04d slots free)\n\n",appl_max_vtimers);
        (void)fflush(stream);
     }

     pups_set_errno(OK);
}





/*-------------------------------------------------------------*/
/* Routine to get (or test) write lock on open file descriptor */
/*-------------------------------------------------------------*/

_PUBLIC _BOOLEAN pups_get_fd_lock(const des_t fdes, const int32_t mode)

{   int32_t f_index;

    if(fdes < 0 || fdes >= appl_max_files || mode != GETLOCK && mode != TSTLOCK)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if(in_psrp_new_segment == FALSE)
    {  if((f_index = pups_get_ftab_index(fdes)) == (-1))
       {  pups_set_errno(EBADF);
          return(-1);
       }
    }
 
    if(mode == GETLOCK)
    {  (void)lockf(fdes,F_LOCK,0);
       pups_set_errno(OK);
       return(TRUE);
    }
    else
    {  if(lockf(fdes,F_TLOCK,0) < 0)
       {  pups_set_errno(OK); 
          return(FALSE);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(in_psrp_new_segment == FALSE)
       ftab[f_index].locked == TRUE;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(TRUE);
}




/*--------------------------------------------*/
/* Release write lock on open file descriptor */ 
/*--------------------------------------------*/

_PUBLIC int32_t pups_release_fd_lock(const des_t fd)

{   int32_t f_index;

    if((f_index = pups_get_ftab_index(fd)) == (-1))
    {  pups_set_errno(EBADF);
       return(-1);
    }

    (void)lockf(fd,F_ULOCK,0); 

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    ftab[f_index].locked = FALSE;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(TRUE);
}




/*------------------------------*/
/* Acquire lock on (named) file */
/*------------------------------*/

_PUBLIC _BOOLEAN pups_get_lock(const char *name, const int32_t mode)

{   des_t    fdes = (-1);
    _BOOLEAN ret;

    if(name == (const char *)NULL || (mode !=  GETLOCK && mode != TSTLOCK))
    { pups_set_errno(EINVAL);
      return(-1);
    }

    fdes = pups_open(name,2,LIVE);
    if((ret = pups_get_fd_lock(fdes,mode)) == (-1))
       return(-1);
    else
       fdes = pups_close(fdes);

    pups_set_errno(OK);
    return(0);
}




/*------------------------------*/
/* Release lock on a named file */
/*------------------------------*/

_PUBLIC _BOOLEAN pups_release_lock(const char *name)

{   des_t    fdes = (-1);
    _BOOLEAN ret;

    if(name == (const char *)NULL)
    { pups_set_errno(EINVAL);
      return(-1);
    }

    fdes = pups_open(name,2,LIVE);
    ret  = pups_release_fd_lock(fdes);
    fdes = pups_close(fdes);

    pups_set_errno(OK);
    return(ret);
}





/*----------------------------------------*/
/* Attach homeostat to a file table entry */
/*----------------------------------------*/

_PUBLIC int32_t pups_attach_homeostat(const des_t fd, const void *homeostat)

{   int32_t f_index;

    if(homeostat == (const void *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_attach_homeostat] attempt by non root thread to perform PUPS/P3 utility operation");

    if((f_index = pups_get_ftab_index(fd)) == (-1))
    {  pups_set_errno(EBADF);
       return(-1); 
    }
 
    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    ftab[f_index].homeostat = homeostat;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}




/*-------------------------------------------------*/
/* Detach homeostat function from file table entry */
/*-------------------------------------------------*/

_PUBLIC  int32_t pups_detach_homeostat(const des_t fd)

{    int32_t f_index;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_detach_homeostat] attempt by non root thread to perform PUPS/P3 utility operation");

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if((f_index = pups_get_ftab_index(fd)) == (-1)) 
    {

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(EBADF);
       return(-1); 
    }

    ftab[f_index].homeostat = (void *)NULL;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}




/*--------------------------------------------------------------------*/
/* Check for space of filesystem associated with open file descriptor */
/*--------------------------------------------------------------------*/

_PUBLIC _BOOLEAN pups_check_fs_space(const des_t fd)

{   int32_t            f_index;
    struct    statfs   fs_state;
    _IMMORTAL _BOOLEAN entered = FALSE;

    if((f_index = pups_get_ftab_index(fd)) == (-1))
    {  pups_set_errno(EBADF);
       return(-1);  
    }
 
    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*--------------------------------------------*/
    /* Find the state of the nominated filesystem */
    /*--------------------------------------------*/

    (void)fstatfs(fd,&fs_state);

    if(fs_state.f_bavail  < ftab[f_index].fs_blocks)
    {  if(appl_verbose == TRUE && entered == FALSE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): file system full (%s) [free:%ld, limit:%ld] (FSHP version %s)\n",
                                                                                                              date,
                                                                                                         appl_name,
                                                                                                          appl_pid,
                                                                                                         appl_host,
                                                                                                        appl_owner,
                                                                                               ftab[f_index].fname,
                                                                                                 fs_state.f_bavail,
                                                                                           ftab[f_index].fs_blocks,
                                                                                                      FSHP_VERSION);
          (void)fflush(stderr);
       }

       entered = TRUE;
    }
    else
    {  entered = FALSE;

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

      pups_set_errno(OK);
      return(TRUE);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(FALSE);
}




/*-------------------------------------------------------------*/
/* Set parameters for homeostatic file system space monitoring */
/*-------------------------------------------------------------*/

_PUBLIC int32_t pups_set_fs_hsm_parameters(const des_t fd, const int32_t fs_blocks, const char *fs_name)

{   int32_t f_index;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_set_fs_hsm_parameters] attempt by non root thread to perform PUPS/P3 utility operation");

    if(fs_blocks <= 0 || fs_name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if((f_index = pups_get_ftab_index(fd)) == (-1)) 
    {  pups_set_errno(EBADF);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    ftab[f_index].fs_blocks = fs_blocks;

    if(fs_name != (char *)NULL)
       (void)strlcpy(ftab[f_index].fs_name,fs_name,SSIZE); 
    else
       (void)strlcpy(ftab[f_index].fs_name,"<unknown>",SSIZE);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}




/*-----------------------------------------------------------------------------------*/
/* Homeostatic write check the default action is to wait until file system has space */
/* available. If mfunc is non-null this is executed (and implements a file           */
/*  migration scheme)                                                                */
/* ----------------------------------------------------------------------------------*/
                                                    /*------------------------------*/
_PUBLIC _BOOLEAN fs_write_blocked = FALSE;          /* If TRUE filesystem full      */
                                                    /*------------------------------*/

_PUBLIC int32_t pups_write_homeostat(const des_t fdes, int32_t (*mfunc)(int32_t))

{      

    int32_t iter = 0, 
            f_index;

    struct statfs  fs_buf;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_write_homeostat] attempt by non root thread to perform PUPS/P3 utility operation");

    if((void *)mfunc == (void *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    f_index = pups_get_ftab_index(fdes);

    if(fstatfs(fdes,&fs_buf) == (-1))
    {  pups_set_errno(EBADF);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*------------------------------------------------------------*/
    /* If a monitoring function is installed, execute it when the */
    /* file system filled to user defined limit                   */ 
    /*------------------------------------------------------------*/

    if(fs_buf.f_bavail < ftab[f_index].fs_blocks && (void *)mfunc != (void *)NULL)
    {  int32_t new_fdes;

       fs_write_blocked = TRUE;
       new_fdes         = (*mfunc)(fdes);
       fs_write_blocked = FALSE;

       pups_set_errno(OK);
       return(new_fdes);
    }
    else
    {  while(fs_buf.f_bavail < ftab[f_index].fs_blocks)
       {    

            /*------------------------------------------------------------------*/
            /* Stop rest of process group (usually processes in same pipeline   */ 
            /* as caller)                                                       */
            /* Note that we have to ignore SIGTSTOP as we have to remain active */
            /* to monitor the state of the filesystem we are writing to         */
            /*------------------------------------------------------------------*/

            if(iter == 0)
            {  (void)pups_sighandle(SIGTSTP,"ignored",SIG_IGN,(sigset_t *)NULL);
               (void)kill(0,SIGTSTP);
            }

            (void)sched_yield();

            if(iter == 0 && appl_verbose == TRUE)
            {  (void)strdate(date);
               (void)fprintf(stderr,"%s %s (%d@%s): file system full (%s:%d) [avail:%ld limit:%ld] (FSHP version %s)\n",
                                                                                                                   date,
                                                                                                              appl_name,
                                                                                                               appl_pid,
                                                                                                              appl_host,
                                                                                                    ftab[f_index].fname,
                                                                                                                   fdes,
                                                                                                        fs_buf.f_bavail,
                                                                                                ftab[f_index].fs_blocks,
                                                                                                           FSHP_VERSION);
               (void)fflush(stderr);

               fs_write_blocked = TRUE;
            }

            ++iter;
            (void)fstatfs(fdes, &fs_buf);
       }

       if(fs_write_blocked == TRUE)
       {  if(appl_verbose == TRUE)
          {  (void)strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): filesystem has space (%s:%d): continuing (FSHP version %s)\n",date,
                                                                                                              appl_name,
                                                                                                               appl_pid,
                                                                                                             appl_owner,
                                                                                                              appl_host,
                                                                                                    ftab[f_index].fname,
                                                                                                                   fdes,
                                                                                                           FSHP_VERSION);
             (void)fflush(stderr);
          }


          /*-----------------------------------------------*/
          /* Restore default signal processing for SIGSTOP */
          /* and restart pipeline                          */
          /*-----------------------------------------------*/

          fs_write_blocked = FALSE;
          (void)pups_sighandle(SIGTSTP,"default",SIG_DFL,(sigset_t *)NULL); 
          (void)kill(0,SIGCONT);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}




/*---------------------------*/
/* PUPS compliant sleep call */
/*---------------------------*/

_PUBLIC int32_t pups_sleep(const int32_t secs)

{   uint32_t i;

    if(secs <= 0)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*-----------------------------------------------------------------*/
    /* Use half seconds so we do not cause an overflow in nanosleep()  */
    /*-----------------------------------------------------------------*/

    for(i=0; i<2*secs; ++i)
       pups_usleep(500000); 

    pups_set_errno(OK);
    return(0);
}




/*--------------------------------------------------------*/
/* Microsecond alarm function (raises SIGALRM on timeout) */
/*--------------------------------------------------------*/

_PUBLIC int32_t pups_malarm(const uint64_t usecs)

{   struct   itimerval itimer;
    sigset_t set;
    int32_t  ret;

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&malarm_mutex);
    #endif /* PTHREAD_SUPPORT */

    itimer.it_interval.tv_sec  = 0;
    itimer.it_interval.tv_usec = 0;
    itimer.it_value.tv_sec     = 0;
    itimer.it_value.tv_usec    = usecs;

    ret = setitimer(ITIMER_REAL,&itimer,(struct itimerval *)NULL);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&malarm_mutex);
    #endif /* PTHREAD_SUPPORT */

    return(ret);
}




/*------------------------------------------------------------*/
/* Unhide the branch part of a file pathname (and its shadow) */
/*------------------------------------------------------------*/

_PRIVATE _BOOLEAN pups_unhide_pathnames(vttab_type *t_info, const char *fname, const char *fshadow)

{   char new_fname[SSIZE] = "";


    /*------------------*/
    /* Update file name */
    /*------------------*/

    if(fname[0] == '.') 
       (void)snprintf (new_fname,SSIZE,"%s",(char *)&fname[1]);
    else
       (void)strep(fname,  new_fname, "/.", "/");

    if(strcmp(fname,new_fname) != 0)
    {  char branch     [SSIZE] = "",
            new_branch [SSIZE] = "",
            leaf       [SSIZE] = "", 
            new_fshadow[SSIZE] = "";


       /*-----------------------*/
       /* Update file homeostat */
       /*-----------------------*/

       (void)strlcpy((char *)t_info->name,new_fname,SSIZE);


       /*-----------------*/
       /* Update filename */
       /*-----------------*/

       (void)strlcpy(fname,new_fname,SSIZE);


       /*-------------------------*/
       /* Update shadow file name */
       /*-------------------------*/

       (void)strbranch(fshadow,branch);
       (void)strleaf  (fshadow,leaf);

       if(branch[0] == '.')
          (void)snprintf (new_fshadow,SSIZE,"%s/%s",(char *)&branch[1],leaf);
       else
       {  (void)strep    (branch, new_branch, "/.", "/");
          (void)snprintf (new_fshadow,SSIZE,"%s/%s",new_branch,leaf);
       }

       (void)rename (fshadow,new_fshadow);
       (void)strlcpy(fshadow,new_fshadow,SSIZE);



       return(TRUE);  
    }

    return(FALSE); 
}




/*----------------------------*/
/* Homeostat for (open) files */
/*----------------------------*/

_PUBLIC _BOOLEAN default_fd_homeostat_action = FALSE;

_PUBLIC int32_t pups_default_fd_homeostat(void *t_info, const char *args)

{   int32_t f_index;

    char    fname[SSIZE]            = "",
            object_type[SSIZE]      = "",
            current_hostname[SSIZE] = "";

    struct  stat buf;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_default_fd_homeostat] attempt by non root thread to perform PUPS/P3 utility operation");

    if(t_info == (void *)      NULL   ||
       args   == (const char *)NULL    )
       pups_error("[pups_default_fd_homeostat] invalid arguments");

    if(sscanf(args,"%d",&f_index) != 1 || f_index < 0)
       pups_error("[pups_default_fd_homeostat] argument decode error");


    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*------------------------------------------------------------------------------*/
    /* If this is one of the stdio descriptors -- check to see if our process group */
    /* is currently in the foreground.                                              */
    /*------------------------------------------------------------------------------*/


    /*--------------------------------------------------*/
    /* Check whether we are currently a foreground task */
    /*--------------------------------------------------*/

    if(appl_fgnd == TRUE)
    {  

       /*---------------------------------------------------------*/
       /* Is this application about to go  int32_to the background?    */ 
       /* If so all unredirected stdio streams must be redirected */
       /* to the data sink                                        */
       /*---------------------------------------------------------*/

       if(appl_nodetach == FALSE && tcgetpgrp(appl_tty) != getpgrp())
       {   if(appl_verbose == TRUE)
           {  (void)strdate(date);
              (void)fprintf(stderr,"%s %s (%d@%s:%s): background (controlling tty [%s] detached)\n",
                                          date,appl_name,appl_pid,appl_host,appl_owner,appl_ttyname);
              (void)fflush(stderr);
           }

           if(isatty(0) == 1)
           {

              /*--------------------------------------*/
              /* Backgrounded channels are never live */ 
              /*--------------------------------------*/

              (void)fclose(stdin);
              stdin = fopen("/dev/null","r");


              /*-----------------------------------------------*/
              /* Make sure that we do not remove /dev/tty      */
              /* this is possible if application is runnig as  */
              /* suid root and we do not protect /dev/tty with */
              /* a guard link                                  */
              /*-----------------------------------------------*/

              if(getuid() == 0)
                 (void)link("/dev/tty","/dev/ttyprot");


              /*-----------------*/
              /* Remove the file */
              /*-----------------*/

              (void)unlink(ftab[0].fname);


              /*--------------------*/
              /* Remove shadow file */
              /*--------------------*/

              (void)unlink(ftab[0].fshadow);

              if(getuid() == 0)
                 (void)rename("/dev/ttyprot","/dev/tty");

              (void)strlcpy(ftab[0].fname,  "/dev/null",SSIZE);
              (void)strlcpy(ftab[0].fshadow,"/dev/null",SSIZE);
           }

           if(isatty(1) == 1)
           {  (void)fclose(stdout);
              stdout = fopen("/dev/null","w");


              /*-----------------------------------------------*/
              /* Make sure that we do not remove /dev/tty      */
              /* this is possible if application is runnig as  */
              /* suid root and we do not protect /dev/tty with */
              /* a guard link                                  */
              /*-----------------------------------------------*/

              if(getuid() == 0)
                 (void)link("/dev/tty","/dev/ttyprot");


              /*-----------------*/
              /* Remove the file */
              /*-----------------*/

              (void)unlink(ftab[1].fname);


              /*--------------------*/
              /* Remove shadow file */
              /*--------------------*/

              (void)unlink(ftab[1].fshadow);

              if(getuid() == 0)
                 (void)rename("/dev/ttyprot","/dev/tty");

              (void)strlcpy(ftab[1].fname,  "/dev/null",SSIZE);
              (void)strlcpy(ftab[1].fshadow,"/dev/null",SSIZE);
           }

           if(isatty(2) == 1)
           {  (void)fclose(stderr);
              stderr = fopen("/dev/null","w");


              /*-----------------------------------------------*/
              /* Make sure that we do not remove /dev/tty      */
              /* this is possible if application is runnig as  */
              /* suid root and we do not protect /dev/tty with */
              /* a guard link                                  */
              /*-----------------------------------------------*/

              if(getuid() == 0)
                 (void)link("/dev/tty","/dev/ttyprot");


              /*-----------------*/
              /* Remove the file */
              /*-----------------*/

              (void)unlink(ftab[2].fname);


              /*--------------------*/
              /* Remove shadow file */
              /*--------------------*/

              (void)unlink(ftab[2].fshadow);

              if(getuid() == 0)
                 (void)rename("/dev/ttyprot","/dev/tty");

              (void)strlcpy(ftab[2].fname,  "/dev/null",SSIZE);
              (void)strlcpy(ftab[2].fshadow,"/dev/null",SSIZE);
           }

           appl_fgnd = FALSE;
       }
    }
    else
    { 

       /*------------------------------------------------------------*/
       /* Are we now back in the foreground process group. If we are */
       /* re-attach stdio streams redirected to /dev/null to the     */
       /* controlling terminal                                       */
       /*------------------------------------------------------------*/ 

       if(tcgetpgrp(appl_tty) == getpgrp())
       {  


          if(strcmp(ftab[0].fname,"/dev/null") == 0) 
          {  (void)fclose(stdin); 
             stdin = fopen("/dev/tty","r");

             (void)snprintf(ftab[0].fname,SSIZE,"%s/pups.%s.%s.stdin.pst.%d:%d", appl_fifo_dir,appl_name,appl_host,appl_pid,appl_uid);
             (void)strlcpy(ftab[0].fshadow,"/dev/tty",SSIZE);


             /*------------------------------------------------------------------*/
             /* If we have been started in the background we will need to create */
             /* lockpost                                                         */
             /*------------------------------------------------------------------*/

             (void)symlink("/dev/tty",ftab[0].fname);
          }

          if(strcmp(ftab[1].fname,"/dev/null") == 0) 
          {  (void)fclose(stdout);
             stdout = fopen("/dev/tty","w");
             (void)snprintf(ftab[1].fname,SSIZE,"%s/pups.%s.%s.stdout.pst.%d:%d", appl_fifo_dir,appl_name,appl_host,appl_pid,appl_uid);
             (void)strlcpy(ftab[1].fshadow,"/dev/tty",SSIZE);


             /*------------------------------------------------------------------*/
             /* If we have been started in the background we will need to create */
             /* lockpost                                                         */
             /*------------------------------------------------------------------*/

             (void)symlink("/dev/tty",ftab[1].fname);
          }

          if(strcmp(ftab[2].fname,"/dev/null") == 0) 
          {  (void)fclose(stderr);
             stderr = fopen("/dev/tty","w");

             (void)snprintf(ftab[2].fname,SSIZE,"%s/pups.%s.%s.stderr.pst.%d:%d", appl_fifo_dir,appl_name,appl_host,appl_pid,appl_uid);
             (void)strlcpy(ftab[2].fshadow,"/dev/tty",SSIZE);


             /*------------------------------------------------------------------*/
             /* If we have been started in the background we will need to create */
             /* lockpost                                                         */
             /*------------------------------------------------------------------*/

             (void)symlink("/dev/tty",ftab[2].fname);
          }


          /*--------------------------------------------*/
          /* Try to get precise name of controlling tty */
          /*--------------------------------------------*/

          if(isatty(0) == 1)
             (void)strlcpy(appl_ttyname,ttyname(0),SSIZE);
          else if(isatty(1) == 1)
             (void)strlcpy(appl_ttyname,ttyname(1),SSIZE);
          else if(isatty(2) == 1)
             (void)strlcpy(appl_ttyname,ttyname(2),SSIZE);
          else
             (void)strlcpy(appl_ttyname,"/dev/tty",SSIZE);

          if(appl_verbose == TRUE && isatty(2) == 1)
          {  (void)strdate(date);
             if(started_detached == TRUE)
                (void)fprintf(stderr,"%s %s (%d@%s:%s): foreground (controlling tty [%s] attached)\n",
                                            date,appl_name,appl_pid,appl_host,appl_owner,appl_ttyname);
             else
                (void)fprintf(stderr,"%s %s (%d@%s:%s): foreground (controlling tty [%s] re-attached)\n",
                                               date,appl_name,appl_pid,appl_host,appl_owner,appl_ttyname);
             (void)fflush(stderr);
          }

          if(started_detached == TRUE)
             started_detached = FALSE;

          appl_fgnd = TRUE;
       }
    }

    /*------------------------------------------------------------------------------------*/
    /* Check for mode change on file system object -- if mode has been changed, change it */
    /* back.                                                                              */
    /*------------------------------------------------------------------------------------*/

    if(ftab[f_index].fname != (char *)NULL && ftab[f_index].fdes != (-1))
    {  (void)fstat(ftab[f_index].fdes,&buf);


       /*--------------------------------------------------------*/
       /* Find out what sort of file system object we have lost. */
       /*--------------------------------------------------------*/

       if(S_ISFIFO(buf.st_mode))
          (void)strlcpy(object_type,"FIFO",SSIZE);
       else if(isatty(ftab[f_index].fdes))
          (void)strlcpy(object_type,"tty",SSIZE);
       else if(ftab[f_index].pheap == TRUE)
          (void)strlcpy(object_type,"persistent heap",SSIZE);
       else
          (void)strlcpy(object_type,"regular file",SSIZE);


       /*-------------------------------------------------------------------------------*/
       /* Make sure protections on object are correct - if they have been changed by an */
       /* entity other than the objects owner - restore them now!                       */
       /*-------------------------------------------------------------------------------*/

       if(ftab[f_index].named == TRUE && ftab[f_index].st_mode & buf.st_mode != ftab[f_index].st_mode)
       {  if(appl_verbose == TRUE)
          {  strdate(date);
             (void)fprintf(stderr,"%s %s (%d@%s:%s): %s \"%s\" protection mode changed (from %o to %o) -- restoring\n",date,
                                                                                                                  appl_name,
                                                                                                                   appl_pid,
                                                                                                                  appl_host,
                                                                                                                 appl_owner,
                                                                                                                object_type,
                                                                                                        ftab[f_index].fname,
                                                                                          ftab[f_index].st_mode,buf.st_mode);
             (void)fflush(stderr); 
          }

          (void)chmod(ftab[f_index].fname,ftab[f_index].st_mode);
       }


       /*-----------------------------------------------------------------------------------*/
       /* If we are a regular file, check that we have enough space for continued writes on */
       /* our FS (given that we are open in write mode). If not we have two options, either */
       /* sit it out until space becomes available, or migrate to a file system which has   */
       /* sufficient space.                                                                 */
       /*-----------------------------------------------------------------------------------*/

       if(ftab[f_index].mode != 0 && S_ISREG(buf.st_mode) && pups_check_fs_space(ftab[f_index].fdes) == FALSE)
       {  int32_t iter = 0;


          /*--------------------------------*/
          /* Do we have a router installed? */
          /*--------------------------------*/

          if((void *)ftab[f_index].homeostat == (void *)NULL && S_ISREG(buf.st_mode))
          {   char tmpname[SSIZE] = "";

              if(appl_verbose == TRUE)
              {  strdate(date);
                 (void)fprintf(stderr,"%s %s (%d@%s:%s): filesystem full (%s:%d): waiting for space (FSHP version %s)\n",date,
                                                                                                                    appl_name,
                                                                                                                     appl_pid,
                                                                                                                   appl_owner,
                                                                                                                    appl_host,
                                                                                                          ftab[f_index].fname,
                                                                                                           ftab[f_index].fdes,
                                                                                                                 FSHP_VERSION);
                 (void)fflush(stderr);
              }

              while(pups_check_fs_space(ftab[f_index].fdes) == FALSE)
              {

                   /*-----------------------------------------------------------------*/
                   /* Stop rest of process group (usually processes in same pipeline  */
                   /* as caller)                                                      */
                   /* Note that we have to ignore SIGSTOP as we have to remain active */
                   /* to monitor the state of the filesystem we are writing to        */
                   /*-----------------------------------------------------------------*/

                   if(iter == 0)
                   {  (void)pups_sighandle(SIGTSTP,"ignored",SIG_IGN,(sigset_t *)NULL);
                      (void)kill(0,SIGTSTP);
                   }

                   (void)sched_yield();
              }

              if(appl_verbose == TRUE)
              {  (void)strdate(date);
                 (void)fprintf(stderr,"%s %s (%d@%s:%s): filesystem now has space (%s:%d): continuing (FSHP version %.2f)\n",date,
                                                                                                                        appl_name,
                                                                                                                         appl_pid,
                                                                                                                       appl_owner,
                                                                                                                        appl_host,
                                                                                                              ftab[f_index].fname,
                                                                                                               ftab[f_index].fdes,
                                                                                                                     FSHP_VERSION);
                 (void)fflush(stderr);
              }


              /*-----------------------------------------------*/
              /* Restore default signal processing for SIGSTOP */
              /* and restart pipeline                          */
              /*-----------------------------------------------*/

              (void)pups_sighandle(SIGTSTP,"default",SIG_DFL,(sigset_t *)NULL);
              (void)kill(0,SIGCONT);

              default_fd_homeostat_action = TRUE;
          }
          else
          { 

             /*------------------------------------------------------------------*/
             /* We have a router -- migrate the file to the file system          */
             /* selected by the router. If we do this we must leave a "marker"   */
             /* for other users of the file so they know about the migration and */
             /* can take appropriate action as a consequence of it. It is the    */
             /* part of the routers function to ensure that this marker is left  */ 
             /* and that it is adequately protected                              */
             /*------------------------------------------------------------------*/

             (*ftab[f_index].homeostat)(ftab[f_index].fdes,FILE_RELOCATE);
             default_fd_homeostat_action = FALSE;
          }
       }
    }


    /*--------------------------------------------------------------------------------*/
    /* Check to see if hostname has changed -- if it has terminate the process as all */
    /* the channelnames will be messed up (note we could restart here).               */
    /*--------------------------------------------------------------------------------*/

    (void)gethostname(current_hostname,SSIZE);


    #ifdef EXIT_IF_HOST_RENAMED 
    if(strcmp(appl_host,current_hostname) != 0)
    {

       /*-------------------------------------------------------------------*/
       /* If we have a host renamer installed update all the                */
       /* PSRP/PUPS files/FIFOS on this host to refect the changed hostname */
       /*-------------------------------------------------------------------*/

       if(ftab[f_index].homeostat == (void *)NULL)
       {  if(appl_verbose == TRUE)
             pups_error("[pups_default_fd_homeosat] host name has been changed -- exiting");
       }
       else
       {  (*ftab[f_index].homeostat)(ftab[f_index].fdes,HOST_RENAME);
          pups_default_fd_homeostat_action = FALSE;
       }
    }
    #endif /* EXIT_IF_HOST_RENAMED */


    /*----------------------------------------------------------------------------*/
    /* Check that we have not lost a file (as a named inode) -- if we have simply */
    /* re-create it.                                                              */
    /*----------------------------------------------------------------------------*/

                                                                     /*------------------------------------------------*/
    if(ftab[f_index].named                            == TRUE    &&  /* Named file (i.e homeostatically protected      */
       ftab[f_index].fdes                             != (-1)    &&  /* File is open                                   */
       access(ftab[f_index].fname,F_OK | R_OK | W_OK) == (-1)     )  /* File is assessible (via is 'given' name)       */
                                                                     /*------------------------------------------------*/
    {  _BOOLEAN file_found = FALSE;


       /*--------------------------------------------------*/
       /* If we have a (partially) hidden path - unhide it */
       /*--------------------------------------------------*/

       if(pups_unhide_pathnames((vttab_type *)t_info,ftab[f_index].fname,ftab[f_index].fshadow) == FALSE)
       {

          /*-------------------------------------------------------------------------------------*/
          /* If we have a homeostat to search for the lost object use it to try and relocate the */
          /* file we have just lost -- in the case of FIFO object the homeostat will simply      */
          /* need to create the object.                                                          */ 
          /*-------------------------------------------------------------------------------------*/

          if((void *)ftab[f_index].homeostat != (void *)NULL)
          {  file_found                  =  (*ftab[f_index].homeostat)(ftab[f_index].fdes,LOST_FILE_LOCATE);
             default_fd_homeostat_action = FALSE;
          }


          /*----------------------------------------------------------------------------------*/
          /* If we do not have a homeostat or it cannot find file default action is required. */
          /*----------------------------------------------------------------------------------*/

          ++ftab[f_index].lost_cnt;
          if(file_found == FALSE)
          {  if(appl_verbose == TRUE)
             {  (void)strdate(date);
                if(ftab[f_index].lost_cnt == 1)
                   (void)fprintf(stderr,"%s %s (%d@%s:%s): %s \"%s\" lost (once) -- recreating\n",date,
                                                                                             appl_name,
                                                                                              appl_pid,
                                                                                             appl_host,
                                                                                            appl_owner,
                                                                                           object_type,
                                                                                   ftab[f_index].fname);
                else
                   (void)fprintf(stderr,"%s %s (%d@%s:%s): %s \"%s\" lost (%d times) -- recreating\n",date,
                                                                                                 appl_name,
                                                                                                  appl_pid,
                                                                                                 appl_host,
                                                                                                appl_owner,
                                                                                               object_type,
                                                                                       ftab[f_index].fname,
                                                                                    ftab[f_index].lost_cnt);

                (void)fflush(stderr);
             }


             /*------------------------*/
             /* Recreate the resource. */
             /*------------------------*/

             if(strcmp(ftab[f_index].fshadow,"/dev/tty") == 0)
                (void)symlink(ftab[f_index].fshadow,ftab[f_index].fname);
             else
             {  if(link(ftab[f_index].fshadow,ftab[f_index].fname) == (-1))
                {  if((void *)ftab[f_index].homeostat != (void *)NULL)
                   {  if((*ftab[f_index].homeostat)(ftab[f_index].fdes,LOST_FILE_LOCATE) == FALSE)
                      {  (void)snprintf(errstr,SSIZE,"[pups_default_fd__homeostat] %s \"%s\" lost (and cannot be found) -- exiting",ftab[f_index].hname,ftab[f_index].fname);
                         pups_error(errstr);
                      }
                   }
                   else
                   {  (void)snprintf(errstr,SSIZE,"[pups_default_fd__homeostat] %s \"%s\" lost (and cannot be found) -- exiting",ftab[f_index].hname,ftab[f_index].fname);
                      pups_error(errstr);
                   }
                }
             }

             default_fd_homeostat_action = TRUE;
          }
       }
    }


    /*---------------------------------------------------------------------------*/
    /* Conversely - if we have lost the shadow (as a named inode) - recreate it. */
    /*---------------------------------------------------------------------------*/

    if(ftab[f_index].named == TRUE && ftab[f_index].fdes != (-1) && access(ftab[f_index].fshadow,F_OK | R_OK | W_OK) == (-1))
    {  if(appl_verbose == TRUE)
       {  strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): %s \"%s\" (shadow \"%s\") lost -- recreating\n",date,
                                                                                             appl_name,
                                                                                              appl_pid,
                                                                                             appl_host,
                                                                                            appl_owner,
                                                                                           object_type,
                                                                                   ftab[f_index].fname,
                                                                                 ftab[f_index].fshadow);
          (void)fflush(stderr);
       }

       /*------------------------*/
       /* Recreate the resource. */
       /*------------------------*/

       if(strcmp(ftab[f_index].fshadow,"/dev/tty") != 0)
          (void)link(ftab[f_index].fname,ftab[f_index].fshadow);

       default_fd_homeostat_action = TRUE;
    }


    /*-------------------------------------------------*/
    /* Check to see the /dev/tty has not been deleted. */
    /*-------------------------------------------------*/

    if(access("/dev/tty",F_OK | R_OK | W_OK) == (-1))
    {  if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): /dev/tty lost - recreating (mktty)\n",
                                           date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }

       (void)pups_system("mktty",shell,PUPS_ERROR_EXIT,(int32_t *)NULL);
    }


    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */


    pups_set_errno(OK);
    return(ftab[f_index].fdes);
}




/*-----------------------------------*/
/* Get file descriptor from filename */
/*-----------------------------------*/

_PUBLIC int32_t pups_fname2fdes(const char *fname)

{   uint32_t i;

    if(fname == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(strcmp(ftab[i].fname,fname) == 0)
       {

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return(ftab[i].fdes);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




/*-------------------------------*/
/* Get file stream from filename */
/*-------------------------------*/

_PUBLIC FILE *pups_fname2fstream(const char *fname)

{   uint32_t i;

    if(fname == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return((FILE *)NULL);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(strcmp(ftab[i].fname,fname) == 0 && ftab[i].stream != (FILE *)NULL)
       {

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return(ftab[i].stream);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return((FILE *)NULL);
}




/*-------------------------------------*/
/* Get file table index from file name */
/*-------------------------------------*/

_PUBLIC int32_t pups_fname2index(const char *fname)

{   uint32_t i;

    if(fname == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(strcmp(ftab[i].fname,fname) == 0)
       {  

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return((int32_t)i);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




/*-------------------------------------------*/
/* Get file table index from file descriptor */
/*-------------------------------------------*/

_PUBLIC int32_t pups_fdes2index(const des_t fdes)

{   uint32_t i;

    if(fdes < 0 || fdes >= appl_max_files)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<appl_max_files; ++i)
    {  if(ftab[i].fdes == fdes)
       {

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&ftab_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(OK);
          return((int32_t)i);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




/*--------------------------------*/
/* Create file without opening it */
/*--------------------------------*/

_PUBLIC int32_t pups_creat(const char *name, const int32_t mode)

{  int32_t ret;

   if(mode < 0 || name == (const char *)NULL)
   {  pups_set_errno(EINVAL);
      return(-1);
   }

   if(access(name,F_OK) == 0)
   {  pups_set_errno(EACCES);
      return(-1);
   }
   else
   {  if((ret = creat(name,mode)) == (-1))
      {

         /*-----------------------*/
         /* Failed to create file */
         /*-----------------------*/

         return(-1);
      }
   }

   /*--------------------------*/
   /* Make sure we have really */
   /* closed the file          */
   /*--------------------------*/
 
   while(close(ret) < 0)
        (void)pups_usleep(100);

   pups_set_errno(OK);
   return(0);
}




/*---------------------------------------------------------------------------*/
/* Change mode of file associated with open descriptor and update file table */
/*---------------------------------------------------------------------------*/

_PUBLIC int32_t pups_fchmod(const des_t fdes, const int32_t mode)

{   int32_t ret,
            f_index;

    if(mode < 0)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if((f_index = pups_get_ftab_index(fdes)) == (-1))
    {

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&ftab_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(EBADF);
       return(-1);
    }

    ftab[f_index].st_mode = mode; 
    ret                   = fchmod(fdes,mode);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(ret); 
}




/*-------------------------------------------------*/
/* Return string size (including terminating NULL) */
/*-------------------------------------------------*/

_PUBLIC int32_t pups_strlen(const char *s)

{    if(s != (const char *)NULL)
        return(strlen(s) + 1);
     else
        return(-1);
}





/*----------------------------------*/
/* Initialise tracked object system */
/*----------------------------------*/
/*-------------------------*/
/* Memory allocation table */
/*-------------------------*/

_PRIVATE matab_type matab;

_PRIVATE void tinit(void)

{   

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    matab.allocated = 0;
    matab.object    = (matab_object_type **)NULL;

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): Static heap object tracking enabled\n",
                                         date,appl_name,appl_pid,appl_host,appl_owner);
       (void)fflush(stderr);
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */
}




/*------------------------------*/
/* Memory block tracking malloc */
/*------------------------------*/

_PUBLIC void *pups_tmalloc(const size_t size, const char *object_name, const char *object_type)

{   uint32_t i,
             j;

    void *ptr = (void *)NULL;

    if(size        <= 0                   ||
       object_name == (const char *)NULL  ||
       object_type == (const char *)NULL   )
    {  pups_set_errno(EINVAL);
       return((void *)NULL);
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */


    /*----------------------*/
    /* Re-use existing slot */
    /*----------------------*/

    for(i=0; i<matab.allocated; ++i)
    {  if(matab.object[i] == (matab_object_type *)NULL)
       {  ptr                   = pups_malloc(size); 
          matab.object[i]       = (matab_object_type *)pups_malloc(sizeof(matab_object_type));
          matab.object[i]->ptr  = ptr;
          matab.object[i]->size = size;

          (void)strlcpy(matab.object[i]->name,object_name,SSIZE);  
          (void)strlcpy(matab.object[i]->type,object_type,SSIZE);

          #ifdef PTHREAD_SUPPPORT
          (void)pthread_mutex_unlock(&matab_mutex);
          #endif /* PTHREAD_SUPPPORT */

          pups_set_errno(OK);
          return(ptr);
       }
    }


    /*--------------*/
    /* Extend table */
    /*--------------*/

    i               =  matab.allocated;
    matab.allocated += ALLOC_QUANTUM;
    matab.object    = (matab_object_type **)pups_realloc((void *)matab.object,matab.allocated*sizeof(matab_object_type *));


    /*-------------------------------------------------*/
    /* Make sure the newly allocated pointers are NULL */
    /*-------------------------------------------------*/

    for(j=i; j<matab.allocated; ++j)
       matab.object[j] = (matab_object_type *)NULL;

    ptr                   = pups_malloc(size);          
    matab.object[i]       = (matab_object_type *)pups_malloc(sizeof(matab_object_type));
    matab.object[i]->ptr  = ptr;
    matab.object[i]->size = size;

    (void)strlcpy(matab.object[i]->name,object_name,SSIZE);    
    (void)strlcpy(matab.object[i]->type,object_type,SSIZE);

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_unlock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    pups_set_errno(OK);
    return(ptr);
}




/*------------------------------*/
/* Memory block tracking calloc */
/*------------------------------*/

_PUBLIC void *pups_tcalloc(const int32_t n_el, const size_t size, const char *object_name, const char *object_type)

{   int32_t i,
            j;

    void *ptr = (void *)NULL;

    if(n_el        <= 0                   ||
       size        <= 0                   ||
       object_name == (const char *)NULL  ||
       object_type == (const char *)NULL   )
    {  pups_set_errno(EINVAL);
       return((void *)NULL);
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    for(i=0; i<matab.allocated; ++i)
    {  if(matab.object[i] == (matab_object_type *)NULL)
       {  ptr                   = pups_calloc(n_el,size);
          matab.object[i]       = (matab_object_type *)pups_malloc(sizeof(matab_object_type));
          matab.object[i]->ptr  = ptr;
          matab.object[i]->size = size;
          matab.object[i]->n_el = n_el;

          (void)strlcpy(matab.object[i]->name,object_name,SSIZE);    
          (void)strlcpy(matab.object[i]->type,object_type,SSIZE);

          #ifdef PTHREAD_SUPPPORT
          (void)pthread_mutex_unlock(&matab_mutex);
          #endif /* PTHREAD_SUPPPORT */

          pups_set_errno(OK);
          return(ptr);
       }
    }

    i               = matab.allocated;
    matab.allocated += ALLOC_QUANTUM;
    matab.object    = (matab_object_type **)pups_realloc((void *)matab.object,matab.allocated*sizeof(matab_object_type *));


    /*-------------------------------------------------*/
    /* Make sure the newly allocated pointers are NULL */
    /*-------------------------------------------------*/

    for(j=i; j<matab.allocated; ++j)
       matab.object[j] = (matab_object_type *)NULL;

    ptr                   = pups_calloc(n_el,size);
    matab.object[i]       = (matab_object_type *)pups_malloc(sizeof(matab_object_type));
    matab.object[i]->ptr  = ptr;
    matab.object[i]->size = size;

    (void)strlcpy(matab.object[i]->name,object_name,SSIZE);
    (void)strlcpy(matab.object[i]->type,object_type,SSIZE);

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_unlock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    pups_set_errno(OK);
    return(ptr);
}




/*-------------------------------*/
/* Memory block tracking realloc */
/*-------------------------------*/

_PUBLIC void *pups_trealloc(void *ptr, const size_t size)

{    int32_t i;


    /*--------------------------------------------------------------*/
    /* If this is our first time we need to be processed by tmalloc */
    /*--------------------------------------------------------------*/

    if(ptr == (void *)NULL || size <= 0)
    {  pups_set_errno(EINVAL);
       return((void *)NULL);
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */


    /*-----------------------------------------------------------*/ 
    /* Find object to be reallocated in the block tracking table */
    /*-----------------------------------------------------------*/ 

    for(i=0; i<matab.allocated; ++i)
    {  if(matab.object[i] != (matab_object_type *)NULL && matab.object[i]->ptr == ptr)
       {  ptr                   = pups_realloc(ptr,size);
          matab.object[i]->size = size;
          matab.object[i]->ptr  = ptr;

          #ifdef PTHREAD_SUPPPORT
          (void)pthread_mutex_unlock(&matab_mutex);
          #endif /* PTHREAD_SUPPPORT */

          pups_set_errno(OK);
          return(ptr);
       }
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_unlock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    pups_set_errno(ERANGE);
    return((void *)NULL);
}




/*---------------------------*/
/* Allocate 2D tracked array */
/*---------------------------*/

_PUBLIC void **pups_taalloc(const pindex_t rows, const pindex_t cols, const psize_t size, const char *name, const char *type_name)

{   uint32_t i;

    char mem_name[SSIZE] = "";
    void *array_mem      = (void *)NULL;
    void **array         = (void **)NULL;

    if(rows      <= 0                   ||
       cols      <= 0                   ||
       size      <= 0                   ||
       name      == (const char *)NULL  ||
       type_name == (const char *)NULL   )
    {  pups_set_errno(EINVAL);
       return((void **)NULL);
    }


    /*-------------------------------------------*/
    /* First allocate memory for the whole array */
    /*-------------------------------------------*/

    (void)snprintf(mem_name,SSIZE,"%s[mem]",name);
    array_mem = (void *)pups_tmalloc(rows*cols*size,mem_name,"void *");


    /*------------------------------------*/
    /* Now row allocate the pointer array */
    /*------------------------------------*/

    array = (void **)pups_tmalloc(rows*sizeof(void **),name,type_name);


    /*---------------------------------------------------------*/
    /* Now allocate the columns addressed by the pointer array */
    /*---------------------------------------------------------*/

    for(i=0; i<rows; ++i)
    {  array[i]  = (void *)array_mem;
       array_mem = (void *)((uint64_t)array_mem + (uint64_t)(cols*size));
    }

    pups_set_errno(OK);
    return(array);
}




/*-----------------------*/
/* Free 2D tracked array */
/*-----------------------*/

_PUBLIC void **pups_tafree(const void **array)

{    if(array != (void **)NULL)
     {  

        /*--------------------------------------*/
        /* Free contiguous slab of array memory */
        /*--------------------------------------*/

        (void)pups_tfree((const void *)array[0]);


        /*-----------------------*/
        /* Free 2D mapping array */
        /*-----------------------*/

        (void)pups_tfree((const void **)array);

        pups_set_errno(OK);
     }
 
     pups_set_errno(EINVAL);
     return((void **)NULL);
}





/*----------------------------------------------------------*/
/* Get tracked object heap address from tracked object name */
/*----------------------------------------------------------*/

_PUBLIC void *pups_tnametoptr(const char *name)

{   uint32_t i;

    if(name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return((void *)NULL);
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    for(i=0; i<matab.allocated; ++i)
    {  if(matab.object[i] != (matab_object_type *)NULL && strcmp(matab.object[i]->name,name) == 0)
       {

          #ifdef PTHREAD_SUPPPORT
          (void)pthread_mutex_unlock(&matab_mutex);
          #endif /* PTHREAD_SUPPPORT */

          pups_set_errno(OK);
          return(matab.object[i]->ptr);
       }
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_unlock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    pups_set_errno(ESRCH);
    return((void *)NULL);
}




/*----------------------------------------------*/
/* Get pointer from partial tracked object name */
/*----------------------------------------------*/

_PUBLIC void *pups_tpartnametoptr(uint32_t *pos, const char *pname)

{   uint32_t i;

    if(pname == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return((void *)NULL);
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    for(i=(*pos); i<matab.allocated; ++i)
    {  if(matab.object[i] != (matab_object_type *)NULL && strin(matab.object[i]->name,pname) == TRUE)
       {  (*pos) = i + 1;

          #ifdef PTHREAD_SUPPPORT
          (void)pthread_mutex_unlock(&matab_mutex);
          #endif /* PTHREAD_SUPPPORT */

          pups_set_errno(OK);
          return(matab.object[i]->ptr);
       }
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_unlock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    pups_set_errno(ESRCH);
    return((void *)NULL);
}




/*-------------------------------*/
/* Get pointer from tracked type */
/*-------------------------------*/

_PUBLIC void *pups_ttypetoptr(uint32_t *pos, const char *type)

{   uint32_t i;

    if(type == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return((void *)NULL);
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    for(i=(*pos); i<matab.allocated; ++i)
    {  if(matab.object[i] != (matab_object_type *)NULL && strcmp(matab.object[i]->type,type) == 0)
       {  (*pos) = i + 1;

          #ifdef PTHREAD_SUPPPORT
          (void)pthread_mutex_unlock(&matab_mutex);
          #endif /* PTHREAD_SUPPPORT */

          pups_set_errno(OK);
          return(matab.object[i]->ptr);
       }
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_unlock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    pups_set_errno(ESRCH);
    return((void *)NULL);
}





/*----------------------------*/
/* Memory block tracking free */
/*----------------------------*/

_PUBLIC void *pups_tfree(const void *ptr)

{   uint32_t i;


    /*------------------------------------------------------*/
    /* Simply ignore a NULL object (but warn user about it) */
    /*------------------------------------------------------*/

    if(ptr == (const void *)NULL)
    {  pups_set_errno(EINVAL);
       return((void *)NULL);
    }
  
    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */


    /*-----------------------------*/
    /* Find the object to be freed */
    /*-----------------------------*/

    for(i=0; i<matab.allocated; ++i)
    {  if(matab.object[i] != (matab_object_type *)NULL && matab.object[i]->ptr == ptr)
       {

          /*----------------------------------------------*/
          /* Free the object returning memory to free pool*/
          /*----------------------------------------------*/

          (void)pups_free((void *)ptr);


          /*------------------------------------------------------------------*/
          /* Clear information about object (in PUPS memory allocation table) */
          /*------------------------------------------------------------------*/

          matab.object[i]->ptr  = (void *)NULL;
          matab.object[i]->size = 0;
          matab.object[i]->n_el = 0;

          (void)strlcpy(matab.object[i]->name,"",SSIZE);
          (void)strlcpy(matab.object[i]->type,"",SSIZE);


          /*------------------------------------------------------------------*/
          /* Free the slot in the table which contained the information about */
          /* freed object                                                     */
          /*------------------------------------------------------------------*/

          matab.object[i] = (matab_object_type *)pups_free((void *)matab.object[i]);

          #ifdef PTHREAD_SUPPPORT
          (void)pthread_mutex_unlock(&matab_mutex);
          #endif /* PTHREAD_SUPPPORT */

          pups_set_errno(OK);
          return((void *)NULL);
       }
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_unlock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    pups_set_errno(ESRCH);
    return((void *)NULL);
}




/*-------------------------*/
/* Display tracked objects */
/*-------------------------*/

_PUBLIC int32_t pups_tshowobject(const FILE *stream, const void *ptr)

{   uint32_t i;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_tobject] attempt by non root thread to perform PUPS/P3 utility operation");

    if(stream == (const FILE *)NULL || ptr == (const void *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if(matab.allocated == 0)
    {  pups_set_errno(EACCES);
       return(0);
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    for(i=0; i<matab.allocated; ++i)
    {  if(matab.object[i] != (matab_object_type *)NULL && matab.object[i]->ptr == ptr)
       {  (void)fprintf(stream,"    Object %-32s (type %-32s) size %016lx (at %016lx virtual)\n",matab.object[i]->name,
                                                                                                 matab.object[i]->type,
                                                                                                 matab.object[i]->size,
                                                                               (uint64_t         )matab.object[i]->ptr); 
          (void)fflush(stream);


          #ifdef PTHREAD_SUPPPORT
          (void)pthread_mutex_unlock(&matab_mutex);
          #endif /* PTHREAD_SUPPPORT */

          pups_set_errno(OK);
          return(0);
       }
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_unlock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */

    pups_set_errno(ESRCH);
    return(-1);
}




/*------------------------------------*/
/* Display all dynamic memory objects */
/*------------------------------------*/
/*--------------*/
/* Sort by size */
/*--------------*/

_PRIVATE int32_t compare_objects(matab_object_type **el_1, matab_object_type **el_2)


{   if(*el_1 == (matab_object_type *)NULL && *el_2 == (matab_object_type *)NULL) 
       return(0);

    if(*el_1 != (matab_object_type *)NULL && *el_2 == (matab_object_type *)NULL)
       return(-1);

    if(*el_1 == (matab_object_type *)NULL && *el_2 != (matab_object_type *)NULL)
       return(1);

    if((*el_1)->size < (*el_2)->size)
       return(1);

    if((*el_1)->size > (*el_2)->size)
       return(-1);

    return(0);
}




/*------------------------------*/
/* Display tracked object table */
/*------------------------------*/

_PUBLIC int32_t pups_tshowobjects(const FILE *stream, int32_t *display_objects)

{   uint32_t          i,
                      t_cnt = 0;

    uint64_t          heap_used        = 0;
    matab_object_type **ordered_object = (matab_object_type **)NULL;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_tshowobjects] attempt by non root thread to perform PUPS/P3 utility operation");

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    if(matab.allocated == 0)
    {  pups_set_errno(EACCES);
       return(0);
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_lock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */


    /*---------------------------------------------------------------------------------*/
    /* As we may inadvertantly re-order the object table while we are doing a critical */
    /* operation on it (e.g. using one of the tracked object allocator routines) make  */
    /* a copy of the table and re-order that                                           */
    /*---------------------------------------------------------------------------------*/

    ordered_object = (matab_object_type **)pups_calloc(matab.allocated,sizeof(matab_type *));
    for(i=0; i<matab.allocated; ++i)
    {  if(matab.object[i] != (matab_object_type *)NULL)
       {  ordered_object[t_cnt] = matab.object[i];
          ++t_cnt;
       }
    }

    #ifdef PTHREAD_SUPPPORT
    (void)pthread_mutex_unlock(&matab_mutex);
    #endif /* PTHREAD_SUPPPORT */


    /*------------------------------------------------------------------------------*/
    /* Order the objects so the largest ones are always towards the top of the list */
    /*------------------------------------------------------------------------------*/

    (void)qsort((void *)ordered_object,t_cnt,sizeof(matab_object_type *),(void *)compare_objects);

    (void)fprintf(stream,"\n\n    Heap objects for %s (%d@%s)\n\n",appl_name,getpid(),appl_host);
    (void)fflush(stream);

    if(*display_objects > t_cnt && display_objects != (int32_t *)NULL)
       *display_objects = t_cnt;

    for(i=0; i<display_objects; ++i)
    {  (void)fprintf(stream,"    %04d: Object %-32s (type %-32s) size %016lx (at %016lx virtual)\n",i,
                                                                              ordered_object[i]->name,
                                                                              ordered_object[i]->type,
                                                                              ordered_object[i]->size,
                                                            (uint64_t         )ordered_object[i]->ptr); 
       (void)fflush(stream);

       heap_used += ordered_object[i]->size;
    }


    if(t_cnt > 1)
       (void)fprintf(stream,"\n\n    %04d tracked objects [in %016lx bytes of heap] (%04d slots free)\n\n",t_cnt,heap_used,matab.allocated - t_cnt);
    else if(t_cnt == 1)
       (void)fprintf(stream,"\n\n    %04d tracked objects [in %016lx bytes of heap] (%04d slots free)\n\n",1,heap_used,matab.allocated - 1);
    else
       (void)fprintf(stream,"    no tracked objects (%04d slots free)\n\n",matab.allocated);
    (void)fflush(stream);

    (void)pups_free((void *)ordered_object);

    pups_set_errno(OK);
    return(0);
}




/*---------------------*/
/* PUPS fgets function */
/*---------------------*/

_PUBLIC char *pups_fgets(char *s, const size_t size, const FILE *stream)


{   size_t c_index;
    char   *ret = (char *)NULL;

    ret = fgets(s,size,stream);
    if((c_index = ch_pos(s,'\n')) != (-1))
       s[c_index] = '\0';

    return(ret);
}




/*-------------------------------------------------------------------------------*/
/* Check to see if line is a comment line (first non-space char is '#') or empty */
/*-------------------------------------------------------------------------------*/

_PUBLIC _BOOLEAN strcomment(const char *s)

{   size_t i;

    if(s == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*------------*/
    /* Empty line */
    /*------------*/

    if(s[0] =='\0')
    {  pups_set_errno(OK);
       return(TRUE);
    }

    /*-----------------------------*/
    /* Comment or white/tab space? */
    /*-----------------------------*/

    for(i=0; i<strlen(s); ++i)
    {  if(s[i] != ' ') 
       {  if(s[i] == '#')
          {  pups_set_errno(OK);
             return(TRUE);
          }
       }
    }

    pups_set_errno(OK);
    return(FALSE);
}




/*---------------------------------------------*/
/* Set all characters in string to NULL ('\0') */
/*---------------------------------------------*/

_PUBLIC void strclr(char *s)

{   size_t i;

    if(s == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return;
    }

    for(i=0; i<strlen(s); ++i)
       s[i] = '\0';

    pups_set_errno(OK);
}




/*-----------*/
/* Set errno */
/*-----------*/

_PUBLIC void pups_set_errno(const int32_t pups_errno)

{   errno = pups_errno;
}





/*-----------*/
/* Get errno */
/*-----------*/

_PUBLIC int32_t pups_get_errno(void)
 
{   return(errno);
}




/*----------------------------------------------------------------------------------------*/
/* Handler for SIGTTIN/SIGTTOU (automatically re-directs stdio to datasink if application */
/* moved into background interactively from a shell)                                      */
/*----------------------------------------------------------------------------------------*/

_PRIVATE int32_t fgio_handler(const int32_t signum)

{   
    if(signum < 1 || signum > MAX_SIGS)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    if(isatty(0) == 1)
    {  (void)fclose(stdin);
       stdin = fopen("/dev/null","r");
       (void)strlcpy(ftab[0].fname,"/dev/null",SSIZE);
       (void)strlcpy(ftab[0].fshadow,"/dev/null",SSIZE);
    }

    if(isatty(1) == 1)
    {  (void)fclose(stdout);
       stdout = fopen("/dev/null","w");
       (void)strlcpy(ftab[1].fname,"/dev/null",SSIZE);
       (void)strlcpy(ftab[1].fshadow,"/dev/null",SSIZE);
    }

    if(isatty(2) == 1)
    {  (void)fclose(stderr);
       stderr = fopen("/dev/null","w");
       (void)strlcpy(ftab[2].fname,"/dev/null",SSIZE);
       (void)strlcpy(ftab[2].fshadow,"/dev/null",SSIZE);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&ftab_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}




/*-------------------------------------------------------------------------*/
/*  Turn tty echoing off                                                   */
/*                                                                         */
/*  It is desirable to use this bit on systems that have it.               */
/*  The only bit of terminal state we want to twiddle is echoing, which is */
/*  done in software; there is no need to change the state of the terminal */
/*  hardware                                                               */
/*-------------------------------------------------------------------------*/

#ifndef TCSASOFT
#define TCSASOFT 0
#endif /* TCSASOFT */


_PUBLIC int32_t pups_tty_echoing_off(FILE *in, struct termios *s)

{   int32_t tty_changed;
    struct  termios t;


    /*-----------------------------------*/
    /* Turn echoing off if it is on now. */
    /*-----------------------------------*/

    if(tcgetattr (fileno (in), &t) == 0)
    {

       /*-------------------*/
       /* Save the old one. */
       /*-------------------*/

       *s = t;


       /*-----------------*/
       /* Tricky, tricky. */
       /*-----------------*/

       t.c_lflag &= ~(ECHO|ISIG);
       tty_changed = (tcsetattr (fileno (in), TCSAFLUSH|TCSASOFT, &t) == 0);
    }
    else
       tty_changed = 0;

    return(tty_changed);
}




/*---------------------*/
/* Turn tty echoing on */
/*---------------------*/

_PUBLIC void pups_tty_echoing_on(FILE *in, int32_t tty_changed, struct termios s)

{   

    /*-------------------------------*/
    /* Restore the original setting. */
    /*-------------------------------*/

    if(tty_changed)
       (void) tcsetattr (fileno (in), TCSAFLUSH|TCSASOFT, &s);
}





/*-------------------------------*.
/* Execute command on local host */
/*-------------------------------*/
                                                  /*--------------------------*/
_PUBLIC pid_t pups_lexec(const char      *shell,  /* Shell to execute comamnd */
                         const char    *command,  /* Command to be executed   */
                         const  int32_t    wait)  /* Wait action flag         */
                                                  /*--------------------------*/

{   pid_t   pid;
    int32_t status;

    _BOOLEAN obituary = FALSE;

    if(shell == (char *)NULL || command == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*-----------------------------------------------*/
    /* Fork thread of control to create new process. */
    /*-----------------------------------------------*/

    if(wait & PUPS_OBITUARY)
       obituary = TRUE;

    /*--------------------*/
    /* Child side of fork */
    /*--------------------*/

    if((pid = pups_fork(TRUE, obituary)) == 0)
    {  uint32_t i;


       /*-----------------------------------*/
       /* Set effective and real user i.d's */
       /*-----------------------------------*/

       (void)setreuid(getuid(),getuid());


       /*-------------------------*/
       /* Clear any pending alarm */
       /*-------------------------*/

       (void)pups_malarm(0);


       /*---------------------------------------*/
       /* Restore all signals to default states */
       /*---------------------------------------*/

       for(i=0; i<MAX_SIGS; ++i)
          (void)pups_sighandle(i,"default",SIG_DFL, (sigset_t *)NULL);


       /*---------------------------------------------------*/
       /* Extract argument list for execl call and overlay. */
       /*---------------------------------------------------*/

       if(execlp(shell,shell,"-c",command,(char *)0) == (-1))
       {  pups_set_errno(ENOEXEC);
          return(-1);
       }
    }


    /*----------------------------------------------------------------------------*/
    /* Parent side of fork - return childs pid if we are not going to wait for it */
    /* otherwise return the child's exit status.                                  */
    /*----------------------------------------------------------------------------*/

    else
    {

       (void)pups_set_child_name(pid,command);

       if(wait == PUPS_WAIT_FOR_CHILD)
       {  

          /*-----------------------------------*/
          /* Wait for the child we have forked */
          /*-----------------------------------*/

          if(waitpid(pid,&status,0) == (-1))
          {  pups_set_errno(ECHILD);
             return(-1);
          }

          pups_set_errno(OK);
          return(status);
       }

       pups_set_errno(OK);
       return(pid);
    }

    pups_set_errno(ENOSYS);
    return(-1);
}





#ifdef PSRP_AUTHENTICATE
/*-------------------------------*/
/* Check secure service password */
/*-------------------------------*/

_PUBLIC _BOOLEAN pups_check_appl_password(const char *password)

{

    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_check_appl_password] attempt by non root thread to perform PUPS/P3 utility operation");

    if(password == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    /*-----------------------------------------------*/
    /* Check arguments if any are NULL simply return */
    /*-----------------------------------------------*/

    if(strcmp(appl_password,"notset") == 0)
    {  pups_set_errno(OK);
       return(TRUE);
    }

    if(strncmp(password,appl_password,strlen(appl_password)-1) != 0)
    {  pups_set_errno(OK);
       return(FALSE);
    }

    pups_set_errno(OK);
    return(TRUE);
}




/*-------------------*/
/* Authenticate user */
/*-------------------*/

_PUBLIC _BOOLEAN pups_checkuser(const char *username, const char *password)

{   struct passwd  *pwent = (struct passwd *)NULL;

    char salt[2]                 = "",
         crypted_password[SSIZE] = "";


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_checkuser] attempt by non root thread to perform PUPS/P3 utility operation");

    if(username == (char *)NULL || password == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*--------------------------*/
    /* Authenticate remote user */
    /*--------------------------*/

    if((pwent = pups_getpwnam(username)) == (struct passwd *)NULL)
    {  pups_set_errno(ESRCH);
       return(FALSE);
    }


    /*-------------------------------------------*/
    /* Crypt password using first two characters */
    /* in password for "salt"                    */
    /* Crypt password using first two characters */
    /* in in encrypted password for "salt"       */
    /*-------------------------------------------*/

    salt[0] = pwent->pw_passwd[0];
    salt[1] = pwent->pw_passwd[1];
    (void)strlcpy((char *)&crypted_password,(char *)crypt(password,salt),SSIZE);

    if(strcmp(pwent->pw_passwd,crypted_password) != 0)
    {  pups_set_errno(EPERM);
       return(FALSE);
    }

    pups_set_errno(OK);
    return(TRUE);
}




/*-----------------------------------------*/
/* Check that application has password set */
/*-----------------------------------------*/

_PUBLIC _BOOLEAN pups_check_pass_set(void)

{   

    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_check_pass_set] attempt by non root thread to perform PUPS/P3 utility operation");

    pups_set_errno(OK);
    if(strcmp(appl_password,"notset") == 0)
       return(FALSE);

    return(TRUE);
}




/*---------------------*/
/* Authenticate a user */
/*---------------------*/

_PUBLIC _BOOLEAN pups_getpass(const char *password_banner)

{   char pwd_file_name[SSIZE]         = "",
         pups_password_banner[SSIZE]  = "";


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_getpass] attempt by non root thread to perform PUPS/P3 utility operation");

    if(password_banner == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*--------------------------------------------------------------*/
    /* Check to see if we actually need to obtain an authentication */
    /* token                                                        */
    /*--------------------------------------------------------------*/

    if(strcmp(appl_password,"notset") != 0)
    {  pups_set_errno(OK);
       return(TRUE);
    }


    /*--------------------------------------------------------------*/
    /* Search /tmp for an authorisation token for application owner */
    /*--------------------------------------------------------------*/

    (void)snprintf(pwd_file_name,SSIZE,"/tmp/auth.%s.tmp",appl_owner);
    if(access(pwd_file_name,F_OK | R_OK | W_OK) == (-1))
    {

       /*------------------------------------------------------*/
       /* Can we get to a controlling tty to talk to our user? */
       /*------------------------------------------------------*/

       if((isatty(0) == 1 || isatty(1) == 1 || isatty(2) == 1) && access(pwd_file_name,F_OK | R_OK | W_OK) == (-1))
       {  int32_t indes = (-1);


          /*-----------------------------------------------------------------*/
          /* Is stdin indirected? If so we need to inverse indirect it so we */
          /* can talk to our user                                            */ 
          /*-----------------------------------------------------------------*/

          if(isatty(0) == 0)
          {  int32_t ttydes = (-1);

             #ifdef UTILIB_DEBUG
             (void)fprintf(stderr,"pups_getpass [%d@%s] duplicating stdin (and saving intial descriptor)\n",getpid(),appl_host);
             (void)fflush(stderr);
             #endif /* UTILIB_DEBUG */

             indes  = dup(0);
          }

          (void)snprintf(pups_password_banner,SSIZE,"PUPS %s [%s] login: ",password_banner,appl_host);
          (void)strlcpy(appl_password,(char *)getpass(pups_password_banner),SSIZE);


          /*----------------------------*/
          /* Check authentication token */
          /*----------------------------*/

          if(pups_checkuser(appl_owner,appl_password) == FALSE)
          {  (void)fprintf(stderr,"pups_getpass [%d@%s]: permission denied\n",getpid(),appl_name);
             (void)fflush(stderr);

             pups_set_errno(EPERM);
             return(FALSE);
          }


          /*--------------------------------------------------*/
          /* If we have had to inverse indirect restore stdin */
          /* to its intial (e.g. indirected) state            */
          /*--------------------------------------------------*/

          if(indes != (-1))
          {  (void)dup2(indes,0);
             (void)close(indes);

             #ifdef DEBUG
             (void)fprintf(stderr,"rfifod [%d@%s]: initial stdin descriptor restored (after password entry)\n",getpid(),appl_host);
             (void)fflush(stderr);
             #endif /* DEBUG */
          }

          pups_set_errno(OK);
          return(TRUE);
       }
    }
    else
    {  FILE *pwd_stream = (FILE *)NULL; 


       /*---------------------------------------------*/
       /* Extract token from authorisation token file */
       /*---------------------------------------------*/

       pwd_stream = pups_fopen(pwd_file_name,"r",DEAD);

       (void)fscanf(pwd_stream,"%s",appl_password);
       (void)pups_fclose(pwd_stream);


       /*----------------------------*/
       /* Check authentication token */
       /*----------------------------*/

       if(pups_checkuser(appl_owner,appl_password) == FALSE)
       {  (void)fprintf(stderr,"%d@%s: permission denied\n",appl_pid,appl_name);
          (void)fflush(stderr);

          pups_set_errno(EPERM);
          return(FALSE);
       }

       pups_set_errno(OK);
       return(TRUE);
    }


    /*---------------------------------------*/
    /* We cannot get an authentication token */
    /*---------------------------------------*/

    pups_set_errno(EPERM);
    return(FALSE);
}
#endif /* PSRP_AUTHENTICATE */




/*----------------------------------------*/
/* Handler for SIGSEGV, SIGBUS and SIGFPE */
/*----------------------------------------*/

_PRIVATE int32_t segbusfpe_handler(const int32_t signum)

{    uint32_t i;

     if(signum < 1 || signum > MAX_SIGS)
     {  pups_set_errno(EINVAL);
        return(-1);
     }


     /*-----------------------------------------------*/
     /* Make sure that shadows are removed. The files */
     /* which they are shadowing are not removed as   */
     /* they may contain information which will aid   */
     /* debugging                                     */
     /*-----------------------------------------------*/

     #ifdef PTHREAD_SUPPORT
     (void)pthread_mutex_lock(&ftab_mutex);
     #endif /* PTHREAD_SUPPORT */

     for(i=0; i<appl_max_files; ++i)
     {  if(ftab[i].fshadow != (char *)NULL && strcmp(ftab[i].fshadow,"none") != 0)
           (void)unlink(ftab[i].fshadow);
     }

     #ifdef PTHREAD_SUPPORT
     (void)pthread_mutex_unlock(&ftab_mutex);
     #endif /* PTHREAD_SUPPORT */

     (void)strdate(date);
     if(appl_verbose == TRUE)
     {
        switch(signum)
        {    case SIGSEGV:  (void)fprintf(stderr,"%s %s (%d@%s:%s): SIGSEGV\n",date,appl_name,appl_pid,appl_host,appl_owner);

                            #ifdef SANITISE
                            (void)fprintf(stderr,"%s %s (%d@%s:%s): have you specified -fsanitize and not linked against debug PUPS/P3 libraries?\n");
                            (void)fflush(stderr);
                            #endif /* SANITIZE */

                            break;

             case SIGFPE:   (void)fprintf(stderr,"%s %s (%d@%s:%s): SIGFPE\n",date,appl_name,appl_pid,appl_host,appl_owner);
                            break;

             case SIGBUS:   (void)fprintf(stderr,"%s %s (%d@%s:%s): SIGBUS\n",date,appl_name,appl_pid,appl_host,appl_owner);
                            break;

             default:       break;

        }
    }

    (void)fflush(stderr);


    /*-------------------------------------------------*/
    /* If we are error trapping wait to be attached to */
    /* a debugger here                                 */
    /*-------------------------------------------------*/

    if(appl_etrap == TRUE)
    {

       /*----------------------------------------------------------*/
       /* SIGTERM must be released so we can terminate the process */
       /*----------------------------------------------------------*/

       (void)sigfillset(&set);

       (void)sigdelset(&set,SIGINT);
       (void)sigdelset(&set,SIGTERM);

       (void)pups_sigprocmask(SIG_SETMASK,&set,(sigset_t *)NULL);

       while(1)
            (void)pups_usleep(100);
    }

    pups_exit(255);
}




/*--------------------------------------------------------------------------------*/
/* PUPS kill function which notes state of signalled process. Processes which are */
/* terminated or stopped return appropriate error condition                       */
/*--------------------------------------------------------------------------------*/

_PUBLIC int32_t pups_statkill(const pid_t pid, const int32_t signum)

{   pid_t next_pid;

    char  line[SSIZE]       = "",
          procstatus[SSIZE] = "";

    FILE  *procstream = (FILE *)NULL;

    if(pid <= 0 || signum < 1 || signum > MAX_SIGS)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    (void)snprintf(procstatus,SSIZE,"/proc/%d/status",pid);
    if(access(procstatus,F_OK) == (-1))
       return(PUPS_TERMINATED);

    procstream = fopen(procstatus,"r");


    /*------------------------------------------------*/
    /* We have to search for the relevant line in the */
    /* status file as this file is kernel dependent   */
    /*------------------------------------------------*/

    do {    (void)fgets(line,SSIZE,procstream);
       } while(strin(line,"State") == FALSE);

    (void)fclose(procstream);

    if(strin(line,"T") == TRUE)
    {  pups_set_errno(OK);
       return(PUPS_STOPPED);
    }

    #ifdef UTILIB_DEBUG
    (void)fprintf(stderr,"UTILIB SENDING SIGNAL %d\n",signum);
    (void)fflush(stderr);
    #endif /* UTILIB_DEBUG */

    if(kill(pid,signum) == (-1))
    {  pups_set_errno(OK);
       return(PUPS_TERMINATED);
    }

    pups_set_errno(OK);
    return(PUPS_EXEC);
}




/*--------------------------*/
/* Apply extended file lock */
/*--------------------------*/

_PUBLIC int32_t pups_lockf(const des_t fdes, const int32_t cmd, const off_t size)

{   int32_t      ret = 0;
    struct flock lock;


    /*---------------*/
    /* Santity check */
    /*---------------*/

    if(fdes <  0                 ||
       fdes >= appl_max_files     ) 
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*--------------------*/
    /* Check lock command */
    /*--------------------*/
                                     /*-------------------------------------------*/
    if(cmd != PUPS_UNLOCK        &&  /* Unlock memory segment                     */
       cmd != PUPS_RDLOCK        &&  /* Blocking read lock on memory segment      */
       cmd != PUPS_TRY_RDLOCK    &&  /* Non blocking read lock on memebry segment */
       cmd != PUPS_WRLOCK        &&  /* Blocking write lock on memory segment     */
       cmd != PUPS_TRY_WRLOCK     )  /* Non blocking write lock on memory segment */
                                     /*-------------------------------------------*/
    {

       pups_set_errno(EINVAL);
       return(-1);
    }


    (void)pups_set_errno(OK);


    /*-------------------------------*/
    /* fcntl() lock type translation */
    /*-------------------------------*/

    if(cmd == PUPS_RDLOCK || cmd == PUPS_TRY_RDLOCK)
       lock.l_type = F_RDLCK;
    else if(cmd == PUPS_WRLOCK || cmd == PUPS_TRY_WRLOCK)
       lock.l_type = F_WRLCK;
    else
       lock.l_type = F_UNLCK;


    /*-----------------*/
    /* Segment details */
    /*-----------------*/

    lock.l_whence = SEEK_SET;
    lock.l_start  = 0L;


    /*-----------------------------------*/
    /* If segment size is zero just lock */
    /* address specified                 */
    /*-----------------------------------*/

    if(size == 0L)
       lock.l_len = sizeof(void *);
    else
       lock.l_len = size;
   

    /*--------------*/
    /* Acquire lock */
    /*--------------*/

    ret = fcntl(fdes,F_SETLK,&lock);
    return(ret);
}




/*--------------------------------*/
/* Get load average for localhost */
/*--------------------------------*/

_PUBLIC FTYPE pups_get_load_average(const int32_t which_load_average)

{   FILE *stream = (FILE *)NULL;

    FTYPE l_1,
          l_2,
          l_3;

    if((stream = fopen("/proc/loadavg","r")) == (FILE *)NULL)
    {  pups_set_errno(EBADF);
       return(-1.0);
    }

    (void)fscanf(stream,"%F%F%F",&l_1,&l_2,&l_3);
    (void)fclose(stream);

    switch(which_load_average)
    {     case LOAD_AVERAGE_1: pups_set_errno(OK);
                               return(l_1);

          case LOAD_AVERAGE_2: pups_set_errno(OK);
                               return(l_2);

          case LOAD_AVERAGE_3: pups_set_errno(OK);
                               return(l_3);

          default:   pups_set_errno(EINVAL);
                     return(-1.0);
    }

    pups_set_errno(OK);
}




/*---------------------------------------------------*/
/* Test if fdes is associated with a seekable device */
/*---------------------------------------------------*/

_PUBLIC _BOOLEAN pups_is_seekable(const des_t fdes)

{   if(pups_lseek(fdes,0,SEEK_CUR) == (-1))
       return(FALSE);

    pups_set_errno(OK);
    return(TRUE);
}



#ifdef CRIU_SUPPORT
/*------------------------*/
/* Checkpoint version tag */
/*------------------------*/

#define SSAVETAG    20

/*------------------------------------------------------------*/
/* Handler for SIGCHECK -- service an asynchronous checkpoint */
/*------------------------------------------------------------*/

_PRIVATE int32_t ssave_handler(const int32_t signum)

{    char buildStr[SSIZE]  = "",
          buildpath[SSIZE] = "",
          ssave_buildpath  = "",
          criu_cmd[SSIZE]  = "";

     struct timespec delay;

     FILE *stream        = (FILE *)NULL;


     #ifdef UTILIB_DEBUG
     (void)fprintf(stderr,"SIGCHECK: SAVING STATE AND EXITING (checkpoint %d)\n",appl_criu_ssaves);
     (void)fflush(stderr);
     #endif /* UTILIB_DEBUG */


     /*----------------------------------------------------*/
     /* Create checkpointing directory if it doesn't exist */
     /*----------------------------------------------------*/

     (void)snprintf(appl_criu_ssave_dir,SSIZE,"/tmp/pups.%s.%d.ssave",appl_name,appl_pid);
     if(access(appl_criu_ssave_dir,F_OK) == (-1))
     {  if(mkdir(appl_criu_ssave_dir,0700) == (-1))


           /*----------------------------------------------*/
           /* Error - cannot create state saving directory */
           /*----------------------------------------------*/

           return(-1);
     }

     /*-------------------------------------*/
     /* Run Criu command to save state      */
     /* it must run in background so it     */
     /* is not included in the process tree */
     /* of the caller                       */
     /*-------------------------------------*/


     /*--------------------------------*/
     /* Kill server after saving state */
     /*--------------------------------*/

     (void)snprintf(criu_cmd,SSIZE,"criu --log-file /dev/null --shell-job --link-remap dump -D %s -t %d &",appl_criu_ssave_dir,appl_pid);


     /*------------------------------------*/
     /* Build details for this application */
     /*------------------------------------*/

     (void)snprintf(buildStr,SSIZE,"%s %d %s",getenv("MACHTYPE"),SSAVETAG,__DATE__);


     /*------------------------------------*/
     /* Put build date of this application */
     /* in checkpoint directory so restart */
     /* can see if it is stale             */
     /*------------------------------------*/

     (void)snprintf(ssave_buildpath,SSIZE,"%s/build",appl_criu_ssave_dir);
     if(access(ssave_buildpath,F_OK) == (-1))
        (void)close(creat(ssave_buildpath,0600));

     stream = fopen(ssave_buildpath,"w");
     (void)fprintf(stream,"%s\n",buildStr);
     (void)fflush(stream);
     (void)fclose(stream);


     /*-----------------*/
     /* Take checkpoint */
     /*-----------------*/

     (void)system(criu_cmd);


     /*------------------------------------------*/
     /* This is a dummy system call - on restore */
     /* EINTR will be generated and the process  */
     /* will restart from here                   */
     /*------------------------------------------*/

     delay.tv_sec  = 60;
     delay.tv_nsec = 0;

     (void)nanosleep(&delay,(struct timespec *)NULL);

     #ifdef UTILIB_DEBUG
     (void)fprintf(stderr,"RESTART (checkpoint: %d)\n",appl_criu_ssaves);
     (void)fflush(stderr);
     #endif /* UTILIB_DEBUG */

     ++appl_criu_ssaves;


     /*-------------------*/
     /* Reschedule timers */
     /*-------------------*/

     (void)pups_malarm(vitimer_quantum);
     return(0);
}
#endif /* CRIU_SUPPORT */




/*---------------------------------------------------*/
/* Relay data to slaved process via a pair of FIFO's */
/*---------------------------------------------------*/

_PUBLIC int32_t pups_fifo_relay(const pid_t slave_pid, const int32_t in_fifo, const int32_t out_fifo)

{   ssize_t in_bytes_read,
            out_bytes_read;

    _BYTE buf[512] = "";

    do {   if(in_bytes_read = read(0,buf,512) < 0)
              return(-1);

           if(in_bytes_read > 0)
           {  if(write(out_fifo,buf,in_bytes_read) < 0)
                 return(-1);
           } 

           if(out_bytes_read = read(in_fifo,buf,512) < 0)
              return(-1);

           if(out_bytes_read > 0)
           {  if(write(1,buf,out_bytes_read) < 0)
                 return(-1);
           }
       } while(pups_statkill(slave_pid,SIGALIVE) == PUPS_EXEC);


    /*----------------------------------------*/
    /* Make sure master is completely emptied */
    /*----------------------------------------*/

    do {  if(out_bytes_read = read(in_fifo,buf,512) < 0)
             return(-1);

          if(out_bytes_read > 0)
          {  if(write(1,buf,out_bytes_read) < 0)
                return(-1);
             else 
                (void)tcdrain(1);
          }
       } while(out_bytes_read > 0);

    pups_set_errno(OK);
    return(0);
}




/*-------------------------------------------------------------------------------------*/
/* Detect window resize data (embedded in a buffer of data). If resizing data is found */
/* extract it and return it as a struct winsize                                        */
/*-------------------------------------------------------------------------------------*/
                                                               /*-----------------------------*/
_PRIVATE _BOOLEAN ws_extract(des_t              fdes,          /* File descriptor for ibuf    */
                             _BYTE              *buf,          /* Input buffer                */
                             size_t             *bytes_read,   /* Bytes in input buffer       */
                             _BYTE              *obuf_1,       /* Pre resize buffer           */
                             size_t             *bytes_out_1,  /* Number of pre resize bytes  */
                             _BYTE              *obuf_2,       /* Post resize buffer          */
                             size_t             *bytes_out_2,  /* Number of post resize bytes */
                             struct winsize     *ws)           /* Winsize structure           */
                                                               /*-----------------------------*/

{   uint32_t i,
             j,
             cnt = 0;

    _BYTE row_bytes[2] = "",
          col_bytes[2] = "";

    _BOOLEAN fill_obuf_2 = FALSE,
             buf_spill   = FALSE;


    /*---------------------------------------------------------*/
    /* Search for '255' which indicates that we need to resize */
    /* window associated with in_buf                           */
    /*---------------------------------------------------------*/

    for(i=0; i<bytes_read; ++i)
    {  if(buf[i] == WINRESIZE_PENDING)
       {  fill_obuf_2  = TRUE;
          *bytes_out_1 = cnt;
          cnt          = 0;


          /*----------------------------------------------*/
          /* Read next 4 bytes (which contain row and col */
          /* co-ordinates for the resize.                 */
          /* If we have less than four bytes read in the  */
          /* extra bytes.                                 */
          /*----------------------------------------------*/

          ++i;
          for(j=0; j<2; ++j)
          {  if(i+j < bytes_read)
                row_bytes[j] = buf[i++];
             else
             {  /* Buffer spill - read missing bytes */
                buf_spill = TRUE;

                (void)fcntl(0,F_SETFL, fcntl(0,F_GETFL,0) & ~O_NONBLOCK);
                (void)read(fdes,&row_bytes[j],1);
                (void)fcntl(0,F_SETFL, fcntl(0,F_GETFL,0) | O_NONBLOCK);
             }
          }

          for(j=0; j<2; ++j)
          {  if(i+j < bytes_read)
                col_bytes[j] = buf[i++];
             else
             {  /* Buffer spill - read missing bytes */
                buf_spill = TRUE;

                (void)fcntl(0,F_SETFL,fcntl(0,F_GETFL,0) & ~O_NONBLOCK);
                (void)read(fdes,&col_bytes[j],1);
                (void)fcntl(0,F_SETFL, fcntl(0,F_GETFL,0) | O_NONBLOCK);
             }
          }

          ws->ws_row = row_bytes[0] + 256*row_bytes[1];
          ws->ws_col = col_bytes[0] + 256*col_bytes[1];
       }

       if(fill_obuf_2 == TRUE)
       {  if(buf_spill == FALSE)
             obuf_2[cnt++] = buf[i];
       }
       else
          obuf_1[cnt++] = buf[i];
    }

    *bytes_out_2 = cnt;

    if(fill_obuf_2 == TRUE || buf_spill == TRUE)
       return(TRUE);
    else
       return(FALSE);
}




/*---------------------------------------------------------*/
/* Wait for file to be unlinked before returning to caller */
/*---------------------------------------------------------*/

_PUBLIC int32_t pups_unlink(const char *linkname)

{   if(linkname == (char *)NULL || unlink(linkname) == (-1))
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    while(access(linkname,F_OK | R_OK | W_OK) == 0)
         (void)pups_usleep(100);

    pups_set_errno(OK);
    return(0);
}




/*-----------------------------------*/
/* Clear virtual timer datastructure */
/*-----------------------------------*/

_PUBLIC int32_t pups_clear_vitimer(const uint32_t t_index)

{  

    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_clear_vitimer] attempt by non root thread to perform PUPS/P3 global utility operation");


    if(index > appl_max_vtimers)
    {  pups_set_errno(ERANGE);
       return(-1);
    } 

    vttab[t_index].priority        = 0;
    vttab[t_index].mode            = VT_NONE;
    vttab[t_index].prescaler       = 0;
    vttab[t_index].interval_time   = 0;
    vttab[t_index].handler         = NULL;

    (void)strlcpy(vttab[t_index].name,        "",SSIZE);
    (void)strlcpy(vttab[t_index].handler_args,"",SSIZE);

    pups_set_errno(OK);
    return(0);
}




/*-------------*/
/* PUPS usleep */
/*-------------*/

_PUBLIC int32_t pups_usleep(const uint64_t useconds)

{   uint32_t i;

    struct timespec req,
                    rem;

    req.tv_sec  = 0;
    req.tv_nsec = useconds*1000;

    rem.tv_sec  = 0;
    rem.tv_nsec = 0;

    while (TRUE)
    {

         /*---------------------------*/
         /* Sleep for specfied period */
         /*---------------------------*/

         if (nanosleep(&req,&rem) == 0)
            break;


         /*-------------*/      
         /* Interrupted */
         /*-------------*/
 
         else
         {  req.tv_sec  = 0;
            req.tv_nsec = rem.tv_nsec;
            rem.tv_sec  = 0;
            rem.tv_nsec = 0;
         }
    }

    pups_set_errno(OK);
    return(0);
}




/*--------------------------------*/
/* Is file on a ISOfs filesystem? */
/*--------------------------------*/

_PUBLIC _BOOLEAN pups_is_on_isofs(const char *f_name)

{   struct statfs buf;

    if(f_name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if(statfs(f_name,&buf) == (-1))
       return(-1);

    pups_set_errno(OK);
    if(buf.f_type == ISOFS_SUPER_MAGIC)
       return(TRUE);
    if(buf.f_type == MS_RDONLY)
       return(TRUE);

    pups_set_errno(ENOSYS);
    return(FALSE);
}




/*------------------------------------------------------*/
/* Is open file descriptor associated ISOfs filesystem? */
/*------------------------------------------------------*/

_PUBLIC _BOOLEAN pups_is_on_fisofs(const des_t fdes)

{   struct statfs buf;

    if(fstatfs(fdes,&buf) == (-1))
       return(-1);

    pups_set_errno(OK);
    if(buf.f_type == ISOFS_SUPER_MAGIC)
       return(TRUE);

    pups_set_errno(ENOSYS);
    return(FALSE);
}





/*-------------------------------*/
/* Is file on an NFS filesystem? */
/*-------------------------------*/

_PUBLIC _BOOLEAN pups_is_on_nfs(const char *f_name)

{   struct statfs buf;

    if(f_name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(FALSE);
    }


    /*-------------------------------------------------*/
    /* If the file system associated with this file an */
    /* NFS file system?                                */
    /*-------------------------------------------------*/

    if(statfs(f_name,&buf) == (-1))
       return(-1);

    pups_set_errno(OK);
    if(buf.f_type == NFS_SUPER_MAGIC)
       return(TRUE);

    pups_set_errno(ENOSYS);
    return(FALSE);
}




/*-----------------------------------------------------------*/
/* Is open file descriptor is associated with NFS filesystem */
/*-----------------------------------------------------------*/

_PUBLIC _BOOLEAN pups_is_on_fnfs(const des_t fdes)

{   struct statfs buf;


    /*-------------------------------------------------*/
    /* If the file system associated with this file an */
    /* NFS file system?                                */
    /*-------------------------------------------------*/

    if(fstatfs(fdes,&buf) == (-1))
       return(-1);

    pups_set_errno(OK);
    if(buf.f_type == NFS_SUPER_MAGIC)
       return(TRUE);

    pups_set_errno(ENOSYS);
    return(FALSE);
}





/*-----------------------*/
/* Copy a file (by name) */
/*-----------------------*/

_PUBLIC int32_t pups_cpfile(const char *from_path, const char *to_path, const int32_t mode)

{    int32_t ret,
             fdes_from        = (-1),
             fdes_to          = (-1);

    char     tmp_to_path[512] = "";

    if(from_path == (const char *)NULL || to_path == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if((fdes_from = open(from_path,0)) == (-1))
    {  pups_set_errno(EBADF);
       return(-1);
    }

    (void)snprintf(tmp_to_path,SSIZE,"%s.%d.nac",to_path,getpid());
    if(pups_creat(tmp_to_path,mode) == (-1))
    {  (void)close(fdes_from);
       pups_set_errno(EPERM);

       return(-1);
    }

    if((fdes_to = open(tmp_to_path,1)) == (-1))
    {  (void)close(fdes_from);
       pups_set_errno(EBADF);

       return(-1);
    }

    if((ret = pups_fcpfile(fdes_from,fdes_to)) == (-1))
    {  (void)unlink(tmp_to_path);
       pups_set_errno(EBADF);

       return(ret);
    }

    (void)close(fdes_from);
    (void)close(fdes_to);

    if(rename(tmp_to_path,to_path) == (-1))
    {  (void)unlink(tmp_to_path);
       pups_set_errno(EPERM);

       return(-1);
    }

    pups_set_errno(OK);
    return(ret);
}




/*-----------------------------------*/
/* Copy a file (by open descriptors) */
/*-----------------------------------*/

_PUBLIC int32_t pups_fcpfile(const des_t fdes_from, const des_t fdes_to)

{   ssize_t bytes_read;
    _BYTE   buf[512]    = "";

    do {    bytes_read = read(fdes_from,buf,512);
            if(bytes_read < 0)
               return(-1);

            if(bytes_read > 0)
            {  if(write(fdes_to,buf,bytes_read) < 0)
                  return(-1);
            }

       } while(bytes_read > 0);

    pups_set_errno(OK);
    return(0);
}




/*----------------------------*/
/* Test if command is running */
/*----------------------------*/

_PUBLIC _BOOLEAN pups_cmd_running(void)

{   DIR           *dirp      = (DIR *)NULL;
    struct dirent *next_item = (struct dirent *)NULL;

    dirp = opendir("/proc");
    pups_set_errno(OK);

    while((next_item = readdir(dirp)) != (struct dirent *)NULL)
    {   pid_t pid;

        char  procpidpath[SSIZE] = "",
              cmd_line[SSIZE]    = "";

        FILE  *stream = (FILE *)NULL;


        /*------------------------------------*/
        /* Is this a valid process directory? */
        /*------------------------------------*/

        if(sscanf(next_item->d_name,"%d",&pid) == 1)
        {  uint32_t i;

           (void)snprintf(procpidpath,SSIZE,"/proc/%d/cmdline",pid);

           stream = fopen(procpidpath,"r");
           (void)fgets(cmd_line,SSIZE,stream);
           (void)fclose(stream);
        }
    }


    /*---------------------------------------*/
    /* Comamnd is not being run (by anybody) */
    /*---------------------------------------*/

    (void)closedir(dirp);
    pups_set_errno(ESRCH);
    return(FALSE);
}




/*--------------------------------*/
/* Check if I own a given process */
/*--------------------------------*/

_PUBLIC _BOOLEAN pups_i_own(const pid_t pid)

{   struct stat buf;
    char   procpidpath[SSIZE] = "";

    if(pid <= 1)
    {   pups_set_errno(EINVAL);
        return(-1);
    }

    (void)snprintf(procpidpath,SSIZE,"/proc/%d",pid);
    if(access(procpidpath,F_OK | R_OK) == (-1))
    {  pups_set_errno(EEXIST);
       return(FALSE);
    }

    (void)stat(procpidpath,&buf);
    if(buf.st_uid != appl_uid)
    {  pups_set_errno(EPERM);
       return(FALSE);
    }

    (void)pups_set_errno(OK);
    return(TRUE);
}




/*-------------------------------------------------------*/
/* Monitor effective parent and exit if it is terminated */
/*-------------------------------------------------------*/

_PUBLIC void pups_default_parent_homeostat(void *t_info, const char *args)

{   
    if(t_info == (void *)      NULL  ||
       args   == (const char *)NULL   )
    {  pups_set_errno(EINVAL);
       return;
    }
 
    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_default_parent_homeostat] attempt by non root thread to perform PUPS/P3 utility operation");

    if(kill(appl_ppid,SIGALIVE) == (-1))
    {  if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): effective parent process [%d] terminated -- exiting\n",
                                                  date,appl_name,appl_pid,appl_host,appl_owner,appl_ppid);
          (void)fflush(stderr);
       }

       (void)pups_exit(255);
    }

    pups_set_errno(OK);
}




/*---------------------------------------------------------------------------------------*/
/* Enable process homeostat (set up a "support" child to grab the thread of execution if */
/* the parent process is fatally damaged by a dangerous operation)                       */
/*---------------------------------------------------------------------------------------*/

_PUBLIC pid_t pups_process_homeostat_enable(const _BOOLEAN wait_if_damaged)

{   pid_t    child_pid;

    _BOOLEAN save_appl_verbose   = FALSE,
             pups_parent_damaged = FALSE;

    if((child_pid = pups_fork(FALSE,FALSE)) == 0)
    {  

       /*-----------------------------------------------*/
       /* We are the parent -- simply return the PID of */
       /* support child.                                */
       /*-----------------------------------------------*/

       pups_set_errno(OK); 
       return(child_pid);
    }


    /*----------------------------------------*/
    /* Child process -- simply monitor parent */
    /*----------------------------------------*/

    appl_ppid = appl_pid;
    appl_pid  = child_pid;

    if(appl_verbose == TRUE)
       save_appl_verbose = appl_verbose;

    while(kill(appl_ppid,SIGALIVE) == 0)
         pups_usleep(1000);


    /*--------------------------------------------------------*/
    /* If we get here the parent has been terminated. Support */
    /* child now takes over the computation.                  */
    /*--------------------------------------------------------*/

    appl_verbose        = save_appl_verbose;
    pups_parent_damaged = TRUE;

    if(appl_verbose == TRUE)
    {  (void)strdate(date);

       if(wait_if_damaged == TRUE)
          (void)fprintf(stderr,"%s %s (%d@%s:%s): PID %d fatally damaged, support child (PID %d) taking over computation (wait mode)\n",
                                                                        date,appl_name,appl_pid,appl_host,appl_owner,appl_ppid,appl_pid);
       else
          (void)fprintf(stderr,"%s %s (%d@%s:%s): PID %d fatally damaged, support child (PID %d) taking over computation\n",
                                                            date,appl_name,appl_pid,appl_host,appl_owner,appl_ppid,appl_pid);
       (void)fflush(stderr);
    }


    /*------------------------*/
    /* Restart virtual timers */
    /*------------------------*/

    if(vitimer_quantum > 0)
       (void)pups_malarm(vitimer_quantum);


    /*----------------------------------------------*/
    /* Rename PSRP channel (so it relates to child) */
    /*----------------------------------------------*/

    (void)psrp_rename_channel(appl_name); 

    /*----------------------------------------*/
    /* Wait if user has requested us to do so */
    /*----------------------------------------*/

    if(wait_if_damaged == TRUE)
    {  while(pups_parent_damaged == TRUE)
            pups_usleep(1000);
    }

    pups_set_errno(EFAULT);
    return(-1);
}




/*------------------------------------------------------*/
/* Cancel support child (at end of dangerous operation) */
/*------------------------------------------------------*/

_PUBLIC void pups_process_hoemostat_disable(const pid_t child_pid)

{   

    /*------------------------------------------------------------------*/
    /* If we are not the child make sure that we do not commit suicide! */ 
    /*------------------------------------------------------------------*/

    if(child_pid != appl_pid)
       (void)kill(child_pid,SIGTERM);
}




/*------------------------------------------------------------------------------*/
/* Extended PUPS getpwnam routine which searches static password table. If this */
/* fails, the appropriate NIS map is then searched                              */
/*------------------------------------------------------------------------------*/

_PUBLIC struct passwd *pups_getpwnam(const char *username) 

{   _IMMORTAL struct passwd *pwent = (struct passwd *)NULL; 

    if(username == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return((struct passwd *)NULL);
    }


    /*--------------------------*/
    /* Authenticate remote user */
    /* By static table          */
    /*--------------------------*/

    if((pwent = getpwnam(username)) == (struct passwd *)NULL)
    { 

      #ifdef NIS_SUPPORT
       _BYTE  pwent_buf[204]     = "";
       struct passwd **pwent_ptr = (struct passwd **)NULL;

       /*------------*/
       /* By NIS map */
       /*------------*/

       if((_nss_nis_getpwnam_r(username,&pwent,pwent_buf,pwent_ptr)) == (-1))
       {  pups_set_errno(ESRCH);
          return((struct passwd *)NULL);
       }
       #else
       pups_set_errno(ESRCH);
       return((struct passwd *)NULL);
       #endif /* NIS_SUPPORT */
    }


    #ifdef SHADOW_SUPPORT 
    /*----------------------------------------------------------------------*/
    /* Are we running a shadow password file? If so get the "real" password */
    /* from /etc/shadow                                                     */
    /*----------------------------------------------------------------------*/

    if(strcmp(pwent->pw_passwd,"x") == 0 && geteuid() == 0) 
    {  struct spwd *shadow = (struct spwd *)NULL;
       
       if((shadow = getspnam(username)) == (struct spwd *)NULL)
       {  pups_set_errno(ESRCH); 
          return(FALSE);
       }
       
       pwent->pw_passwd = shadow->sp_pwdp;
    }
    #endif /* SHADOW_SUPPORT */

    pups_set_errno(OK);
    return(pwent);
}



/*------------------------------------------------------------------------------*/
/* Extended PUPS getpwnam routine which searches static password table. If this */
/* fails, the appropriate NIS map is then searched                              */
/*------------------------------------------------------------------------------*/

_PUBLIC struct passwd *pups_getpwuid(const uid_t uid)

{   _IMMORTAL struct passwd *pwent = (struct passwd *)NULL;


    /*--------------------------*/
    /* Authenticate remote user */
    /* By static table          */
    /*--------------------------*/

    if((pwent = getpwuid(uid)) == (struct passwd *)NULL)
    {  

       #ifdef NIS_SUPPORT
       _BYTE  pwent_buf[204]     = "";
       struct passwd **pwent_ptr = (struct passwd **)NULL;


       /*------------*/
       /* By NIS map */
       /*------------*/

       if((_nss_nis_getpwnam_r(uid,&pwent,pwent_buf,pwent_ptr)) == (-1))
       {  pups_set_errno(ESRCH);
          return((struct passwd *)NULL);
       }
       #else
       pups_set_errno(ESRCH);
       return((struct passwd *)NULL);
       #endif /* NIS_SUPPORT */
    }


    #ifdef SHADOW_SUPPORT
    /*----------------------------------------------------------------------*/
    /* Are we running a shadow password file? If so get the "real" password */
    /* from /etc/shadow                                                     */
    /*----------------------------------------------------------------------*/

    if(strcmp(pwent->pw_passwd,"x") == 0 && geteuid() == 0)
    {  struct spwd *shadow = (struct spwd *)NULL;

       if((shadow = getspnam(pwent->pw_name)) == (struct spwd *)NULL)
       {  pups_set_errno(ESRCH);
          return((struct passwd *)NULL);
       }

       pwent->pw_passwd = shadow->sp_pwdp;
    }
    #endif /* SHADOW_SUPPORT */

    pups_set_errno(OK);
    return(pwent);
}




/*-------------------------------------------*/
/* Table initialisation function for icrc_16 */
/*-------------------------------------------*/

_PRIVATE unsigned short icrc1(unsigned short crc, _BYTE onech)

{   uint32_t i;
    unsigned short ans = (crc ^ onech << 8);


    /*----------------------------------------------------------------------*/
    /* Generator for one bit shifts and XOR's with the generator polynomial */
    /*----------------------------------------------------------------------*/

    for(i=0; i<8; ++i)
    {  if(ans & 0x8000)
          ans = (ans <<= 1) ^ 4129;
       else
          ans <<= 1;
    }

    return(ans);
}




/*-------------------------------------------*/
/* Compute 16 bit cyclic redundancy checksum */
/*-------------------------------------------*/
/*-------------------------------*/
/* Macros required by 32 bit CRC */
/*-------------------------------*/

#define LOBYTE(x) ((_BYTE)((x) & 0xFF))
#define HIBYTE(x) ((_BYTE)((x) >> 8))


/*-------------------------------------*/
/* Global variables need by 32 bit CRC */
/*-------------------------------------*/

_PRIVATE _BOOLEAN       crc32_table_generated = FALSE;         
_PRIVATE unsigned short icrctb[256]           = { [0 ... 255] = 0 };
_PRIVATE _BYTE rchr[256]                      = "";


/*-------------------------*/
/* Table of 4 bit reverses */
/*-------------------------*/

_PRIVATE _BYTE it[16] = { 0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15 };


#ifdef PTHREAD_SUPPORT
_PRIVATE pthread_mutex_t   crc32_init_mutex;
#endif /* PTHREAD_SUPPORT */



                                                    /*----------------------------------------------------*/
_PRIVATE unsigned short icrc_16(uint16_t      crc,  /* Used to initialise input register if jrev negative */
                                _BYTE     *bufptr,  /* Pointer to buffer we are computing checksum for    */
                                size_t        len,  /* Lenght of the buffer                               */
                                int16_t     jinit,  /* Initialistion                                      */
                                int32_t      jrev)  /* Compute reversed checksum                          */
                                                    /*----------------------------------------------------*/

{   uint16_t irc1(uint16_t crc, _BYTE onech);

    uint64_t i;
    uint16_t j,
             cword = crc;


    /*---------------------------------*/
    /* Do we need to intialise tables? */
    /*---------------------------------*/

    if (crc32_table_generated == FALSE)
    {  crc32_table_generated = TRUE;

       #ifdef PTHREAD_SUPPORT
       pthread_mutex_trylock(&crc32_init_mutex);


       /*-------------------------------------------------------------*/
       /* If someone has generated the table while we were waiting to */
       /* out work is done                                            */
       /*-------------------------------------------------------------*/

       if(crc32_table_generated == TRUE)
       {  pthread_mutex_unlock(&crc32_init_mutex);
          return(0);
       }
       #endif /* pthread support */

       for(j=0; j<=255; ++j)
       {

           /*--------------------------------------------------------------------------------*/
           /* The two tables are: CRC's of all characters and bit reverses of all characters */
           /*--------------------------------------------------------------------------------*/

           icrctb[j] = icrc1(j << 8,(_BYTE)0);
           rchr[j]  = (_BYTE)(it[j & 0xF] << 4 | it[j >> 4]);
       }

       #ifdef PTHREAD_SUPPORT
       pthread_mutex_unlock(&crc32_init_mutex);
       #endif /* PTHREAD_SUPPORT */
    }

    if(jinit >= 0)
       cword = ((_BYTE)jinit) | (((_BYTE)jinit) << 8);
    else if(jrev < 0)
       cword = rchr[HIBYTE(cword)] | rchr[LOBYTE(cword)] << 8;


    /*-----------------------------------*/
    /* Initialise the remainder register */
    /*-----------------------------------*/

    for(i=0; i<len; ++i)
       cword = icrctb[(jrev < 0 ? rchr[bufptr[i]] : bufptr[i]) ^ HIBYTE(cword)] ^ LOBYTE(cword) << 8;

    return(jrev >= 0 ? cword : rchr[HIBYTE(cword)] | rchr[LOBYTE(cword)] << 8);
}




/*----------------------------------------------------*/
/* Compute (pseudo) 32 bit cyclic redundancy checksum */
/*----------------------------------------------------*/

_PUBLIC int32_t pups_crc_32(const size_t len, _BYTE *bufptr)

{   uint32_t crc32,
             low_16_bits,
             high_16_bits;

    if(len <= 0 || bufptr == (_BYTE *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*----------------------*/
    /* Geneate 16 bit CRC's */
    /*----------------------*/

    low_16_bits  = (int32_t)icrc_16(0,bufptr,len,0,1);
    high_16_bits = (int32_t)icrc_16(0,bufptr,len,0,(-1)) >> 16;
   
    crc32 = low_16_bits | high_16_bits;

    pups_set_errno(OK);
    return(crc32);
}



/*-------------------------------*/
/* Generate table for 64 bit CRC */
/*-------------------------------*/

_PRIVATE _BOOLEAN  crc64_table_generated    = FALSE;         
_PRIVATE uint64_t  poly                     = 0xC96C5795D7870F42;
_PRIVATE uint64_t  crc64_lookup_table[256]  = { [0 ... 255] = 0L };

#ifdef PTHREAD_SUPPORT
_PRIVATE pthread_mutex_t   crc64_init_mutex;
#endif /* PTHREAD_SUPPORT */


_PRIVATE void generate_crc64_lookup_table(void)
{   uint32_t i,
             j;


    #ifdef PTHREAD_SUPPORT
    pthread_mutex_trylock(&crc64_init_mutex);
    

    /*-------------------------------------------------------------*/
    /* If someone has generated the table while we were waiting to */
    /* out work is done                                            */
    /*-------------------------------------------------------------*/

    if(crc64_table_generated == TRUE)
    {  pthread_mutex_unlock(&crc64_init_mutex);
       return;
    }
    #endif /* PTHREAD_SUPPORT */


    /*---------------------------------------*/
    /* Generate look up table for 64 bit CRC */
    /*---------------------------------------*/

    for(i=0; i<256; ++i)
    {
       uint64_t crc64 = i;
       for(j=0; j<8; ++j)
       {

          /*-----------------------------*/
          /* is current coefficient set? */
          /*-----------------------------*/

          if(crc64 & 1)
          {

             /*--------------------------------------------------------------------------*/
             /* yes, then assume it gets zero'd (by implied x^64 coefficient of dividend)*/
             /*--------------------------------------------------------------------------*/

             crc64 >>= 1;

             /*-------------------------*/
             /* Add rest of the divisor */
             /*-------------------------*/

             crc64 ^= poly;
          }


          /*------------------------------*/
          /* No? Move to next coefficient */
          /*------------------------------*/

          else
             crc64 >>= 1;
      }

      crc64_lookup_table[i] = crc64;
   }

   crc64_table_generated = TRUE; 

   #ifdef PTHREAD_SUPPORT
   pthread_mutex_unlock(&crc64_init_mutex);
   #endif /* PTHREAD_SUPPORT */
}




/*---------------------*/
/* Generate 64 bit CRC */
/*---------------------*/

_PUBLIC uint64_t pups_crc_64(const size_t len, _BYTE *bufptr)
{   size_t   i;
    uint64_t crc64 = 0;


    if(len <= 0 || bufptr == (_BYTE *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*-------------------------------------------------*/

    /* Generate lookup table for 64 bit CRC if we need */
    /* to do so                                        */
    /*-------------------------------------------------*/

    if(crc64_table_generated == FALSE)
       generate_crc64_lookup_table();
       

    for(i=0; i<len; ++i)
    {   uint64_t lookup;
        _BYTE    t_index;

        t_index = bufptr[i] ^ crc64;
        lookup  = crc64_lookup_table[t_index];
        crc64   >>= 8;
        crc64   ^=  lookup;
    }

    pups_set_errno(OK);
    return(crc64);
}




/*--------------------------------------*/
/* Extract CRC signature from file name */
/*--------------------------------------*/

_PUBLIC uint64_t pups_get_signature(const char *name, const char signer)

{   uint32_t i,
             cnt = 0;

    uint64_t crc;

    char     crc_signature[SSIZE]  = "";
    _BOOLEAN begin_crc_signature = FALSE;

    if(name == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    for(i=0; i<strlen(name); ++i)
    {  if(name[i] == signer)
       {  if(begin_crc_signature == FALSE)
             begin_crc_signature = TRUE;
          else
          {  (void)sscanf(crc_signature,"%lx",&crc);

             pups_set_errno(OK);
             return(crc);
          }
       }
       else if(begin_crc_signature == TRUE)
       {  crc_signature[cnt] = name[i];
          ++cnt;
       }
    }

    pups_set_errno(ESRCH);
    return(-1);
}




/*------------------------------------------------*/
/* If a checksum is present sign filename with it */
/*------------------------------------------------*/

_PUBLIC int32_t pups_sign(const uint64_t crc, const char *name, char *signed_name, const char signer)

{  ssize_t c_index;
   char    tmpstr[SSIZE] = "";

   if(name == (char *)NULL || signed_name == (char *)NULL)
   {  pups_set_errno(EINVAL);
      return(-1);
   }

   c_index = ch_pos(name,'/');
   if(ch_index == (-1))


      /*----------------------------------------*/
      /* Leafname - simply prepend check string */
      /*----------------------------------------*/

      (void)snprintf(signed_name,SSIZE,"%c%lx%c%s",signer,crc,signer,name);
   else
   {

      /*---------------------------------------------------------*/
      /* Pathname -- extract leaf and prepend check string to it */
      /*---------------------------------------------------------*/

      char path[SSIZE] = "";

      (void)strlcpy(path,name,SSIZE);
      path[c_index] = '\0';

      (void)strlcpy(tmpstr,&name[c_index+1],SSIZE);
      (void)snprintf(signed_name,SSIZE,"%s/#%d#%s",path,crc,name);
   }

   pups_set_errno(OK);
   return(0);
}




/*---------------------------------*/
/* PUPS lseek which is signal safe */
/*---------------------------------*/

_PUBLIC off_t pups_lseek(const des_t fdes, const off_t pos, const int32_t whence)

{    int32_t ret;

    sigset_t set,
             o_set;

    (void)sigfillset(&set);

    (void)sigdelset (&set,SIGINT);
    (void)sigdelset (&set,SIGTERM);
    (void)sigdelset (&set,SIGSEGV);

    (void)pups_sigprocmask(SIG_SETMASK,&set,&o_set);

    if((ret = lseek(fdes,pos,whence)) == (-1))
       return(-1);

    (void)pups_sigprocmask(SIG_SETMASK,&o_set,(sigset_t *)NULL);
    return(ret);
}




/*------------------------------*/
/* Replace item in command tail */
/*------------------------------*/

_PUBLIC _BOOLEAN pups_replace_cmd_tail_item(const char *flag, const char *item)

{   uint32_t i;

    if(flag == (const char *)NULL || item == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    pups_set_errno(OK);
    for(i=0; i<appl_t_args; ++i)
    {  if(strcmp(flag,args[i]) == 0)
       {  if(i == appl_t_args - 1)
            args[t_args] = (char *)pups_malloc(SSIZE);

          (void)strlcpy(args[i+1],item,SSIZE);
          ++appl_t_args;

          pups_set_errno(OK);
          return(TRUE);
       }
    }

    pups_set_errno(ESRCH);
    return(FALSE);
}




/*--------------------*/
/* Clear command tail */
/*--------------------*/

_PUBLIC int32_t pups_clear_cmd_tail(void)
{   uint32_t i;

    if(appl_t_args <= 0)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    for(i=0; i<appl_t_args; ++i)
    {  args[i] = (char *)pups_free(args[i]);
       argd[i] = FALSE;
    }

    sargc       = 0;
    appl_t_args = 0;

    pups_set_errno(OK); 
    return(0);
}




/*-------------------------------*/
/* Insert item into command tail */
/*-------------------------------*/

_PUBLIC int32_t pups_insert_cmd_tail_item(const char *flag, const char *item)
{  
    if(appl_t_args > MAX_CMD_ARGS)
    {  pups_set_errno(ENOSPC);
       return(-1);
    }

    if(flag == (const char *)NULL || item == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    } 

    if(args[appl_t_args] == (char *)NULL)
    {  args[appl_t_args] = (char *)pups_malloc(SSIZE);
       (void)strlcpy(args[appl_t_args],flag,SSIZE);
       argd[appl_t_args] = FALSE;
       
       if(item != (const char *)NULL)
       {  args[appl_t_args + 1] = (char *)pups_malloc(SSIZE);
          (void)strlcpy(args[appl_t_args + 1],item,SSIZE);
          argd[appl_t_args + 1] = FALSE;
       }
    }

    appl_t_args += 2;
    sargc       += 2;

    pups_set_errno(OK);
    return(0);
}




/*-------------------------------------------------------------------------------*/
/* Backtrack to given point if a segmentation violation occurs -- this partially */
/* protects a dynamic application from badly written code!                       */
/*-------------------------------------------------------------------------------*/

_PRIVATE sigjmp_buf segv_backtrack;


/*---------------------------------------------------------------*/
/* Handler for backtracking (when segmentation violation occurs) */
/*---------------------------------------------------------------*/

_PRIVATE int32_t pups_segv_backtrack_handler(const int32_t signum)

{   (void)pups_usleep(100);
    (void)siglongjmp(segv_backtrack,1);
}


/*-----------------------------------------------------------*/
/* Set a backtrack re-entry point (in process address space) */
/*-----------------------------------------------------------*/

_PUBLIC _BOOLEAN pups_backtrack(const _BOOLEAN set)

{

    /*----------------------------------------------------------------*/
    /* Enable backtracking -- SIGSEGV will cause process to backtrack */
    /* to this point                                                  */
    /*----------------------------------------------------------------*/

    if(set == TRUE)
    {  (void)pups_sighandle(SIGSEGV,"segv_backtrack_handler",(void *)&pups_segv_backtrack_handler,(sigset_t *)NULL);

       if(sigsetjmp(segv_backtrack,1) == 1)
          return(PUPS_BACKTRACK);
       else
          return(TRUE);
    }


    /*-----------------------------------------*/
    /* Default handler - backtracking disabled */
    /*-----------------------------------------*/

    else
       (void)pups_sighandle(SIGSEGV,"segbusfpe_handler",(void *)&segbusfpe_handler,(sigset_t *)NULL);

    return(FALSE);
}




/*------------------------*/
/* Has file been updated? */
/*------------------------*/

_PUBLIC _BOOLEAN pups_file_updated(const char *filename)

{   struct    stat   stat_buf;
    _IMMORTAL time_t last_update = (-1);

    if(filename == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if(stat(filename,&stat_buf) == (-1))
       return(-1);

    pups_set_errno(OK);
    if(last_update == (-1))
    {  last_update = stat_buf.st_ctime;
       return(FALSE);
    }
    else if(stat_buf.st_ctime != last_update)
    {  last_update = stat_buf.st_ctime;
       return(TRUE);
    }

    return(FALSE);
}




/*---------------------------------------*/
/* Set (round robin) scheduling priority */
/*---------------------------------------*/

_PUBLIC int32_t pups_set_rt_sched(const int32_t priority)

{   int32_t ret;
    struct  sched_param p;

    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_set_rt_sched] attempt by non root thread to perform PUPS/P3 utility operation");

    if(priority < sched_get_priority_min(SCHED_RR) || priority > sched_get_priority_max(SCHED_RR))
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    p.sched_priority = priority;
    ret = sched_setscheduler(appl_pid,SCHED_RR,&p);

    pups_set_errno(OK);
    return(ret);
}




/*---------------------------------------------*/
/* Switch to standard (time sharing) scheduler */
/*---------------------------------------------*/

_PUBLIC int32_t pups_set_tslice_sched(void)  

{   int32_t ret;
    struct  sched_param p;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_set_tslice_sched] attempt by non root thread to perform PUPS/P3 utility operation");

    p.sched_priority = 0;
    ret = sched_setscheduler(appl_pid,SCHED_OTHER,&p);

    return(ret);
}




/*-------------------------------------------------------------*/
/* Is host is reachable? Note that IP address can be either an */
/* IPBV4 or IPV6 address or a host name                        */
/*-------------------------------------------------------------*/

_PUBLIC _BOOLEAN pups_host_reachable(const char *ip_addr)

{   char ping_command[SSIZE] = "",
	 ping_ret[SSIZE]     = "";

    FILE *stream = (FILE *)NULL;

    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_host_reachable] attempt by non root thread to perform PUPS/P3 utility operation");

    if(ip_addr == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    (void)snprintf(ping_command,SSIZE,"ping -q -w 1 -c 1 %s",ip_addr);
    stream = popen(ping_command,"r");

    (void)fgets(ping_ret,255,stream);
    (void)pclose(stream);

    if(strcmp(ping_ret,"rtt") != 0)
    {  pups_set_errno(EACCES);
       return(FALSE);
    }

    pups_set_errno(OK);
    return(TRUE);
}




/*-----------------------------------------------------------------------*/
/* Set a discretionary lock (note this type of lock is safe over network */
/* filesystem such as NFS                                                */
/*-----------------------------------------------------------------------*/

_PRIVATE int32_t lock_fdes [PUPS_MAX_FLOCKS] = { [0 ... PUPS_MAX_FLOCKS-1] = (-1) };
_PRIVATE int32_t lock_type [PUPS_MAX_FLOCKS] = { [0 ... PUPS_MAX_FLOCKS-1] = (-1) };
_PRIVATE off_t   lock_start[PUPS_MAX_FLOCKS] = { [0 ... PUPS_MAX_FLOCKS-1] = 0    };
_PRIVATE off_t   lock_size [PUPS_MAX_FLOCKS] = { [0 ... PUPS_MAX_FLOCKS-1] = 0    };

_PUBLIC int32_t pups_flock(const des_t fdes, const int32_t cmd, const off_t l_start, const off_t l_len, const _BOOLEAN wait)

{   uint32_t i,
             lock_index;

    int32_t ret;

    struct flock lock;

    if(l_start < 0 || l_len <= 0 || wait != TRUE && wait != FALSE)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    for(i=0; i<PUPS_MAX_FLOCKS; ++i)
    {  if(lock_fdes[i] == fdes)
       {

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&lock_mutex);
          #endif /* PTHREAD_SUPPORT */

          lock_index = i;
          goto lock_found;
       }
    }


    /*-------------------------------------------------------------*/
    /* We must have a valid file descriptor for a unlock operation */
    /*-------------------------------------------------------------*/

    if(cmd == F_UNLCK)
    {

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&lock_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(EBADF);
       return(-1);
    }

lock_found:

    if(cmd == F_WRLCK)
    {  lock.l_type = F_WRLCK;
       lock.l_start            = l_start;
       lock.l_len              = l_len;
       lock.l_whence           = SEEK_SET;

       lock_type[lock_index]   = cmd;
       lock_start[lock_index]  = l_start;
       lock_size[lock_index]   = l_len;
    }
    else if(cmd == F_RDLCK)
    {  lock.l_type = F_RDLCK;
       lock.l_start            = l_start;
       lock.l_len              = l_len;
       lock.l_whence           = SEEK_SET;

       lock_type[lock_index]   = cmd;
       lock_start[lock_index]  = l_start;
       lock_size[lock_index]   = l_len;
    }
    else if(lock.l_type == F_UNLCK)
    {  lock.l_type = F_UNLCK;
       lock_type[lock_index]   = (-1);
       lock_start[lock_index]  = 0;
       lock_size[lock_index]   = 0;
    }
    else
    {

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&lock_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(EINVAL);
       return(-1);
    }


    /*---------------------------------------------------*/
    /* If wait is TRUE wait to acquire lock -- otherwise */
    /* return if we cannot acquire lock.                 */
    /*---------------------------------------------------*/

    if(wait == TRUE)
    {  if((ret = fcntl(fdes,F_SETLKW,&lock)) == (-1))
       {

         #ifdef PTHREAD_SUPPORT
         (void)pthread_mutex_unlock(&lock_mutex);
         #endif /* PTHREAD_SUPPORT */

          pups_set_errno(EBADF);
          return(-1);
       }
    }
    else
    {  if((ret = fcntl(fdes,F_SETLK, &lock)) == (-1))
       {

          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&lock_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(EBADF);
          return(-1);
       }
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    (void)pups_set_errno(OK);
    return(ret);
}




/*----------------------------------------------------------------------------*/
/* Set a discretionary lock for a named file. (note this type of lock is safe */
/* over network filesystem such as NFS                                        */
/*----------------------------------------------------------------------------*/

_PRIVATE char lock_name[PUPS_MAX_FLOCKS][SSIZE] = { [0 ... PUPS_MAX_FLOCKS-1] = {""}};

_PUBLIC int32_t pups_flockfile(const char *lock_file, const int32_t cmd, const off_t l_start, const off_t l_len, const _BOOLEAN wait)

{   uint32_t i,
             min_index  = 0,
             lock_index = 0;

    int32_t  ret;

    if(lock_file == (char *)NULL || l_start < 0 || l_len < 1)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*----------------------------------------*/
    /* If lock file does not exist create it. */
    /*----------------------------------------*/

    if(access(lock_file,F_OK) == (-1))
    {

       /*----------------------------------------------*/
       /* Lock file must exist if we are attempting an */
       /* unlock operation.                            */
       /*----------------------------------------------*/

       if(cmd == F_UNLCK)
       {
          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&lock_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(EEXIST);
          return(-1);
       } 

       if(pups_creat(lock_file,0600) == (-1))
          return(-1);


       /*------------------*/
       /* Find a free lock */
       /*------------------*/

       for(i=0; i<PUPS_MAX_FLOCKS; ++i)
       {  if(lock_fdes[i] == (-1))
          {  lock_index = i;
             goto lock_index_found;
          }
       }


       /*---------------------*/
       /* Too many locks held */
       /*---------------------*/

       if(min_index == (-1))
       {
          #ifdef PTHREAD_SUPPORT
          (void)pthread_mutex_unlock(&lock_mutex);
          #endif /* PTHREAD_SUPPORT */

          pups_set_errno(ENFILE);
          return(-1);
       }

lock_index_found:


       /*----------------------------*/
       /* Open file (and apply lock) */
       /*----------------------------*/

       if((ret = pups_flock(lock_fdes[lock_index],cmd,l_start,l_len,wait)) == 0)
       {  (void)strlcpy(lock_name[lock_index],lock_file,SSIZE);
          lock_fdes[lock_index] = open(lock_file,2);
       }

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&lock_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(OK);
       return(ret);
    }


    /*---------------------------------*/
    /* Get lock index for current file */
    /*---------------------------------*/

    for(i=0; i<PUPS_MAX_FLOCKS; ++i)
    {  if(strcmp(lock_name[i],lock_file) == 0)
       {  

          /*--------------------------------------------------*/
          /* Attempt to unlock file which is already unlocked */ 
          /*--------------------------------------------------*/

          if(cmd == F_UNLCK && lock_fdes[i] == (-1))
          {

             #ifdef PTHREAD_SUPPORT
             (void)pthread_mutex_unlock(&lock_mutex);
             #endif /* PTHREAD_SUPPORT */

             pups_set_errno(EACCES);
             return(-1);
          }

          lock_index = i;
          goto lock_found;
       }


       /*---------------------------------------------*/
       /* Record lowest unused slot in the lock table */
       /*---------------------------------------------*/

       if(min_index == (-1) && lock_fdes[i] == (-1))
          min_index = i;
    }


    /*---------------------*/
    /* Too many locks held */
    /*---------------------*/

    if(min_index == (-1))
    {

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&lock_mutex);
       #endif /* PTHREAD_SUPPORT */

       pups_set_errno(ENFILE);
       return(-1);
    }


    /*-------------------------------------*/
    /* We don't hold a lock for this file  */
    /* Use the lowest numbered unused lock */
    /*-------------------------------------*/

    lock_index = min_index;

lock_found:

    if(cmd == F_UNLCK)
    {  

       /*------------*/
       /* Clear lock */
       /*------------*/

       (void)close(lock_fdes[lock_index]);
       lock_fdes[lock_index]       = (-1);
       (void)strlcpy(lock_name[lock_index],"",SSIZE);
    }
    else
    {  

       /*--------------------------------------*/
       /* If file is currently closed, open it */
       /*--------------------------------------*/

       if(lock_fdes[lock_index] == (-1))
          lock_fdes[lock_index] = open(lock_file,2);


       /*------------*/
       /* Apply lock */
       /*------------*/

       (void)strlcpy(lock_name[lock_index],lock_file,SSIZE);
       ret = pups_flock(lock_fdes[lock_index],cmd,l_start,l_len,wait);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(ret);
}




/*----------------------------------*/
/* Display process flock lock table */
/*----------------------------------*/

_PUBLIC int32_t pups_show_flock_locks(const FILE *stream)

{   uint32_t i,
             flock_cnt = 0;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_show_flock_locks] attempt by non root thread to perform PUPS/P3 utility operation");

    if(stream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    (void)fprintf(stream,"\n\nFlocks held by %s (%d@%s)\n\n",appl_name,appl_pid,appl_host);
    (void)fflush(stream);

    for(i=0; i<PUPS_MAX_FLOCKS; ++i)
    {  if(lock_fdes[i] != (-1))
       {  if(lock_type[i] == F_RDLCK)
             (void)fprintf(stream,"    %04d: (slot %04d): %-32s [READLOCK  starting %016lx, length %016lx]",flock_cnt,i,lock_name[i],lock_start[i],lock_size[i]);  
          else if(lock_type[i] == F_WRLCK)
             (void)fprintf(stream,"    %04d: (slot %04d): %-32s [WRITELOCK starting %016lx, length %016lx]]",flock_cnt,i,lock_name[i],lock_start[i],lock_size[i]);  
          (void)fflush(stream);

          ++flock_cnt;
       }
    }

    if(flock_cnt == 1)
       (void)fprintf(stream,"\n\n    Total of %04d flock held (%04d slots free)\n\n",1,PUPS_MAX_FLOCKS - 1);
    else if(flock_cnt > 0)
       (void)fprintf(stream,"\n\n    Total of %04d flocks held (%04d slots free)\n\n",flock_cnt,PUPS_MAX_FLOCKS - flock_cnt);
    else
       (void)fprintf(stream,"    No flocks held (%04d slots free)\n\n",PUPS_MAX_FLOCKS);
    (void)fflush(stream);

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&lock_mutex);
    #endif /* PTHREAD_SUPPORT */

    (void)pups_set_errno(OK);
    return(0);
}




/*----------------------------------------------------------------------------*/
/* Substitute string s3 for substring s2 in s1, returning result in string s4 */
/*----------------------------------------------------------------------------*/

_PUBLIC int32_t strsub(const char *s1, const char *s2, const char *s3, char *s4)

{   ssize_t i;

    if(s1 == (const char *)NULL  ||
       s2 == (const char *)NULL  ||
       s3 == (const char *)NULL  ||
       s4 == (char       *)NULL   )
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    for(i=0; i<strlen(s1); ++i)
    {  if(strncmp((char *)&s1[i],s2,strlen(s2)) == 0)
       {  int32_t s1_index;

          (void)strlcpy(s4,s1,SSIZE);
          s4[i] = '\0';
          (void)strlcat(s4,s3,SSIZE);

          s1_index = i + strlen(s2);
          (void)strlcat(s4,(char *)&s1[s1_index],SSIZE);

          pups_set_errno(OK);
          return(0);
       }
    }

    pups_set_errno(ESRCH);
    return(-1);
}




/*------------------------------------------------------------*/
/* Return file type (wrapper function for the "file" command) */
/*------------------------------------------------------------*/

_PUBLIC char *pups_get_file_type(const char *file_name)

{   char strdum[SSIZE]   = "",
         line[SSIZE]     = "",
         file_cmd[SSIZE] = "";

    FILE      *pstream         = (FILE *)NULL;
    _IMMORTAL char type[SSIZE] = "";

    if(file_name == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return((char *)NULL);
    }
    else
       (void)snprintf(file_cmd,SSIZE,"file %s",file_name);

    pstream = popen(file_cmd,"r");
    (void)fgets(line,SSIZE,pstream);
    (void)pclose(pstream);

    (void)sscanf(line,"%s%s",strdum,type);

    pups_set_errno(OK);
    return(type);
}




/*--------------------------------*/
/* Convert a string to lower case */
/*--------------------------------*/

_PUBLIC int32_t downcase(char *s)

{   size_t i;

    if(s == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    
    for(i=0; i<strlen(s); ++i)
       s[i] = tolower(s[i]);

    pups_set_errno(OK);
    return(0);
}



/*------------------------------*/
/* Convert string to upper case */
/*------------------------------*/

_PUBLIC int32_t upcase(char *s)

{   size_t i;

    if(s == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }
    
    for(i=0; i<strlen(s); ++i)
       s[i] = toupper(s[i]);

    pups_set_errno(OK);
    return(0);
}



/*-------------------------------------*/
/* Get name of host exporting NFS file */
/*-------------------------------------*/

_PUBLIC int32_t pups_get_nfs_host(const char *pathname, const char *host)

{   char strdum[SSIZE]     = "",
         df_cmd[SSIZE]     = "",
         line[SSIZE]       = "",
         nfs_mount[SSIZE]  = "";

    _BOOLEAN looper      = TRUE;
    FILE     *pstream    = (FILE *)NULL;
     

    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_get_nfs_host] attempt by non root thread to perform PUPS/P3 utility operation");

    if(pathname == (const char *)NULL || host == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    pstream = popen(df_cmd,"r");
    do {    (void)fgets(line,SSIZE,pstream);

            if(feof(pstream) != 0)
               looper = FALSE;
            else if(sscanf(line,"%s%s",nfs_mount,strdum) == 2)
            {  (void)mchrep(' ',":",line);
               (void)sscanf(nfs_mount,"%s%s",host,strdum);

               (void)pclose(pstream);
               pups_set_errno(OK);
               return(0);
            }
       } while(looper == TRUE);

    pups_set_errno(ESRCH);
    return(-1);
}




/*-----------------------------*/
/* Reflect endianness of FTYPE */
/*-----------------------------*/

_PUBLIC float fconv(const FTYPE fOld)
{
    char *pcOrig = (char *)NULL,
         *pcNew  = (char *)NULL;

    int32_t i,
            j;

    FTYPE fNew;

    pcOrig = (char *) &fOld,
    pcNew  = (char *) &fNew;

    j = sizeof(FTYPE) - 1;
    for(i = 0; i<sizeof(FTYPE); i++)
    {   pcNew[j] = pcOrig[i];
        j--;
    }

    return(fNew);
}




/*-------------------------------*/
/* Reflect endianness of int32_t */
/*-------------------------------*/

_PUBLIC int32_t iconv(const int32_t iOld)
{
    char *pcOrig = (char *)NULL,
         *pcNew  = (char *)NULL;

     int32_t i,
             j,
             iNew;

    pcOrig = (char *)&iOld;
    pcNew  = (char *)&iNew;

    j = sizeof(int32_t)-1;
    for(i = 0; i<sizeof(int32_t); i++)
    {   pcNew[j] = pcOrig[i];
        j--;
    }

    return(iNew);
}




/*-----------------------------*/
/* Reflect endianness of short */
/*-----------------------------*/

_PUBLIC int16_t sconv(const int16_t sOld)
{
    char *pcOrig = (char *)NULL,
         * pcNew = (char *)NULL;

    int16_t i,
            j,
            sNew;

    pcOrig = (char *)&sOld;
    pcNew = (char *) &sNew;

    j = sizeof(int16_t)-1;
    for(i = 0; i<sizeof(int16_t); i++)
    {   pcNew[j] = pcOrig[i];
        j--;
    }

    return(sNew);
}




/*----------------------------*/
/* Reflect endianness of long */
/*----------------------------*/

_PUBLIC int64_t lconv(const  int64_t lOld)
{
    char  *pcOrig = (char *)NULL,
          *pcNew  = (char *)NULL;

     int64_t i,
             j,
             lNew;

    pcOrig = (char *)&lOld;
    pcNew  = (char *)&lNew;

    j = sizeof(int64_t )-1;
    for(i = 0; i<sizeof(int64_t); i++)
    {   pcNew[j] = pcOrig[i];
        j--;
    }

    return(lNew);
}




/*-------------------------------------*/
/* Set context of non-local goto valid */
/*-------------------------------------*/

_PUBLIC int32_t pups_set_jump_vector(void)

{   jump_vector = TRUE;
    return(0);
}




/*---------------------------------------*/
/* Set context of non-local goto invalid */
/*---------------------------------------*/

_PUBLIC int32_t pups_reset_jump_vector(void)

{   jump_vector = FALSE;
    return(0);
}




/*------------------------------------------------------*/
/* Return TRUE if Linux version information matches key */
/*------------------------------------------------------*/

_PUBLIC _BOOLEAN pups_is_linuxversion(const char *key)

{   FILE *stream           = (FILE *)NULL;
    char versionstr[SSIZE] = "";

    if(key == (char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    stream = fopen("/proc/version","r");
    (void)fgets(versionstr,SSIZE,stream);
    (void)fclose(stream);

    if(strin(versionstr,key) == TRUE)
       return(TRUE);

    return(FALSE);
}




/*--------------------------------------------------------------*/
/* Get current process loading (and memory statistics) for host */
/*--------------------------------------------------------------*/ 
                                                                 /*---------------------------------------------*/
_PUBLIC int32_t pups_get_resource_loading(FTYPE    *cpu_loading, /* CPU loading                                 */
                                          uint32_t    *free_mem) /* Free (virtual) memory                       */
                                                                 /*---------------------------------------------*/

{   char line[SSIZE]   = "",
         strdum[SSIZE] = "",
         tmpstr[SSIZE] = "";

    uint32_t i,
             free,
             next_mem,
             cnt  = 3;

    FTYPE bogomips;

    _BOOLEAN looper  = TRUE;
    FILE     *stream = (FILE *)NULL;

    if(cpu_loading == (FTYPE **)NULL || free_mem == (int32_t *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    stream = fopen("/proc/stat","r");
    (void)fscanf(stream,"%f",cpu_loading);
    (void)fclose(stream);

    *cpu_loading /= (FTYPE)get_nprocs();


    /*--------------------------------------------------------------*/
    /* LINUX returns the real amount of free (e.g. inactive) memory */
    /* as the cached field.                                         */
    /*--------------------------------------------------------------*/

    (*free_mem) = 0;
    stream = fopen("/proc/meminfo","r");
    do {    (void)fscanf(stream,"%s%d",tmpstr,&next_mem);
            if(strcmp(tmpstr,"MemFree:") == 0)
               (*free_mem) += next_mem;

            else if(strcmp(tmpstr,"Cached:") == 0)
            {  (*free_mem) += next_mem;
               looper = FALSE;
            }
       } while(looper == TRUE);
               
    (void)fclose(stream);

    (void)pups_set_errno(OK);
    return(0);
}




#ifdef SSH_SUPPORT
/*----------------------------*/
/* Peer to peer file transfer */
/*----------------------------*/

_PUBLIC int32_t pups_scp(const char *ssh_port, const char *to_filepath, const char *from_filepath)

{   char    line[SSIZE]      = "",
            ssh_opts[SSIZE]  = "";

    int32_t status,
            ret              = 0;

    pid_t   scp_pid          = (-1);

    if(from_filepath == (const char *)NULL  ||
       to_filepath   == (const char *)NULL   )
    {  (void)pups_set_errno(EINVAL);
       return(-1);
    }


    /*------------------------------------------------------*/
    /* Fork of a process to run sftp and transfer the files */
    /*------------------------------------------------------*/

    if((scp_pid = pups_fork(FALSE,FALSE)) == 0)
    {

       /*--------------------*/
       /* Child side of fork */
       /*--------------------*/


       /*-------------------------------*/
       /* We don't need stdio for child */
       /*-------------------------------*/

       (void)fclose(stdin);
       (void)fclose(stdout);
       (void)fclose(stderr);


       /*----------------------------------------*/
       /* Transaction is public/private key only */
       /*----------------------------------------*/

       (void)snprintf(ssh_opts,SSIZE,"-oPasswordAuthentication=no");


       /*----------------------------------------*/
       /* Are we sending to a non-standard port? */
       /*----------------------------------------*/

       if(ssh_port != (const char *)NULL)
       {   char portOption[26] = "";

          (void)snprintf(portOption,SSIZE," -P %s",ssh_port);
          (void)strlcat(ssh_opts,portOption,SSIZE);
       }
       else if(strcmp(ssh_remote_port,"") != 0)
       {  char portOption[26] = "";

          (void)snprintf(portOption,SSIZE," -P %s",ssh_remote_port);
          (void)strlcat(ssh_opts,portOption,SSIZE);
       }
   

       /*-------------------------------------*/
       /* Compress material being transferred */
       /*-------------------------------------*/
        
       if(ssh_compression == TRUE)
          (void)strlcat(ssh_opts," +C",SSIZE);
  

       /*-----------------*/
       /* Run scp command */
       /*-----------------*/

       (void)execlp("scp","scp",ssh_opts,from_filepath,to_filepath,(char *)NULL);


       /*---------------------------------------------------------*/
       /* We should not get here -- if we do an error has occured */
       /*---------------------------------------------------------*/

       _exit(255);
    }


    /*---------------------*/
    /* Parent side of fork */
    /*---------------------*/

    else if(scp_pid == (-1))
    {  pups_set_errno(ENOEXEC);
       return(-1);
    }


    /*-----------------------------*/
    /* Wait for child to terminate */
    /* and read reply (if any)     */
    /*-----------------------------*/

    while((ret = waitpid(scp_pid,&status,WNOHANG)) != scp_pid)
    {    if(ret == (-1))
         {  pups_set_errno(ENOEXEC);
            return(-1);
         }

          pups_usleep(100);
    }


    /*--------------------------------*/
    /* Get exit status of scp command */
    /*--------------------------------*/

    if(WEXITSTATUS(status) > 0)
    {

       /*-------------------------------*/
       /* scp command failed to execute */
       /*-------------------------------*/

       pups_set_errno(EACCES);
       return(-1);
    }


    /*--------------------------*/
    /* We transferred the file  */
    /*--------------------------*/

    (void)pups_set_errno(OK);
    return(WEXITSTATUS(status));
}
#endif /* SSH_SUPPORT */




/*----------------------*/
/* Is process P3 aware? */
/*----------------------*/

_PRIVATE _BOOLEAN pups_p3_aware(const pid_t pid)

{   DIR           *dirp         = (DIR *)NULL;
    struct dirent *next_item    = (struct dirent *)NULL;
    char          pidstr[SSIZE] = "";


    /*--------------------------------------*/
    /* Assuming default P3 patchboard here! */
    /*--------------------------------------*/

    dirp = opendir("/tmp");
    (void)snprintf(pidstr,SSIZE,"%d",pid);

    while((next_item = readdir(dirp)) != (struct dirent *)NULL)
    {   if(strcmp(next_item->d_name,".") != 0 && strcmp(next_item->d_name,"..") != 0)
        {  if(strin(next_item->d_name,"psrp") == TRUE && strin(next_item->d_name,pidstr) == TRUE)
           {  (void)closedir(dirp);
              return(TRUE);
           }
        }
    }

    (void)closedir(dirp);
    return(FALSE);
}




/*--------------------------------------*/
/* Open pipe stream returning child pid */
/*--------------------------------------*/
                                                    /*--------------------------------*/
_PUBLIC des_t pups_popen(const char        *shell,  /* Shell to run command           */
                         const char          *cmd,  /* Command to run                 */
                         const int32_t   rw_flags,  /* r/w flags                      */
                         pid_t         *child_pid)  /* PID of child (payload) command */
                                                    /*--------------------------------*/
{   des_t fildes[2] = { [0 ... 1] = (-1) };

    if(shell     == (const char *)NULL  ||
       cmd       == (const char *)NULL  ||
       child_pid == (int        *)NULL   )
    {  pups_set_errno(EINVAL);
       return(-1);
    }


    /*---------------*/
    /* Create a pipe */
    /*---------------*/

    (void)pipe(fildes);
    if((*child_pid = fork()) == 0)
    {

       /*----------------------------*/ 
       /* Child on end 2 of the pipe */
       /*----------------------------*/
       /*-------------------------------------------------------*/
       /* Child operation must mirror that requested for parent */
       /*-------------------------------------------------------*/

       if(rw_flags == 0)
       {  (void)dup2(fildes[1],1);
          (void)dup2(fildes[1],2);
       }
       else
          (void)dup2(fildes[1],0);
       (void)close(fildes[1]);

       (void)execlp(shell,shell,"-c",cmd,0);

       (void)close(rw_flags);
       _exit(0);
   }

   if(kill(*child_pid,SIGCONT) == (-1))
   {  pups_set_errno(ENOEXEC);
      return(-1);
   }

   /*-------------------------*/
   /* Parent on end 1 of pipe */
   /*-------------------------*/

   (void)close(fildes[1]);

   pups_set_errno(OK);
   return(fildes[0]);
}




/*-----------------------------------------------*/
/* Open buffered pipe stream retruning child pid */
/*-----------------------------------------------*/
                                                 /*--------------------------------*/
_PUBLIC FILE *pups_fpopen(const char *shell,     /* Shell to run command           */
                          const char *cmd,       /* Command to run                 */
                          const char *rw_flags,  /* r/w flags                      */
                          pid_t      *child_pid) /* PID of child (payload) command */
                                                 /*--------------------------------*/

{   des_t  pdes     = (-1);
    FILE   *pstream = (FILE *)NULL;

    if(shell     == (const char *)NULL  ||
       cmd       == (const char *)NULL  ||
       rw_flags  == (const char *)NULL  ||
       child_pid == (pid_t      *)NULL   )
    {  pups_set_errno(EINVAL);
       return((FILE *)NULL);
    }

    if(strcmp((char *)rw_flags,"r") == 0)
    {  if((pdes = pups_popen(shell,cmd, 0, child_pid)) == (-1))
       {  pups_set_errno(ECHILD);
          return((FILE *)NULL);
       }

       if((pstream = fdopen(pdes,"r")) == (FILE *)NULL)
          return((FILE *)NULL);
    }
    else if(strcmp(rw_flags,"w") == 0)
    {  if((pdes = pups_popen(shell,cmd, 1, child_pid)) == (-1))
       {  pups_set_errno(ECHILD);
          return((FILE *)NULL);
       }

       if((pstream = fdopen(pdes,"w")) == (FILE *)NULL)
          return((FILE *)NULL);
    } 

    pups_set_errno(OK);
    return(pstream);
}




/*-------------------*/
/* Close pipe stream */
/*-------------------*/

_PUBLIC int32_t pups_pclose(const des_t pdes, const pid_t pid)

{   int32_t status;

    if(pdes <= 0 || pid <= 0)
    {   pups_set_errno(EINVAL);
       return(-1);
    }


    if(close(pdes) == (-1))
    {  pups_set_errno(EBADF);
       return(-1);
    }

    if(kill(pid,SIGTERM) == (-1))
    {  pups_set_errno(ECHILD);
       return(-1);
    }

    (void)waitpid(pid,&status,0);

    pups_set_errno(OK);
    return(0);
}




/*----------------------------*/
/* Close buffered pipe stream */
/*----------------------------*/

_PUBLIC int32_t pups_fpclose(const FILE *pstream, const pid_t pid)

{   des_t pdes = (-1);

    if(pstream == (const FILE *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    pdes = fileno(pstream);
    if(pups_pclose(pdes,pid) == (-1))
       return(-1);

    pups_set_errno(OK);
    return(0);
}




/*--------------------------------*/
/* Check if process in foreground */
/*--------------------------------*/

_PUBLIC _BOOLEAN pups_is_foreground(void)

{   if(tcgetpgrp(0) != getpgrp())
       return(FALSE);

    return(TRUE);
}




/*--------------------------------*/
/* Check if process in background */ 
/*--------------------------------*/

_PUBLIC _BOOLEAN pups_is_background(void)

{   if(tcgetpgrp(0) == getpgrp())
       return(FALSE);

    return(TRUE);
}




/*---------------------------------*/
/* Is command installed on system? */
/*---------------------------------*/

_PUBLIC _BOOLEAN pups_cmd_installed(const char *command)

{   char line[SSIZE]        = "",
         pstream_cmd[SSIZE] = "";

    FILE *pstream = (FILE *)NULL;

    if(command == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    (void)snprintf(pstream_cmd,SSIZE,"which %s",command);
    if((pstream = popen(pstream_cmd,"r")) == (FILE *)NULL)
    {  pups_set_errno(ENOEXEC);
       return(-1);
    }

    (void)fgets(line,SSIZE,pstream); 
    (void)pclose(pstream);
  
    pups_set_errno(OK); 
    if(strin(line,"Command not found") == TRUE)
       return(FALSE);
 
    return(TRUE); 
}




/*---------------------*/
/* Get CPU utilisation */
/*---------------------*/

_PUBLIC FTYPE pups_cpu_utilisation(void)

{    char  line[SSIZE]         = "",
           pstream_cmd[SSIZE]  = "";
     FILE  *pstream            = (FILE *)NULL;
     FTYPE utilisation         = 0.0;

     /*----------------------------------*/
     /* Is the mpstat command installed? */
     /*----------------------------------*/

     if(pups_cmd_installed("mpstat") == FALSE)
     {  pups_set_errno(ESRCH);
        return(-1.0);
     }

     (void)snprintf(pstream_cmd,SSIZE,"mpstat | tail -1 | awk '{print $11}'");


     /*--------------------------*/
     /* Find the CPU utilisation */
     /*--------------------------*/

     if((pstream = popen(pstream_cmd,"r")) == (FILE *)NULL)
     {   pups_set_errno(EINVAL);
         return(-1.0);
     } 

     (void)fgets(line,SSIZE,pstream);
     (void)pclose(pstream);


     /*-----------------------------*/
     /* Returns idle CPU percentage */
     /*-----------------------------*/

     if(sscanf(line,"%F",&utilisation) != 1)
     {   pups_set_errno(EINVAL);
         return(-1.0);
     }


     /*-----------------------------*/
     /* Get occupied CPU percentage */
     /*-----------------------------*/

     else
        utilisation = 100.0 - utilisation;

     pups_set_errno(OK);
     return(utilisation);    
}




/*------------*/
/* Get prefix */
/*------------*/

_PUBLIC int32_t pups_prefix(const char dm_ch, const char *s, char *strPrefix)

{   size_t i,
           size;

    if(s == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);
    for(i=size; i>0; --i)
    {  if(s[i] == dm_ch)
       {  (void)strlcpy(strPrefix,s,SSIZE);
          strPrefix[i] = '\0';
          pups_set_errno(OK);

          return(0);
       }
    }

    pups_set_errno(ESRCH);
    return(-1);
}




/*------------*/
/* Get suffix */
/*------------*/

_PUBLIC int32_t pups_suffix(const char dm_ch, const char *s, char *strSuffix)

{   size_t i,
           size;

    if(s == (const char *)NULL || strSuffix == (const char *)NULL)
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    size = strlen(s);
    for(i=size; i>0; --i)
    {  if(s[i] == dm_ch)
       {  (void)strlcpy(strSuffix,(char *)&s[i+1],SSIZE);
          pups_set_errno(OK);

          return(0);
       }
    }

    pups_set_errno(ESRCH);
    return(-1);
}




/*---------------*/
/* Get file size */
/*---------------*/

_PUBLIC ssize_t pups_get_fsize(const char *file_name)

{   ssize_t     size;
    struct stat buf;

    if(file_name == (const char *)NULL || access(file_name,F_OK | R_OK) == (-1))
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    (void)stat(file_name,&buf);
    size = (ssize_t)buf.st_size;

    pups_set_errno(OK);
    return(size);
}




/*--------------------------------------*/
/* Missing tests on an open descriptor  */
/* isatty() is often a blunt instrument */
/* if we need to know exactly what is   */
/* attached to a descriptor             */
/*--------------------------------------*/
/*-----------------------------------*/
/* Descriptor associated with a pipe */
/*-----------------------------------*/

_PUBLIC int32_t isapipe(const des_t fdes)

{   struct stat buf;

    if(fstat(fdes,&buf) < 0)
       return(-1);

    pups_set_errno(OK);
    if(S_ISFIFO(buf.st_mode))
       return(1);

    return(0);
}




/*-----------------------------------*/
/* Descriptor associated with a file */
/*-----------------------------------*/

_PUBLIC int32_t isafile(const des_t fdes)

{   struct stat buf;

    if(fstat(fdes,&buf) < 0)
       return(-1);

    pups_set_errno(OK);
    if(S_ISREG(buf.st_mode))
       return(1);

    return(0);
}




/*-------------------------------------*/
/* Descriptor associated with a socket */
/*-------------------------------------*/

_PUBLIC int32_t isasock(const des_t fdes)

{   struct stat buf;

    if(fstat(fdes,&buf) < 0)
       return(-1);

    pups_set_errno(OK);
    if(S_ISSOCK(buf.st_mode))
       return(1);

    return(0);
}



/*---------------------------------------------------------------------------*/
/* Is the descriptor associated with a data conduit ((pipe, file or socket)) */
/*---------------------------------------------------------------------------*/

_PUBLIC int32_t isaconduit(const des_t fdes)

{   if(isafile(fdes) == 1)
       return(1);
    else if(isapipe(fdes) == 1)
       return(1);
    else if(isasock(fdes) == 1)
       return(1);

    pups_set_errno(OK);
    return(0);
}




/*-----------------------------------------*/
/* Get system information for current host */
/*-----------------------------------------*/

_PUBLIC int32_t pups_sysinfo(char *hostname,         // Name of host
                             char *ostype,           // OS running on host
                             char *osversion,        // Version of OS running on host
                             char *machtype)         // Host (CPU) hardware

{   FILE     *pstream = (FILE *)NULL;

    char     strdum[SSIZE]   = "",
             line[SSIZE]     = "";

    sigset_t set,
             old_set;


    /*---------------*/
    /* Block signals */
    /*---------------*/

    (void)sigfillset(&set);
    (void)sigdelset(&set,SIGINT);
    (void)sigdelset(&set,SIGTERM);
    (void)sigprocmask(SIG_BLOCK,&set,&old_set);


    /*---------*/
    /* Machine */
    /*---------*/

    if(machtype != (char *)NULL)
    {  if((pstream = popen("uname --machine","r")) == (FILE *)NULL)
       {  (void)sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);
          pups_set_errno(ENOEXEC);
          return(-1);
       }

       (void)fgets(line,SSIZE,pstream);
       (void)pclose(pstream);
       (void)sscanf(line,"%s",machtype);
    }


    /*------------------*/
    /* Operating system */
    /*------------------*/

    if(ostype != (char *)NULL)
    {  if((pstream = popen("uname --operating-system","r")) == (FILE *)NULL)
       {  (void)sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);
          pups_set_errno(ENOEXEC);
          return(-1);
       }

       (void)fgets(line,SSIZE,pstream);
       (void)pclose(pstream);
       (void)sscanf(line,"%s",ostype);
    }


    /*--------------------------*/
    /* Operating system version */
    /*--------------------------*/

    if(osversion != (char *)NULL)
    {  if((pstream = popen("uname --kernel-release","r")) == (FILE *)NULL)
       {  (void)sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);
          pups_set_errno(ENOEXEC);
          return(-1);
       }

       (void)fgets(line,SSIZE,pstream);
       (void)pclose(pstream);
       (void)sscanf(line,"%s",osversion);
    }


    /*----------*/
    /* Hostname */
    /*----------*/

    if(hostname != (char *)NULL)
    {  if((pstream = popen("uname --nodename","r")) == (FILE *)NULL)
       {  (void)sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);
          pups_set_errno(ENOEXEC);
          return(-1);
       }

       (void)fgets(line,SSIZE,pstream);
       (void)pclose(pstream);
       (void)sscanf(line,"%s",hostname);
    }


    /*-----------------*/
    /* Restore signals */
    /*-----------------*/

    (void)sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);
    pups_set_errno(OK);
    return(0);
} 




/*-----------------------------------------*/
/* Am I running in container or other sort */
/* of virtual environment?                 */
/*-----------------------------------------*/

_PUBLIC _BOOLEAN pups_os_is_virtual(char *vmType)

{   char     line[SSIZE] = "";
    FILE     *pstream    = (FILE *)NULL;

    sigset_t set,
             old_set;

    pups_set_errno(OK);


    /*---------------*/
    /* Block signals */
    /*---------------*/

    (void)sigfillset (&set);
    (void)sigdelset  (&set,SIGINT);
    (void)sigdelset  (&set,SIGTERM);
    (void)sigprocmask(SIG_BLOCK,&set,&old_set);


    /*----------------------------------*/
    /* Open pstream (with sanity check) */
    /*----------------------------------*/
    /*----------------------------------------------*/
    /* Must add the following line to /etc/sudoers: */
    /* ALL ALL=NOPASSWD: /sbin/virt-what            */
    /*----------------------------------------------*/

    
    if((pstream = popen("sudo virt-what | head -1","r")) == (FILE *)NULL || vmType == (char *)NULL)
    {  (void)sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);
       pups_set_errno(EINVAL);
       return(-1);
    }

    (void)fgets(line,SSIZE,pstream);
    (void)sscanf(line,"%s",vmType);
    (void)fclose(pstream); 


    /*-----------------*/
    /* Restore signals */
    /*-----------------*/

    (void)sigprocmask(SIG_SETMASK,&old_set,(sigset_t *)NULL);


    /*-------------------------------*/
    /* OS is running on "bare metal" */
    /*-------------------------------*/

    if(strcmp(vmType,"") == 0)
       return(FALSE);


    /*-------------------------------*/
    /* OS is running in some kind of */
    /* virtual environment           */
    /*-------------------------------*/

    return(TRUE);
}




/*-----------------------------*/
/* Is architecture big endian? */
/*-----------------------------*/

_PUBLIC _BOOLEAN pups_bigendian(void) 
{
    uint32_t x  = 0x76543210;
    char     *c = (char*) &x;
 
  if(*c == 0x10)
     return(FALSE);
 
  return(TRUE);
}




#ifdef CRIU_SUPPORT
/*----------------------------*/
/* Enable (Criu) state saving */
/*----------------------------*/

_PUBLIC int32_t pups_ssave_enable(void)
{
     int32_t t_index;


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_ssave_enable] attempt by non root thread to perform PUPS/P3 utility operation");


    /*-------------------------------------------*/
    /* Create directory in /tmp for state saving */
    /*-------------------------------------------*/

    (void)snprintf(appl_criu_ssave_dir,SSIZE,"%s/pups.%s.%d.ssave",appl_criu_dir,appl_name,appl_pid);


    /*---------------------------------------------*/
    /* If checkpoint directory exists then we have */
    /* resumed from a checkpoint                   */
    /*---------------------------------------------*/

 
    if(access(appl_criu_ssave_dir,F_OK) == (-1))
    {
       if(mkdir(appl_criu_ssave_dir,0700) == (-1))


          /*----------------------------------------------*/
          /* Error - cannot create state saving directory */
          /*----------------------------------------------*/

          return(-1);
    }


    /*-------------------------------*/
    /* Note that we are saving state */
    /*-------------------------------*/

    appl_ssave = TRUE;


    /*------------------------------------------------*/
    /* Set polling time for state saving (in seconds) */
    /*------------------------------------------------*/

    t_index = pups_setvitimer("ssave_homeostat",
                              1,VT_CONTINUOUS,10,
                              (void *)NULL,
                              (void *)ssave_homeostat);


    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): ssave homeostat activated (polling via vtimer %d (payload function: ssave_homeostat)\n",
                                                                                  date,appl_name,appl_pid,appl_host,appl_owner,t_index);
       (void)fflush(stderr);
    }

    pups_set_errno(OK);
    return(0);
}




/*-----------------------------*/
/* Disable (Criu) state saving */
/*-----------------------------*/

_PUBLIC int32_t pups_ssave_disable(void)

{   char rm_cmd[SSIZE]    = "";


    /*----------------------------------*/
    /* Only the root thread can process */
    /* PSRP requests                    */
    /*----------------------------------*/

    if(pupsthread_is_root_thread() == FALSE)
       pups_error("[pups_ssave_enable] attempt by non root thread to perform PUPS/P3 utility operation");


    /*------------------*/
    /* Remove homeostat */
    /*------------------*/

    (void)pups_clearvitimer("ssave_homeostat");


    /*-------------------------------------------*/
    /* Remove directory in /tmp for state saving */
    /* and all its contents                      */
    /*-------------------------------------------*/

    (void)snprintf(rm_cmd,SSIZE,"rm -rf %s",appl_criu_ssave_dir);
    (void)system(rm_cmd);

    appl_criu_ssave  = FALSE;
    appl_criu_ssaves = 0;

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): ssave homeostat deactivated\n",
                                 date,appl_name,appl_pid,appl_host,appl_owner);
       (void)fflush(stderr);
    }

    pups_set_errno(OK);
    return(0);
}



/*-------------------------------------*/
/* Perioidically save state (via Criu) */
/*-------------------------------------*/

_PRIVATE  int32_t ssave_homeostat(void *t_info, char *args)

{   _IMMORTAL _BOOLEAN entered = FALSE;

    _IMMORTAL time_t   base_time,
                       elapsed_time;


    struct timespec delay;


    /*-----------------------------------------*/
    /* First time homeosat called - initialise */
    /*-----------------------------------------*/

    if(entered == FALSE)
    {  entered = TRUE; 
       base_time = time((time_t *)NULL);

       return(1);
    }

    elapsed_time = time((time_t *)NULL) - base_time;


    #ifdef UTILIB_DEBUG
    if(elapsed_time >= 5)
    {  (void)fprintf(stderr,"ELAPSED TIME: %ld\n",elapsed_time);
       (void)fflush(stderr);
    }
    #endif /* UTILIB_DEBUG */


    /*----------------------------------*/
    /* Save state if poll time exceeded */
    /*----------------------------------*/

    if(elapsed_time >= appl_poll_time)
    {  char criu_cmd[SSIZE]  = "";


       #ifdef UTILIB_DEBUG
       (void)fprintf(stderr,"ELAPSED TIME: %ld SAVING STATE (%d)\n",elapsed_time,appl_criu_ssaves);
       (void)fflush(stderr);
       #endif /* UTILIB_DEBUG */


       /*-------------------------------------*/
       /* Run Criu command to save state      */
       /* it must run in background so it     */
       /* is not included in the process tree */
       /* of the caller                       */
       /*-------------------------------------*/
       /*--------------------------------*/
       /* Kill server after saving state */
       /*--------------------------------*/

       if(args != (char *)NULL && strcmp(args,"kill") == 0)
          (void)snprintf(criu_cmd,SSIZE,"criu --log-file /dev/null --shell-job --link-remap dump -D %s -t %d &",appl_criu_ssave_dir,appl_pid);


       /*-----------------------------------------*/
       /* Server stays running after saving state */
       /*-----------------------------------------*/

       else
          (void)snprintf(criu_cmd,SSIZE,"criu --log-file /dev/null --leave-running --shell-job --link-remap dump -D %s -t %d &",appl_criu_ssave_dir,appl_pid);

       (void)system(criu_cmd);


       /*------------------------------------------*/
       /* This is a dummy system call - on restore */
       /* EINTR will be generated and the process  */
       /* will restart from here                   */
       /*------------------------------------------*/

       delay.tv_sec  = 60;
       delay.tv_nsec = 0;

       (void)nanosleep(&delay,(struct timespec *)NULL);

       #ifdef UTILIB_DEBUG
       (void)fprintf(stderr,"RESTART (checkpoint: %d)\n",appl_criu_ssaves);
       (void)fflush(stderr);
       #endif /* UTILIB_DEBUG */


       /*---------------------------------------*/
       /* Reset base time for new poll interval */
       /*---------------------------------------*/

       base_time = time((time_t *)NULL);
       ++appl_criu_ssaves; 


       /*-------------------*/
       /* Reschedule timers */
       /*-------------------*/

       (void)pups_malarm(vitimer_quantum);
    }

    return(0);
}
#endif /* CRIU_SUPPORT */




/*-------------*/
/* Copy a file */
/*-------------*/

_PUBLIC int32_t pups_copy_file(const int32_t copy_op, const char *from, const char *to)

{   uint64_t file_size,
             bytes_read,
             total_read         = 0L;

    _BYTE    buf[COPY_BUF_SIZE] = "";

     int32_t to_des             = (-1),
             from_des           = (-1);

    struct stat stat_buf;


    /*--------------*/
    /* Sanity check */
    /*--------------*/

    if(from == (const char *)NULL  ||
       to   == (const char *)NULL   )
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    if(copy_op != PUPS_FILE_MOVE    &&
       copy_op != PUPS_FILE_COPY     )
    {  pups_set_errno(EINVAL);
       return(-1);
    }

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_lock(&copy_mutex);
    #endif /* PTHREAD_SUPPORT */


    /*------------------*/
    /* Open source file */
    /*------------------*/

    if((from_des = open(from,O_RDONLY)) == (-1))
    {

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&copy_mutex);
       #endif /* PTHREAD_SUPPORT */

       return(-1);
    }


    /*-----------------------*/
    /* Open destination file */
    /*-----------------------*/

    if((to_des = open(to,O_WRONLY | O_CREAT, 0600)) == (-1))
    {

       #ifdef PTHREAD_SUPPORT
       (void)pthread_mutex_unlock(&copy_mutex);
       #endif /* PTHREAD_SUPPORT */

       (void)close(from_des);
       return(-1);
    }


    /*---------------*/
    /* Get file size */
    /*---------------*/

    (void)fstat(from_des,&stat_buf);
    file_size = stat_buf.st_size;


    /*---------------*/
    /* Copy the file */
    /*---------------*/

    do {    bytes_read = read(from_des,buf,COPY_BUF_SIZE);
            (void)write(to_des,buf,bytes_read);

            total_read += bytes_read;
       } while(total_read < file_size);

    (void)close(from_des);
    (void)close(to_des);


    /*---------------------------*/
    /* If we are moving the file */
    /* delete source file        */
    /*---------------------------*/

    if(copy_op == PUPS_FILE_MOVE)
       (void)unlink(from); 

    #ifdef PTHREAD_SUPPORT
    (void)pthread_mutex_unlock(&copy_mutex);
    #endif /* PTHREAD_SUPPORT */

    pups_set_errno(OK);
    return(0);
}




/*---------------------*/
/* Can we run command? */
/*---------------------*/

_PUBLIC _BOOLEAN pups_can_run_command(const char *cmd)
{

    if(strchr(cmd, '/'))
    {

        /*-----------------------------------------------------*/
        /* If cmd includes a slash, no path search is required */
        /* go straight to checking if it's executable          */
        /*-----------------------------------------------------*/

        return access(cmd, X_OK)==0;
    }

    const char *path = getenv("PATH");


    /*-------*/
    /* Error */
    /*-------*/

    if(path == (const char *)NULL)
       return(FALSE);


    /*------------------------------------------------*/
    /*  We are sure we won't need a buffer any longer */
    /*------------------------------------------------*/

    char *buf = (char *)malloc(PATH_MAX);

    if(buf == (char *)NULL)
       return(FALSE);

    /*--------------------------------------------------*/
    /* loop as long as we have stuff to examine in path */
    /*--------------------------------------------------*/

    for(; *path; ++path)
    {

        /*----------------------------------------*/
        /* start from the beginning of the buffer */
        /*----------------------------------------*/

        char *p = buf;


        /*------------------------------------*/
        /* Copy current path element into buf */
        /*------------------------------------*/

        for(; *path && *path!=':'; ++path,++p)
            *p = *path;


        /*-----------------------------------------*/
        /* empty path entries are treated like "." */
        /*-----------------------------------------*/

        if(p == buf)
           *p++='.';


        /*------------------------*/
        /* slash and command name */
        /*------------------------*/

        if(p[-1] != '/')
           *p++='/';

        (void)strlcpy(p, cmd,PATH_MAX);


        /*----------------------------*/
        /* check if we can execute it */
        /*----------------------------*/

        if(access(buf, X_OK)==0) {
           (void)free((void *)buf);
           return(TRUE);
        }


        /*------------------------*/
        /* Quit in last iteration */
        /*------------------------*/

        if(!*path)
           break;
    }


    /*-------------------*/
    /* Command not found */
    /*-------------------*/

    (void)free((void *)buf);
    return(FALSE);
}



/*---------------------------*/
/* Enable (memory) residency */
/*---------------------------*/

_PUBLIC void pups_enable_resident(void)

{   appl_enable_resident = TRUE;
}




/*--------------------------------*/
/* Get size of directory in bytes */
/*--------------------------------*/

_PUBLIC size_t pups_dsize(const char *directory_name)
{
    off_t directory_size = 0;
    DIR   *pDir          = (DIR *)NULL;

    if ((pDir = opendir(directory_name)) != (DIR *)NULL)
    {
        struct dirent *pDirent = (struct dirent *)NULL;

        /*--------------------------------*/
	/* Get size of files in directory */
	/*--------------------------------*/

        while ((pDirent = readdir(pDir)) != (struct dirent *)NULL)
        {
            char   buffer[PATH_MAX + 1] = "";
            struct stat file_stat;

            (void)strlcpy(buffer, directory_name,SSIZE);
            (void)strlcat(buffer, "/", SSIZE);
            (void)strlcat(buffer,pDirent->d_name, SSIZE);

            if (stat(buffer, &file_stat) == 0)
                directory_size += file_stat.st_blocks * S_BLKSIZE;


	    /*-----------------*/
	    /* Sub-directory ? */
	    /*-----------------*/

            if (pDirent->d_type == DT_DIR)
            {  

	       /*-----*/
	       /* Yes */
	       /*-----*/

	       if (strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0)
                    directory_size += pups_dsize(buffer);
            }
        }

        (void) closedir(pDir);
    }


    /*------------------------*/
    /* Size of directory tree */
    /* in bytes               */
    /*------------------------*/

    return(directory_size);
}




/*--------------------------*/
/* Software watchdog status */
/*--------------------------*/

_PUBLIC int32_t pups_show_softdogstatus(const FILE *stream)

{

    /*-----------------------------*/
    /* Software watchdog is active */
    /*-----------------------------*/

    if (appl_softdog_enabled == TRUE)
    {  (void)fprintf(stream,"\n    softdog: software watchdog is enabled (pid: %d timeout: %d seconds)\n\n",appl_softdog_pid,appl_softdog_timeout);
       (void)fflush(stream);
    }


    /*---------------------------------*/
    /* Software watchdog is not active */
    /*---------------------------------*/

    else
    {  (void)fprintf(stream,"\n    softdog: software watchdog is disabled (timeout: %d seconds)\n\n",appl_softdog_timeout);
       (void)fflush(stream);
    }

    pups_set_errno(OK);
    return(0);
}



/*--------------------------------------*/
/* Perioidically kick software watchdog */
/*--------------------------------------*/

_PRIVATE int32_t softdog_homeostat(void *t_info, const char *args)

{

    /*------------------------*/
    /* Kick software watchdog */
    /*------------------------*/

    if (kill(appl_softdog_pid,SIGCONT) != 0)
    {  
	    
       /*-------------------------------------*/	
       /* Software watchdog lost - restart it */
       /*-------------------------------------*/

       (void)pups_start_softdog(appl_softdog_timeout);

       if(appl_verbose == TRUE)
       {  (void)strdate(date);
          (void)fprintf(stderr,"%s %s (%d@%s:%s): software watchdog lost -- restarting\n",
                                             date,appl_name,appl_pid,appl_host,appl_owner);
          (void)fflush(stderr);
       }
    }

    return(0);
}



/*-------------------------*/
/* Start software watchdog */
/*-------------------------*/

_PUBLIC int32_t pups_start_softdog(const uint32_t timeout)

{    int32_t t_index  = (-1);


     /*-------*/
     /* Child */
     /*-------*/

     if ((appl_softdog_pid = pups_fork(FALSE,FALSE)) == 0)
     {  char applpidstr[SSIZE]  = "",
                      timeoutstr[SSIZE]  = "";


        /*---------------------------------*/
        /* Cancel timers so child does not */
        /* get signalled                   */
        /*---------------------------------*/

        (void)pups_malarm(0);


	/*---------------------------------*/
	/* Exec software watchdog as child */
	/*---------------------------------*/

        (void)snprintf(applpidstr,SSIZE,"%d",appl_pid);
        (void)snprintf(timeoutstr,SSIZE,"%d",timeout);
        (void)execlp("softdog","softdog","-monitorpid",applpidstr,"-timeout",timeoutstr,(char *)NULL);


	/*------------------------*/
	/* We should not get here */
	/*------------------------*/

	_exit(255);
     }


     /*--------*/
     /* Parent */
     /*--------*/

     else


        /*------------------------------------------------*/
        /* Set polling time software watchdog (in seconds) */
        /*------------------------------------------------*/

        t_index = pups_setvitimer("softdog_homeostat",
                                  1,VT_CONTINUOUS,500,
                                  (void *)NULL,
                                  (void *)softdog_homeostat);



     if(appl_verbose == TRUE)
     {  (void)strdate(date);
        (void)fprintf(stderr,"%s %s (%d@%s:%s): softdog homeostat enabled (polling via vtimer %d (payload function: softdog_homeostat)\n",
                                                                                     date,appl_name,appl_pid,appl_host,appl_owner,t_index);
        (void)fflush(stderr);
     }
   
     appl_softdog_enabled = TRUE;

     pups_set_errno(OK);
     return(0);
}



/*------------------------*/
/* Stop software watchdog */
/*------------------------*/

_PUBLIC int32_t pups_stop_softdog(void)

{   int32_t ret = (-1);


    /*-----------------------------*/
    /* Terminate software watchdog */
    /*-----------------------------*/

    pups_set_errno(OK);
    ret = kill(appl_softdog_pid,SIGTERM);


    /*------------------*/
    /* Remove homeostat */
    /*------------------*/

    (void)pups_clearvitimer("softdog_homeostat");
    appl_softdog_enabled = FALSE;
    appl_softdog_pid     = (-1);

    if(appl_verbose == TRUE)
    {  (void)strdate(date);
       (void)fprintf(stderr,"%s %s (%d@%s:%s): softdog homeostat disabled\n",date,appl_name,appl_pid,appl_host,appl_owner);
       (void)fflush(stderr);
    }
   
    pups_set_errno(OK);
    return(0);
}




/*--------------------------------------*/
/* Non-blocking poll of file descriptor */
/*--------------------------------------*/

_PUBLIC _BOOLEAN pups_fd_has_data(const des_t fildes)

{  int32_t ret;
   struct  pollfd fd;


   fd.fd = fildes;

                        /*-----------------------*/
   fd.events = POLLIN;  /* Poll for data to read */
                        /*-----------------------*/

   fd.revents = 0;
   ret = poll(&fd, 1, 0);


   /*----------------*/
   /* Data available */
   /*----------------*/

   if(ret > 0 && (fd.revents & POLLIN != 0))
   {  pups_set_errno(OK);
      return(TRUE);
   }


   /*--------------------------------------*/
   /* Bad descriptor state                 */
   /* read from wrtie only descriptor etc. */
   /*--------------------------------------*/

   if(ret == (-1))
      pups_set_errno(EBADFD);


   /*-------------------*/
   /* No data available */
   /*-------------------*/

   else
      pups_set_errno(OK);

   return(FALSE);
}

