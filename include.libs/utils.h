/*------------------------------------------- 
    Header for the standard utilities library

    Author:  M.A. O'Neill
             Tumbling Dice Ltd
             Gosforth
             Newcastle upon Tyne
             NE3 4RT
             United Kingdom

    Version: 8.11
    Dated:   2nd January 2025 
    E-Mail:  mao@tumblingdice.co.uk
-------------------------------------------*/


#ifndef UTILS_H
#define UTILS_H


/*------------------*/
/* Keep c2man happy */
/*------------------*/

#include <c2man.h>

#ifndef __C2MAN__
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <termios.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <setjmp.h>


#ifdef _OPENMP
#include <omp.h>
#endif /* _OPENMP */

#endif /* __C2MAN__ */


/***********/
/* Version */
/***********/

#define UTILIB_VERSION              "8.11"


/******************/
/* P3 application */
/******************/

#define P3


/*-------------*/
/* String size */
/*-------------*/

#define SSIZE                       512 


/*-------------*/
/* Sleep delay */
/*-------------*/

#define SHORT_DELAY                 1
#define DELAY                       100
#define MEDIUM_DELAY                1000
#define LONG_DELAY                  100000


/*-------------------------------*/
/* Floating point representation */
/*-------------------------------*/

#include <ftype.h>


/*------------------------*/
/* Default Criu poll time */
/* (between state saves)  */
/*------------------------*/


#define DEFAULT_CRIU_POLL_TIME      60


/*-----------------------------------*/
/* Default software watchdog timeout */
/*-----------------------------------*/

#define DEFAULT_SOFTDOG_TIMEOUT     60



/*----------------------------------------------------------*/
/* Default resource allocations (for an application to run) */
/*----------------------------------------------------------*/

#define PUPS_DEFAULT_MIN_CPU         2.0 
#define PUPS_DEFAULT_MIN_MEM         10000
#define PUPS_DEFAULT_RESOURCE_WAIT   3600
#define PUPS_IGNORE_MIN_MEM          (-1)


/*-------------------*/
/* Checkpooint flags */
/*-------------------*/

#define CKPT_INCREMENTAL    (1 << 0)
#define CKPT_EXIT           (1 << 1) 
#define CKPT_VERBOSE        (1 << 2) 
#define CKPT_WRAP_FILES     (1 << 3) 
#define CKPT_CHILDREN       (1 << 4) 


/*------------------------*/
/* Checkpoint return type */
/*------------------------*/

#define CKPT_DONE           (1 << 0)
#define CKPT_RESTART        (1 << 1)


/*-----------------*/
/* File copy flags */
/*-----------------*/

#define PUPS_FILE_COPY      (1 << 0) 
#define PUPS_FILE_MOVE      (1 << 1) 
#define COPY_BUF_SIZE       32768 



/*------------------------------------*/
/* File transfer modes (for pups_scp) */
/*------------------------------------*/

#define FROMC        1  // Copy from remote 
#define FROMX        2  // Transfer from remote
#define TOC          3  // Copy to remote 
#define TOX          4  // Transfer to remote 


/*---------------------------------*/
/* Support for compressing streams */
/*---------------------------------*/

#ifdef ZLIB_SUPPORT
#include <zlib.h>
#endif /* SUPPORT_ZLIB */


/*------------------------------------------------------------*/
/* Quantum for PUPS virtual interval timers (in microseconds) */
/*------------------------------------------------------------*/

#define VITIMER_QUANTUM    10000


/*------------------------------------------------*/
/* Backtrack exit code (from SIGSEGV backtracker) */
/*------------------------------------------------*/

#define PUPS_BACKTRACK     (-9999)


/*----------------------*/
/* Link file lock types */
/*----------------------*/

#define LOCK               1  
#define EXLOCK             1  
#define WRLOCK             2
#define RDLOCK             4



/*----------------------------*/
/* pups_lockf lock operations */
/*----------------------------*/

#define PUPS_UNLOCK        0
#define PUPS_RDLOCK        (1 << 0)
#define PUPS_TRY_RDLOCK    (1 << 1) 
#define PUPS_WRLOCK        (1 << 2) 
#define PUPS_TRY_WRLOCK    (1 << 3) 


/*--------------------------------*/
/* Wait forever (to acquire lock) */
/*--------------------------------*/

#define WAIT_FOREVER       (-1)


/*----------------------------------------------------*/
/* Try lock (but don't wait if it cannot be acquired) */
/*----------------------------------------------------*/

#define TRYLOCK            0


/*--------------------------------------------------*/
/* Maximum number of (concurrent) locks per process */
/*--------------------------------------------------*/

#define PUPS_MAX_LOCKS   512 


/*-------------------------------------------*/
/* Maximum number of flock locks per process */
/*-------------------------------------------*/

#define PUPS_MAX_FLOCKS  512 



/*-----------------------------------------------*/
/* Defer process exit when pups_exit() is called */
/*-----------------------------------------------*/

#define PUPS_DEFER_EXIT   9999


/*------------------------------------------*/
/* Offset used in PUPS file protection mode */
/*------------------------------------------*/

#define PUPS_PROT_OFFSET  (-1000)


/*---------------*/
/* Load averages */
/*---------------*/

#define LOAD_AVERAGE_1     1
#define LOAD_AVERAGE_2     2
#define LOAD_AVERAGE_3     3


/*----------------------------------*/
/* Error code to return if no error */
/*----------------------------------*/

#undef  OK
#define OK                 0


/*-------------------------------------*/
/* Define homeostasis revocation flags */
/*-------------------------------------*/

#define PUPS_HPROT_NOREVOKE  0
#define PUP_HPROT_REVOKE     1




/*-----------------*/
/* Version control */
/*-----------------*/

#define NO_VERSION_CONTROL (-1)


/*--------------------------*/
/* Memory allocator options */
/*--------------------------*/

#define MALLOC_DUMB         0  // Ordinary xmalloc semantics
#define MALLOC_HOMEOSTATIC  1  // Homeostatic xmalloc semantics


/*--------------------------------------------------------------*/
/* These definitions are used by initftab() for stdio homeostat */
/* initialisation                                               */
/* -------------------------------------------------------------*/

#define STDIO_DEAD          1  // Use standard fork call
#define STDIO_LIVE          2  // Use virtual fork call


/*-----------------------------------------*/
/* Signals to be held by pupshold function */
/*-----------------------------------------*/

#define ALL_PUPS_SIGS       (2 << 0) 
#define PSRP_SIGS           (2 << 1)
#define ALL_SIGS            (2 << 2) 


/*--------------------------------*/
/* File system object lock states */
/*--------------------------------*/

#define GETLOCK             (1 << 1) 
#define TSTLOCK             (1 << 2) 


/*--------------------------*/
/* Dynamic homeostat states */
/*--------------------------*/

#define FILE_RELOCATE       (1 << 0)
#define HOST_RENAME         (1 << 1) 
#define LOST_FILE_LOCATE    (1 << 2)


/*--------------------------*/
/* Extended file open flags */
/*--------------------------*/

#ifdef SUPPORT_MOPI
#define O_MOPI    65535
#endif /* SUPPORT_MOPI */

#define O_DUMMY             32768 
#define O_LIVE              16384


/*-----------------------------------------*/
/* Return codes for PUPS statkill function */
/*-----------------------------------------*/

#define PUPS_EXEC           0
#define PUPS_TERMINATED     (-1)
#define PUPS_STOPPED        (-2)


/*-----------------------------------------*/
/* Homeostatic (file system) object states */
/*-----------------------------------------*/

#define DEAD                (1 << 1) 
#define LIVE                (1 << 2) 


/*-----------------------------------------------------------------------*/
/* Maximum number of attempts to re-connect to a resource which has been */
/* lost                                                                  */
/* ----------------------------------------------------------------------*/

#define TRY_TIMEOUT         1 
#define MAX_TRYS            4 


/*------------------------------------------------*/
/* Maximum number of open files (per application) */
/*------------------------------------------------*/

#define MAX_FILES           512 


/*---------------------------*/
/* Maximum number of signals */
/*---------------------------*/

#define MAX_SIGS            68 


/*------------------------------------------------------------------------------
    Maximum number of children ...
------------------------------------------------------------------------------*/

#define MAX_CHILDREN        512 


/*------------------------------------------------*/
/* Timer modes (for PUPS virtual interval timers) */
/*------------------------------------------------*/

#define MAX_VTIMERS         512 

#define VT_NONE             0
#define VT_ONESHOT          1
#define VT_CONTINUOUS       2


/*-----------------------------------------------------*/
/* Allocation quantum for dynamically allocated arrays */
/*-----------------------------------------------------*/

#define ALLOC_QUANTUM       64




/*-------------------------------------*/
/* Definition of signal parameter type */
/*-------------------------------------*/

typedef struct {   int32_t  pid;                             // (Remote) pid to signal
                   int32_t  signum;                          // Signal to be sent
                   int32_t  r_pid;                           // PID of remote client
                   int32_t  uid;                             // Id. of clients owner
                   char     r_host[SSIZE];                   // Remote host name
                   char     r_client[SSIZE];                 // Remote client name
               } sigparams_type;


/*----------------------------------*/
/* Definition of virtual timer type */
/*----------------------------------*/

typedef struct {   int32_t  priority;                        // Priority of timer
                   char     name[SSIZE];                     // Name of payload func
                   int32_t  mode;                            // Mode of timer
                   int32_t  prescaler;                       // Interval prescaler
                   int32_t  interval_time;                   // Length of  interval
                   char     handler_args[SSIZE];             // Handler argument string
                   void     (*handler)(void *, char *);      // Handler function
               } vttab_type;



/*-------------------------------*/
/* Definition of file table type */
/*-------------------------------*/

typedef struct {   _BOOLEAN psrp;                            // Is this a PSRP stream?
                   _BOOLEAN pheap;                           // Is this a persistent heap?
                   _BOOLEAN creator;                         // TRUE if creator
                   _BOOLEAN named;                           // File (inode) is named
                   _BOOLEAN locked;                          // TRUE if file locked
                   _BOOLEAN mounted;                         // TRUE if on mounted fs
                   des_t    fdes;                            // File descriptor
                   int32_t  id;                              // Entry id tag 
                   int32_t  mode;                            // File access mode 
                   int32_t  homeostatic;                     // Is this a live stream?
                   pid_t    rd_pid;                          // Remote service daemon
                   pid_t    fifo_pid;                        // Pipestream leaders PID
                   int32_t  fs_blocks;                       // Min fs blocks for write
                   int32_t  st_mode;                         // File protections
                   int32_t  lost_cnt;                        // Loss count for file
                   FILE     *stream;                         // Stdio stream

#ifdef ZLIB_SUPPORT
                   gzFile   zstream;                         // Compression stream
#endif /* SUPPORT_ZLIB */

                   char     hname[SSIZE];                    // Handler's name
                   char     fname[SSIZE];                    // File name (if any)
                   char     fs_name[SSIZE];                  // File system name
                   char     fshadow[SSIZE];                  // File shadow
                   char     rd_host[SSIZE];                  // RSD host (for rd_pid)
                   char     rd_ssh_port[SSIZE];              // RSD host ssh port (for rd_pid)
                   _BOOLEAN (*homeostat)(int32_t,  int32_t); // Dynamic fs. homeostat
                   void     (*handler)(void *, char *);      // Handler
               } ftab_type;


/*---------------------------------*/
/* Definition of signal table type */
/*---------------------------------*/

typedef struct {   uint64_t  haddr;                           // Signal handler address
                   char      hname[SSIZE];                    // Signal handler name
                   sigset_t  sa_mask;                         // Sigs blocked in handler
                    int32_t  sa_flags;                        // Signal handler flags
               } sigtab_type;


/*------------------------------------------*/
/* Definition of dynamic memory object type */
/*------------------------------------------*/

typedef struct {  uint64_t   size;                            // Size of object (bytes)
                  int32_t    n_el;                            // Number of elements
                  void       *ptr;                            // Pointer to object
                  char       name[SSIZE];                     // Object name
                  char       type[SSIZE];                     // Object type
               } matab_object_type;


/*-----------------------------------------*/
/* Definition of dynamic memory table type */
/*-----------------------------------------*/

typedef struct {   int32_t           allocated;              // Objects in table
                   matab_object_type **object;               // Dynamic memory objects
               } matab_type;


/*---------------------------------*/
/* Definitions of child table type */
/*---------------------------------*/

typedef struct {   pid_t    pid;                             // Child pid
                   _BOOLEAN obituary;                        // If TRUE log child death
                   char     name[SSIZE];                     // Child name
               } chtab_type;


/*-----------------------*/
/* Seal pipeline on exit */
/*-----------------------*/

#define PUPS_PIPE_DETACH   9998


/*------------------------------------------------------*/
/* Error parameter definitions (for PUPS error handler) */
/*------------------------------------------------------*/

#define NONE                1
#define PRINT_ERROR_STRING  2
#define EXIT_ON_ERROR       4
#define CHILD_EXIT_ON_ERROR 8


/*---------------------------*/
/* Indicates a free resource */
/*---------------------------*/

#define FREE               (-9999)


/*-----------------------------------------------*/
/* Defines used by string manipulation functions */
/*-----------------------------------------------*/

#define N_STREXT_STRINGS   64
#define RESET_STREXT       (void)strext('*',(char *)NULL,(char *)NULL)
#define RESET_MSTREXT      1,(char *)NULL,(char *)NULL
#define END_STRING         (-9999) 
#define STREXT_ERROR       (-8888)
#define MSTREXT_RESET_ALL  1
#define MSTREXT_RESET_CUR  2


/*-------------------------------*/
/* Monitor function return codes */
/*-------------------------------*/

#define TIMED_OUT          0
#define READ_DATA          1
#define WRITE_DATA         2
#define EXCEPTION_RAISED   4 


/*---------------------------------------*/
/* Defines used by utilib error handlers */
/*---------------------------------------*/

#define PUPS_ERROR         (-1)
#define PUPS_OK            0
#define MAX_FUNCS          512 


/*-----------------------------------*/
/* Defines used by utilities library */
/*-----------------------------------*/

#define STRING_LENGTH      1024 
//#define LONG_LINE          1024
#define LONG_LONG_LINE     4096
#define NOT_FOUND          (-1)
#define INVALID_ARG        (-9999)
#define PI                 3.14159265
#define DEGRAD             0.01745329
#define RADDEG             57.29578778
#define ASSUMED_ZERO       1.0e-128


/*---------------------------------------------*/
/* Maximum number of arguments in command tail */
/*---------------------------------------------*/

#define ARGS               256
#define MAX_CMD_ARGS       253


#ifdef __NOT_LIB_SOURCE__
/*--------------------------------*/
/* External variable declarations */
/*--------------------------------*/

_IMPORT jmp_buf        appl_resident_restart;                  // Restart location (for memory resident application)

_EXPORT char           boldOn[];                               // Make character bold
_EXPORT char           boldOff[];                              // Make charavter non-bold

#ifdef SSH_SUPPORT
_EXPORT char           ssh_remote_port[];                      // Remote (ssh) port
_EXPORT char           ssh_remote_uname[];                     // Remote (ssh) username
_EXPORT _BOOLEAN       ssh_compression;                        // Use ssh compression
#endif /* SSH_SUPPORT */

_EXPORT _BOOLEAN       pups_abort_restart;                     // TRUE if abort handler enabled
_EXPORT  sigjmp_buf    pups_restart_buf;                       // Abort restart buffer
_EXPORT _BOOLEAN       fs_write_blocked;                       // TRUE if filesystem is full

#ifdef CRIU_SUPPORT
_EXPORT _BOOLEAN       appl_criu_ssave;                        // TRUE if (criu) state saving enabled
#endif /* CRIU_SUPPORT */

_EXPORT pid_t          appl_pid,                               // Process i.d.
                       appl_ppid,                              // (Effective) parent PID
                       appl_remote_pid,                        // Remote PID to relay signals to
                       appl_softdog_pid,                       // Software watchdog process PID
                       appl_sid;                               // Session (process group) i.d. 

_EXPORT uid_t          appl_uid;                               // Process UID (user i.d.)
_EXPORT gid_t          appl_gid;                               // Process group i.d.

_EXPORT  int32_t       sargc,                                  // Number of arguments in command tail
                       ptr,                                    // Pointer to current option
                       t_args,                                 // Total args in generic command tail
                       vitimer_quantum,                        // Timer quantum (for PSRP timers)
                       appl_fsa_mode,                          // Filesystem access mode
                       appl_t_args,                            // Total args in command tail
                       appl_alloc_opt,                         // Memory allocator options (for xmalloc etc)
                       appl_max_files,                         // Maximum number of ftab slots
                       appl_max_child,                         // Maximum number of child slots
                       appl_max_pheaps,                        // Maximum number of persistent heaps
                       appl_max_vtimers,                       // Maximum number of virtual timers
                       appl_max_orifices,                      // Maximum number of orifice [DLL] slots
                       appl_vtag,                              // Application (compile) version tag
                       base_arg,                               // Base argument for argfile ops
                       appl_last_child,                        // Last child process forked by xsystem
                       appl_tty,                               // Process controlling terminal descriptor


                       #ifdef CRIU_SUPPORT
                       appl_criu_ssaves,                       // Number of times state has been saved
                       appl_poll_time,                         // Pol time for (Criu) state saving
                       #endif /* CRIU_SUPPORT */


                       #ifdef _OPENMP
                       appl_omp_threads,                       // Number of OMP threads
                       #endif /* _OPENMP */


                       appl_nice_lvl,                          // Process niceness
                       appl_softdog_timeout;                   // Software watchdog timeout


                       #ifdef PTHREAD_SUPPORT
_EXPORT pthread_t      appl_root_tid;                          // LWP i.d. of root thread
                       #endif /* PTHREAD_SUPPORT */


_EXPORT _BOOLEAN       appl_verbose,                           // TRUE if verbose mode selected
                       appl_resident,                          // Application is memory resident 
                       appl_enable_residence,                  // Application can be memory resident
                       appl_etrap,                             // TRUE if error trapping for gdb analysis
                       appl_proprietary,                       // TRUE if application is proprietary


                       #ifdef MAIL_SUPPORT
                       appl_mailable,                          // TRUE if mail can be sent to process
                       #endif /* MAIL_SUPPORT */


                       test_mode,                              // TRUE if test mode
                       init,                                   // TRUE if first option
                       appl_pg_leader,                         // TRUE if process group leader
                       appl_nodetach,                          // Don't detach stdio in background
                       appl_fgnd,                              // TRUE if process in foreground process group
                       appl_snames_crypted,                    // TRUE if shadow filenames crypted
                       appl_wait,                              // TRUE if process waiting (SIGSTOP)
                       appl_psrp,                              // TRUE if application PSRP enabled
                       appl_ppid_exit,                         // If TRUE exit if parent terminates 
                       appl_rooted,                            // IF TRUE system context cannot migrate
                       appl_psrp_load,                         // If TRUE load PSRP resources at startup
                       appl_psrp_save,                         // If TRUE save dispatch table at exit
                       appl_have_pen,                          // TRUE if binname != exec name
                       appl_default_chname,                    // TRUE if PSRP channel name default
                       appl_softdog_enabled,                   // TRUE if software watchdog enabled
                       argd[];                                 // Argument decode status flags

_EXPORT char           version[],                              // Version of program
                       appl_owner[],                           // Name of application owner
                       appl_password[],                        // Password of application owner
                       appl_name[],                            // Application (process) name of program 
                       appl_logfile[],                         // Error/log file 
                       appl_bin_name[],                        // Application (binary) name of program
                       appl_remote_host[],                     // Remote host to relay signals to


                       #ifdef MAIL_SUPPORT
                       appl_mdir[],                            // Application mailbox
                       appl_mh_folder[],                       // Application MH (MIME) folder
                       appl_mime_dir[],                        // Process MIME workspace
                       appl_mime_type[],                       // MIME message part type (to retreive)
                       appl_replyto[],                         // Applicationm reply to address (for mail)
                       #endif /* MAIL_SUPPORT */


                       appl_fifo_dir[],                        // Application default FIFO patchboard directory
                       appl_fifo_topdir[],                     // Directory containing application patchboards
                       appl_ch_name[],                         // Application PSRP channel name
                       appl_err[],                             // Application error string
                       appl_hostpool[],                        // Application capabilities database
                       appl_ben_name[],                        // Application binary
                       appl_pam_name[],                        // PUPS authentication module name
                       appl_ttyname[],                         // Applications controlling terminal
                       appl_uprot_tag[],                       // Homeostasis (file) revocation tag
                       author[],                               // Author of program
                       revdate[],                              // Date of last modification
                       shell[],                                // Application shell
                       appl_home[],                            // Effective home directory
                       appl_cwd[],                             // Application current working directory
                       appl_cmd_str[],                         // Application command string
                       appl_host[],                            // Host node running this application
                       appl_state[],                           // State of this application
                       arg_f_name[],                           // Argument file name
                       appl_tunnel_path[],                     // Tunnel path (binary for tunnel process
                       appl_argfifo[],                         // Argument FIFO (for memory resident process)
                       *args[ARGS];                            // Secondary argument vectors


                        #ifdef CRIU_SUPPORT
_EXPORT _BOOLEAN        checkpointing;                         // Checkpointing flag
_EXPORT char            appl_criu_ssave_dir[];                 // Criu checkpoint directory for state saving
_EXPORT char            appl_criu_dir[];                       // Criu directory (holds migratable files and checkpoints) 
                        #endif /* CRIU_SUPPORT */


_EXPORT char            date[];                                // Date and time stamp
_EXPORT char            errstr[];                              // Error string
_EXPORT char            appl_machid[];                         // Unique machine (host) identifier
_EXPORT void            *exit_function;                        // Exit function to be called by SIGTERM


                         #ifdef PTHREAD_SUPPORT 
_EXPORT pthread_mutex_t  pups_fork_mutex;                      // Thread safe pups_fork mutex
_EXPORT pthread_mutex_t  ftab_mutex;                           // Thread safe file table mutex
_EXPORT pthread_mutex_t  chtab_mutex;                          // Thread safe child table mutex
                         #endif /* PTHREAD_SUPPORT */


_EXPORT _BOOLEAN         appl_secure;                          // TRUE if application secure
_EXPORT _BOOLEAN         appl_kill_pg;                         // If TRUE kill process group on exit
_EXPORT _BOOLEAN         pups_process_homeostat;               // If TRUE kill process group on exit 
_EXPORT _BOOLEAN         ftab_extend;                          // TRUE if extending file table
_EXPORT ftab_type        *ftab;                                // PUPS file table
_EXPORT chtab_type       *chtab;                               // PUPS child (process) table
_EXPORT sigtab_type      sigtab[MAX_SIGS];                     // Addresses of signal handlers
_EXPORT vttab_type       *vttab;                               // Virtual timer table
_EXPORT _BOOLEAN         default_fd_homeostat_action;          // Default action for stdio (FIFO) redirection 
_EXPORT _BOOLEAN         ignore_pups_signals;                  // Ignore PUPS/P3 protocol signals if TRUE
_EXPORT _BOOLEAN         in_pups_exit;                         // TRUE if in pups_exit()
_EXPORT _BOOLEAN         in_vt_handler;                        // True if in vt_handler()


#ifdef MAIL_SUPPORT
/*----------------------------*/
/* Application mail handler   */
/*----------------------------*/

_EXPORT  int32_t (*appl_mail_handler)(char *);
#endif /* MAIL_SUPPORT */

#else /* External variables */
#   undef  _EXPORT
#   define _EXPORT
#endif /* __NOT_LIB_SOURCE__ */


/*--------------------------------*/
/* Prototype function definitions */
/*--------------------------------*/

// Thread aware sigprockmask()
_PROTOTYPE _EXPORT int32_t pups_sigprocmask(int32_t, const sigset_t *restrict, sigset_t *restrict);

// Get time (accurate to milliseconds)
_PROTOTYPE _EXPORT double millitime(void);

// Get node name and I.P. address associated with network  interface
_PROTOTYPE _EXPORT int32_t pups_get_ip_info(const char *, char *, char *);

// Return the position of nth occurence of character in string (relative to start) 
_PROTOTYPE _EXPORT ssize_t ch_index(const char *, const char);

// Return the position of nth occurence of character in string (relative to end) 
_PROTOTYPE _EXPORT ssize_t rch_index(const char *, const char);

// Extended getpwnam routine (which searches NIS passwd map) [not thread safe]
_PROTOTYPE _EXPORT struct passwd *pups_getpwnam(const char *);

// Extended getpwuid routine (which searches NIS passwd map) [not thread safe]
_PROTOTYPE _EXPORT struct passwd *pups_getpwuid(const uid_t);

// Set up support child prior to performing dangerous operation
_PROTOTYPE _EXPORT pid_t pups_process_homeostat_enable(const _BOOLEAN);

// Cancel support child (at end of dangerous operation)
_PROTOTYPE _EXPORT void pups_process_hoemostat_disable(const pid_t);

// Clear a virtual timer datastructure [root thread]
_PROTOTYPE _EXPORT int32_t pups_clear_vitimer(const uint32_t);

// Get file table index (associated with a file descriptor)
_PROTOTYPE _EXPORT int32_t pups_get_ftab_index_from_fd(const des_t);

// Get file table index (associated with a stream)
_PROTOTYPE _EXPORT  int32_t pups_get_ftab_index_from_stream(const FILE *);

// Restart virtual interval timers [root thread]
_PROTOTYPE _EXPORT int32_t pups_vitrestart(void);

// Find free file table index
_PROTOTYPE _EXPORT int32_t pups_find_free_ftab_index(void);

#ifdef ZLIB_SUPPORT
// Get file table index (associated with a zstream)
_PUBLIC int32_t pups_get_ftab_index_from_zstream(const gzFile);
#endif /* ZLIB_SUPPORT */


// Show link file locks concurrently in use [root thread]
_PROTOTYPE _PUBLIC int32_t pups_show_link_file_locks(const FILE *);

// Apply extended file lock
_PROTOTYPE _EXPORT int32_t pups_lockf(const des_t, const int32_t, const off_t);

// Test to see if file is on a mounted filesystem
_PROTOTYPE _EXPORT _BOOLEAN pups_get_fs_mouNtinfo(const char *, char *);

// Set system error number
_PROTOTYPE _EXPORT void pups_set_errno(const int32_t);

// Get system error number
_PROTOTYPE _EXPORT int32_t pups_get_errno(void);

// Swap a pair of short  integers
_PROTOTYPE _EXPORT void sswap(int16_t *, int16_t *);

// Swap a pair of integers
_PROTOTYPE _EXPORT void iswap(int *, int32_t *);

// Swap a pair of long integers
_PROTOTYPE _EXPORT void lswap(int64_t  *, int64_t  *);

// Swap a pair of floats
_PROTOTYPE _EXPORT void fswap(FTYPE *, FTYPE *);

// Set error handler parameters
_PROTOTYPE _EXPORT void pups_seterror(const FILE *, const int32_t, const int32_t);

// Error handler
_PROTOTYPE _EXPORT int32_t pups_error(const char *);

// Initialise PUPS/P3 environment [root thread] 
_PROTOTYPE _EXPORT void pups_std_init(_BOOLEAN, int32_t  *,char *,char *,char *,char *, const char *[]);

// Load argument vector from file
_PROTOTYPE _EXPORT void pups_argfile(int32_t, uint32_t *,  char *[], _BOOLEAN []);

// Generate effective command tail string from secondary argument vector
_PROTOTYPE _EXPORT void pups_argtline(char *);

// Make copy of comamnd line arguments
_PROTOTYPE _EXPORT void pups_copytail(int32_t *, char *[], const char *[]);

// Locate argument in command tail
_PROTOTYPE _EXPORT int32_t pups_locate(_BOOLEAN *, const char *, uint32_t *, char *[], uint32_t);

// Get character from command tail
_PROTOTYPE _EXPORT char pups_ch_dec  (int32_t *, uint32_t *, char *[]);

// Get  integer from command tail argument
_PROTOTYPE _EXPORT int32_t pups_i_dec(int32_t *, uint32_t *, char *[]);

// Get float/double from command tail argument
_PROTOTYPE _EXPORT FTYPE pups_fp_dec(int32_t *,  uint32_t *, char *[]);

// Get string from command tail argument [not thread safe]
_PROTOTYPE _EXPORT char *pups_str_dec(int32_t *, uint32_t *, char *[]);

// Check stdio descriptor redirectiom 
_PROTOTYPE void pups_check_redirection(const des_t des);

// Convert character to integer
_PROTOTYPE _EXPORT int32_t actoi(const char);

// Test for short even  integer
_PROTOTYPE _EXPORT _BOOLEAN seven(const int16_t);

// Test for even  integer
_PROTOTYPE _EXPORT _BOOLEAN ieven(const int32_t);

// Test for long even integer
_PROTOTYPE _EXPORT _BOOLEAN leven(const  int64_t );

// Test for short integer sign
_PROTOTYPE _EXPORT _BOOLEAN ssign(const int16_t);

// Test for  integer sign
_PROTOTYPE _EXPORT _BOOLEAN isign(const int32_t);

// Test for long integer sign
_PROTOTYPE _EXPORT _BOOLEAN lsign(const int64_t );

// Find the maximum of a pair of short integers
_PROTOTYPE _EXPORT int32_t smax(const int16_t, const int16_t);

// Find the maximum of a pair of integers
_PROTOTYPE _EXPORT int32_t imax(const  int32_t, const  int32_t);

// Find the maximum of a pair of  int32_t64_t egers
_PROTOTYPE _EXPORT int32_t lmax(const  int64_t , const  int64_t );

// Get sign of character
_PROTOTYPE _EXPORT int32_t chsign(const char);

// Pause/test routine
_PROTOTYPE _EXPORT int32_t upause(const char *);

// Check for file and open it if it exists
_PROTOTYPE _EXPORT int32_t pups_open(const char *, const int32_t, const int32_t);

// Close file
_PROTOTYPE _EXPORT int32_t pups_close(const des_t);

// Test if (named) file is living
_PROTOTYPE _EXPORT _BOOLEAN pups_isalive(const char *);

// Protect an (unopened) file
_PROTOTYPE _EXPORT int32_t pups_protect(const char *, const char *, const void *);

// Unprotect an (unopened) file
_PROTOTYPE _EXPORT int32_t pups_unprotect(const char *);

// Return the number of times a live file descriptor has been lost (and recreated)
_PROTOTYPE _EXPORT int32_t pups_lost(const des_t);

// Test if file descriptor is living 
_PROTOTYPE _EXPORT _BOOLEAN pups_fd_islive(const des_t);

// Make dead file descriptor alive
_PROTOTYPE _EXPORT int32_t pups_fd_alive(const des_t, const char *, const void *);

// Kill living file descriptor
_PROTOTYPE _EXPORT int32_t pups_fd_dead(const des_t);

// CLose all open (ftab) file descriptors
_PROTOTYPE _EXPORT void pups_closeall(void);

// Display currently open extended file descriptors [root thread]
_PROTOTYPE _EXPORT void pups_show_open_fdescriptors(const FILE *);

// Strip a comment from an input file
_PROTOTYPE _EXPORT _BOOLEAN strip_comment(const FILE *, int32_t *, char *);

// Test for existence of substring
_PROTOTYPE _EXPORT _BOOLEAN strncmps(const char *, const char *);

// Strip an extension from a string
_PROTOTYPE _EXPORT _BOOLEAN strrext(const char *, char *, const char);

// Reverse strip an extension from a string
_PROTOTYPE _EXPORT _BOOLEAN strrextr(const char *, char *, const char);

// Copy string excluding characters
_PROTOTYPE _EXPORT ssize_t strexccpy(const char *, char *, const char *);

// Extract substring from string [root thread]
_PROTOTYPE _EXPORT _BOOLEAN strext(const char, char *, const char *);

// Case insensitive comparison of two strings (up to n significant characters)
_PROTOTYPE _EXPORT int32_t strncmpi(const char *, const char *, size_t);

// Convert string to lower case
_PROTOTYPE _EXPORT char *strtolower(const char *);

// Convert string to upper case
_PROTOTYPE _EXPORT char *strtoupper(const char *);

// Check if string 2 is a substring of string 1
_PROTOTYPE _EXPORT _BOOLEAN strin(const char *, const char *);

// Return tail of string s1 beginning at string s2
_PROTOTYPE _EXPORT char *strin2(const char *, const char *);

// Look for character token demarcated occurence of string s2 within string s1
_PROTOTYPE _EXPORT _BOOLEAN strintok(const char *, const char *, const char *);

// Check if string 3 is to right of string 2 within string 1
_PROTOTYPE _EXPORT _BOOLEAN stright(const char *, const char *, const char *);

// Replace string with a character constant
_PROTOTYPE _EXPORT _BOOLEAN strepch(char *, const char *, const char);

// Globally replace string s3 with string s4 globally in string s1, returning as string s2
_PROTOTYPE _EXPORT _BOOLEAN strep(const char *, char *, const char *, const char *);

// Globally replace all token demarcated instances of string s3 in string s1 with string s4 returning result as string s2
_PROTOTYPE _EXPORT _BOOLEAN streptok(const char *, char *, const char *, const char *, const char *);

//  Reverse truncate string at demarcation character (c_cnt instance from end of string)
_PROTOTYPE _EXPORT _BOOLEAN strtrnc(char *, const char, size_t);

// Truncate string at demarcation character (c_cnt instance from start of string)
_PROTOTYPE _EXPORT _BOOLEAN strtrnch(char *, const char, size_t);

// Strip trailing characters from string
_PROTOTYPE _EXPORT _BOOLEAN strail(char *, const char);

// Extract leaf from pathname (/branch/leaf)
_PROTOTYPE _EXPORT _BOOLEAN strleaf(const char *, char *);

// Extract branch from pathname (/branch/leaf)
_PROTOTYPE _EXPORT _BOOLEAN strbranch(const char *, char *);

// Strip first digit in string (and all characters after it)
_PROTOTYPE _EXPORT _BOOLEAN strdigit(char *);

// Strip leading characters from string (before demarcation character)
_PROTOTYPE _EXPORT char *strlead(char *, const char);

// Return substring which starts at first non-whitespace character
_PROTOTYPE _EXPORT char *strfirst(const char *);

// Extract string starting at demarcation character
_PROTOTYPE _EXPORT _BOOLEAN strfrm(const char *, const char, int32_t, char *);

// Count number of time character occurs in string
_PROTOTYPE _EXPORT ssize_t strchcnt(const char, const char *);

// Extract multiple substrings from string [root thread]
_PROTOTYPE _EXPORT _BOOLEAN m_strext(const int32_t, const char, char *, const char *);

// Routine to return the position of a character in string relative to start 
_PROTOTYPE _EXPORT ssize_t ch_pos(const char *, const char);

// Routine to return the position of character in string relative to end
_PROTOTYPE _EXPORT ssize_t rch_pos(const char *, const char);

// Extract substrings from string demarcated by arbitrary characters [root thread]
_PROTOTYPE _EXPORT _BOOLEAN mdc_strext(const char *, int32_t *, char *, const char *);

// Extract substrings from string demarcated by arbitrary characters [root thread]
_PROTOTYPE _EXPORT _BOOLEAN mdc_strext2(const char *, int32_t *, int32_t *, size_t *, char *, const char *);

// Look for occurence of string 1 inside string 2 returning index to string 1
_PROTOTYPE _BOOLEAN strinp(size_t *, const char *, const char *);

// Strip numeric characters from a string (including '#' and '^')
_PROTOTYPE _EXPORT _BOOLEAN strpdigit(const char *, char *);

// Strip character from string
_PROTOTYPE _EXPORT char *strpch(const char, char *);

// Replace multiple characters in string with character
_PROTOTYPE _EXPORT int32_t mchrep(const char, const char *, char *);

// Copy string checking for NULL or invalid argument string
_PROTOTYPE _EXPORT char *strccpy(char *, const char *);

// Test for empty string (contains only whitespace and control chars)
_PROTOTYPE _EXPORT _BOOLEAN strempty(const char *);

// Generate random string constaining hexidecimal characters
_PROTOTYPE _EXPORT int32_t strand(const size_t, char *);

// Find sign of float
_PROTOTYPE _EXPORT FTYPE fsign(const FTYPE);

// Round a floating point number to N significant figures
_PROTOTYPE _EXPORT FTYPE sigfig(const FTYPE, const uint32_t);

// Pascal compatable squaring routine
_PROTOTYPE _EXPORT FTYPE sqr(const FTYPE);

// Pascal compatable round routine
_PROTOTYPE _EXPORT int32_t iround(const FTYPE);

// Get next free PUPS file table index
_PROTOTYPE _EXPORT int32_t pups_find_ftab_index(void);

// Get next free PUPS file table index
_PROTOTYPE _EXPORT int32_t pups_get_ftab_index(const des_t);

// Update PSRP channel entry in file table
_PROTOTYPE _EXPORT int32_t pups_update_psrp_channel_ftab_entry(const char *, const char *);

// Open file
_PROTOTYPE _EXPORT FILE *pups_fopen(const char *, const char *, const int32_t);

// Bind descriptor to stream
_PROTOTYPE _EXPORT FILE *pups_fdopen(const des_t, const char *);
 
// Close file
_PROTOTYPE _EXPORT FILE *pups_fclose(const FILE *);


#ifdef ZLIB_SUPPORT
// Open file associated with zstream
_PROTOTYPE _EXPORT gzFile pups_xgzopen(const char *, const char *, const int32_t);

// Close file associated with zstream
_PROTOTYPE _EXPORT gzFile pups_gzclose(const gzFile);
#endif /* ZLIB_SUPPORT */


// Strip comments from file
_PROTOTYPE _EXPORT FILE *pups_strp_commnts(const char, const FILE *, char *);

// Flag unparsed command line arguments [root thread]
_PROTOTYPE _EXPORT void pups_t_arg_errs(const _BOOLEAN *, const char *[]);

// Extended error checking write 
_PROTOTYPE _EXPORT int32_t pups_write(const des_t, const void *, const psize_t);

// Extended error checking read 
_PROTOTYPE _EXPORT int32_t pups_read(const des_t, void *, const psize_t);

// Set memory allocator (extended) options [root thread]
_PROTOTYPE _EXPORT void pups_set_alloc_opt(const int32_t);

// Extended error checking malloc
_PROTOTYPE _EXPORT void *pups_malloc(const psize_t);

// Extended error checking realloc
_PROTOTYPE _EXPORT void *pups_realloc(void *, const psize_t);

// Extended error checking calloc
_PROTOTYPE _EXPORT void *pups_calloc(const pindex_t, const psize_t);

// Extended error checking free 
_PROTOTYPE _EXPORT void *pups_free(const void *);

// Set process state
_PROTOTYPE _EXPORT void pups_set_state(const char *);

// Get process state
_PROTOTYPE _EXPORT void pups_get_state(const char *);

// Display process state [root thread]
_PROTOTYPE _EXPORT void pups_show_state(void);

// Register name of (file) creator
_PROTOTYPE _EXPORT int32_t pups_creator(const des_t);

// Relinquish creator rights to file
_PROTOTYPE _EXPORT int32_t pups_not_creator(const des_t);

// Execute command string
_PROTOTYPE _EXPORT int32_t pups_execls(const char *);

// Allocate dynamic 2D array
_PROTOTYPE _EXPORT void **pups_aalloc(const pindex_t, const pindex_t, const psize_t);

// Free dynamic 2D array
_PROTOTYPE _EXPORT void **pups_afree(pindex_t, void **);

// Sort an array of floating point values using heapsort
_PROTOTYPE _EXPORT void pups_sort(int32_t, FTYPE [], FTYPE[]);

// Build a table of filesystem objects in the current directory matching key
_PROTOTYPE _EXPORT char **pups_get_directory_entries(const char *, const char *, int32_t *, int32_t *);

// Build a table of filesystem objects in the current directory matching multiple keys
_PROTOTYPE _EXPORT char **pups_get_multikeyed_directory_entries(const char *, const int32_t, const char **, int32_t *, int32_t *);

// Read call which is not interrupted by signal handling
_PROTOTYPE _EXPORT ssize_t pups_sread(const des_t, char *, const size_t);

// Write call which is not interrupted by signal handling
_PROTOTYPE _EXPORT ssize_t pups_swrite(const des_t, const char *, const size_t);

// Search (shell) path directories for item
_PROTOTYPE _EXPORT char *pups_search_path(const char *, const char *);

// Set process execution name (in kernel process table) [root thread]
_PROTOTYPE _EXPORT void pups_set_pen(char *[], const char *, const char *);

// Register process entrance function [root thread]
_PROTOTYPE _EXPORT int32_t pups_register_entrance_f(const char *, const void *, const char *);

// Deregister process entrance function [root thread]
_PROTOTYPE _EXPORT int32_t pups_deregister_entrance_f(const void *);

// Display process entrance functions [root thread]
_PROTOTYPE _EXPORT void pups_show_entrance_f(const FILE *);

// Register process exit function [root thread]
_PROTOTYPE _EXPORT int32_t pups_register_exit_f(const char *, const void *, const char *);

// Deregister process exit function [root thread]
_PROTOTYPE _EXPORT int32_t pups_deregister_exit_f(const void *);

// Display process exit functions [root thread]
_PROTOTYPE _EXPORT void pups_show_exit_f(const FILE *);

// Register process abort function [root thread]
_PROTOTYPE _EXPORT int32_t pups_register_abort_f(const char *, const void *, const char *);

// Deregister process abort  function [root thread]
_PROTOTYPE _EXPORT int32_t pups_deregister_abort_f(const void *);

// Display process abort functions [root thread]
_PROTOTYPE _EXPORT void pups_show_abort_f(const FILE *);

// PUPS exit [root thread]
_PROTOTYPE _EXPORT int32_t pups_exit(const int32_t);

// Pipe read 
_PROTOTYPE _EXPORT ssize_t pups_pipe_read(const des_t, void *, const size_t);

// Block signal (PUPS compliant)
_PROTOTYPE _EXPORT int32_t pupsighold(const int32_t, const _BOOLEAN);

// Unblock signal (PUPS/P3 compliant)
_PROTOTYPE _EXPORT  int32_t pupsigrelse(const int32_t);

// Block signal set (PUPS/P3 compliant)
_PROTOTYPE _EXPORT void pupshold(const int32_t);

// Unblock signal set (PUPS compliant)
_PROTOTYPE _EXPORT void pupsrelse(const int32_t);

// PUPS compliant POSIX signal handler
_PROTOTYPE _EXPORT int32_t pups_sighandle(const int32_t, const char *, const void *, const sigset_t *);

// PUPS compliant process suspension (until specified signal recieved)
_PROTOTYPE _EXPORT int32_t pups_signalpause(const int32_t);

// PUPS complaint sigpending
_PUBLIC _BOOLEAN pups_signalpending(const int32_t);

// Display process signal table [root thread]
_PROTOTYPE _EXPORT void pups_show_sigstatus(const FILE *);

// Display state of process signal mask (and signals pending) [root thread]
_PROTOTYPE _EXPORT void pups_show_sigmaskstatus(const FILE *);

// Convert signal name to signal number
_PROTOTYPE _EXPORT int32_t pups_signametosigno(const char *);

// Convert signal number to signal name
_PROTOTYPE _EXPORT char *pups_signotosigname(const int32_t, char *);

// Display detailed state of signal [root thread]
_PROTOTYPE _EXPORT int32_t pups_show_siglstatus(const int32_t, const FILE *);

// Execute non-local goto (as exit from a signal handler)
_PROTOTYPE _EXPORT int32_t pups_sigvector(const int32_t, const sigjmp_buf *);

// Check descriptor for data and/or exceptions using select
_PROTOTYPE _EXPORT int32_t pups_monitor(const des_t, const int32_t, const int32_t);

// Return current date string
_PROTOTYPE _EXPORT void strdate(char *);

// Set virtual  interval timer [root thread]
_PROTOTYPE _EXPORT int32_t pups_setvitimer(const char *, const int32_t, const int32_t, const time_t, const char *, const void *);

// Clear virtual  interval timer [root thread]
_PROTOTYPE _EXPORT int32_t pups_clearvitimer(const char *);

// Restart virtual interval timer system [root thread]
_PROTOTYPE _EXPORT _BOOLEAN vitmer_restart(void);

// Display virtual interval timer table [root thread]
_PROTOTYPE _EXPORT void pups_show_vitimers(const FILE *);

// Set parameters for homeostatic file system space monitoring [root thread]
_PROTOTYPE _EXPORT int32_t pups_set_fs_hsm_parameters(const des_t, const int32_t, const char *);

// Homeostatic write check for filesystem [root thread]
_PUBLIC int32_t pups_write_homeostat(const des_t, int32_t (*)(int32_t));

// Homeostat for (open) files [root thread]
_PROTOTYPE _EXPORT int32_t pups_default_fd_homeostat(void *, const char *);

// Monitor effective parent and exit if it is terminated [root thread]
_PROTOTYPE _EXPORT void pups_default_parent_homeostat(void *, const char *);

// Check for space of filesystem associated with open file descriptor
_PROTOTYPE _EXPORT _BOOLEAN pups_check_fs_space(const des_t);

// Attach homeostat to file table entry [root thread]
_PROTOTYPE _EXPORT int32_t pups_attach_homeostat(const des_t, const void *);

// Detach homeostat from file table entry [root thread]
_PROTOTYPE _EXPORT int32_t pups_detach_homeostat(const des_t);

// PUPS compliant sleep call
_PROTOTYPE _EXPORT int32_t pups_sleep(const int32_t);

// Microsecond alarm function (raises SIGALRM on timeout)
_PROTOTYPE _EXPORT int32_t pups_malarm(const uint64_t);

// Acquire lock on (named) file
_PROTOTYPE _EXPORT _BOOLEAN pups_get_lock(const char *, const int32_t);

// Release lock on (named) file
_PROTOTYPE _EXPORT _BOOLEAN pups_release_lock(const char *);

// Get (or test) write lock on open file descriptor
_PROTOTYPE _EXPORT _BOOLEAN pups_get_fd_lock(const des_t, const int32_t);

// Release write lock on open file descriptor
_PROTOTYPE _EXPORT int32_t pups_release_fd_lock(const des_t);

// Get file descriptor from filename
_PROTOTYPE _EXPORT int32_t pups_fname2fdes(const char *);

// Get file stream from filename
_PROTOTYPE _EXPORT FILE *pups_fname2fstream(const char *);

// Get file table index from file name
_PROTOTYPE _EXPORT int32_t pups_fname2index(const char *);

// Get file table index from file descriptor
_PROTOTYPE _EXPORT int32_t pups_fdes2index(const des_t);

// Enable obituary for pipestream (child) process
_PROTOTYPE _EXPORT int32_t pups_pipestream_obituary_enable(const des_t);

// Disable obituary for pipestream (child) process
_PROTOTYPE _EXPORT int32_t pups_pipestream_obituary_disable(const des_t);

// Extended fork routine (which records forked children)
_PROTOTYPE _EXPORT pid_t pups_fork(const _BOOLEAN, const _BOOLEAN);

// Create file without opening it 
_PROTOTYPE _EXPORT int32_t pups_creat(const char *, const int32_t);

// Change mode of file associated with open descriptor and update file table
_PROTOTYPE _EXPORT  int32_t pups_fchmod(const des_t, const int32_t);

// Return string size (including terminating NULL)
_PROTOTYPE _EXPORT  int32_t pups_strlen(const char *);

// Memory block tracking malloc
_PROTOTYPE _EXPORT void *pups_tmalloc(const size_t, const char *, const char *);

// Memory block tracking calloc
_PROTOTYPE _EXPORT void *pups_tcalloc(const int32_t, const size_t, const char *, const char *);

// Memory block tracking reaalloc
_PROTOTYPE _EXPORT void *pups_trealloc(void *, const size_t);

// Get tracked object heap address from tracked object name
_PROTOTYPE _EXPORT void *pups_tnametoptr(const char *);

// Get pointer from partial tracked object name
_PROTOTYPE _EXPORT void *pups_tpartnametoptr(uint32_t *, const char *);

// Get pointer from tracked type 
_PROTOTYPE _EXPORT void *pups_ttypetoptr(uint32_t *, const char *);

// Memory block tracking free
_PROTOTYPE _EXPORT void *pups_tfree(const void *);

// Tracked dynamic array allocation
_PROTOTYPE _EXPORT void **pups_taalloc(const pindex_t, const pindex_t, const psize_t, const char *, const char *);

// Free tracked 2D arrray
_PROTOTYPE _EXPORT void **pups_tafree(const void **);

// Display tracked objects [root thread]
_PROTOTYPE _EXPORT int32_t pups_tshowobject(const FILE *, const void *);

// Display tracked object table [root thread]
_PROTOTYPE _EXPORT int32_t pups_tshowobjects(const FILE *, int32_t *);

// PUPS fgets function
_PROTOTYPE _EXPORT char *pups_fgets(char *, const size_t, const FILE *);

// Wait function compatable with PUPS child managment
_PROTOTYPE _EXPORT int32_t pupswait(const _BOOLEAN,  int32_t *);

// Waitpid function compatable with PUPS child managment
_PROTOTYPE _EXPORT int32_t pupswaitpid(const _BOOLEAN, const pid_t, int32_t *);

// Set child name 
_PROTOTYPE _EXPORT _BOOLEAN pups_set_child_name(const pid_t, const char *);

// Clear a child table slot
_PROTOTYPE _EXPORT  int32_t pups_clear_chtab_slot(const _BOOLEAN, const uint32_t);

// Display child table [root thread]
_PROTOTYPE _EXPORT void pups_show_children(const FILE *);

// Install handler for PUPS automatic child management
_PROTOTYPE _EXPORT void pups_auto_child(void);

// Remove handler for PUPS automatic child management
_PROTOTYPE _EXPORT void pups_noauto_child(void);

// Check to see if line is a comment line (first non-space char is '#') or empty
_PROTOTYPE _EXPORT _BOOLEAN strcomment(const char *);

// Set all characters in string to NULL ('\0')
_PROTOTYPE _EXPORT void strclr(char *);

// Save PUPS signal table entry
_PROTOTYPE _EXPORT void pups_sigtabsave(const int32_t, sigtab_type *);

// Restore a PUPS signal table entry (and reinstall signal handler)
_PROTOTYPE _EXPORT void pups_sigtabrestore(const int32_t, const sigtab_type *);

// Turn tty echoing off
_PROTOTYPE _EXPORT int32_t pups_tty_echoing_off(FILE *, struct termios *);

// Turn tty echoing on 
_PROTOTYPE _EXPORT void pups_tty_echoing_on(FILE *, int32_t, struct termios);

// Execute command on local host
_PROTOTYPE _EXPORT pid_t pups_lexec(const char *, const char *, const int32_t);

// PUPS statkill (pids of terminated or stopped processes return error)
_PROTOTYPE _EXPORT int32_t pups_statkill(const pid_t, const  int32_t);

// Set file table id tag
_PROTOTYPE _EXPORT int32_t pups_set_ftab_id(const des_t, const  int32_t);

// Get file table index (from id tag)
_PROTOTYPE _EXPORT int32_t pups_get_ftab_index_by_id(const  int32_t);

// Get file table index (from file name)
_PROTOTYPE _EXPORT int32_t pups_get_ftab_index_by_name(const char *);

// Clear file table slot
_PROTOTYPE _EXPORT int32_t pups_clear_ftab_slot(const _BOOLEAN, const  int32_t);

// Initialise child table [root thread]
_PROTOTYPE _EXPORT void pups_init_child_table(const uint32_t);

// lseek() function which is PUPS signal safe
_PROTOTYPE _EXPORT off_t pups_lseek(des_t, off_t, int32_t);

// Get load average for localhost
_PROTOTYPE _EXPORT FTYPE pups_get_load_average(const int32_t which_load_average);

// Test to see if fdes is asscoiated with a seekable device
_PROTOTYPE _EXPORT _BOOLEAN pups_is_seekable(const des_t);

// Relay data to a process slaved via a pair of FIFO's
_PROTOTYPE _EXPORT int32_t pups_fifo_relay(const pid_t, const  int32_t, const  int32_t);

// Set read or write link file lock
_PROTOTYPE int32_t pups_get_rdwr_link_file_lock(const int32_t, const int32_t, const char *);

// Set a write (exclusive) link file lock
_PROTOTYPE _EXPORT int32_t pups_get_link_file_lock(const uint32_t, const char *);

// Release link file lock
_PROTOTYPE _EXPORT int32_t pups_release_link_file_lock(const char *);

// Release all link file locks
_PROTOTYPE _EXPORT int32_t pups_release_all_link_file_locks(void);

// Wait for file to be unlinked before returning to caller
_PROTOTYPE _EXPORT int32_t pups_unlink(const char *);

// PUPS usleep
_PROTOTYPE _EXPORT int32_t pups_usleep(const uint64_t);

// Is current file is on ISOfs filesystem
_PROTOTYPE _EXPORT _BOOLEAN pups_is_on_isofs(const char *);

// Is file descriptor is associated with ISOfs filesystem
_PROTOTYPE _EXPORT _BOOLEAN pups_is_on_fisofs(const des_t);

// Is current file is on NFS filesystem
_PROTOTYPE _EXPORT _BOOLEAN pups_is_on_nfs(const char *);

// Is open file descriptor is associated with NFS filesystem
_PROTOTYPE _EXPORT _BOOLEAN pups_is_on_fnfs(const des_t);

// Copy file (by name)
_PROTOTYPE _EXPORT int32_t pups_cpfile(const char *, const char *, int32_t);

// Copy file (by open descriptor descriptors)
_PROTOTYPE _EXPORT int32_t pups_fcpfile(const des_t, const des_t);

// Check if command is running
_PROTOTYPE _EXPORT _BOOLEAN pups_cmd_running(void);

// Check if I own a given process
_PROTOTYPE _EXPORT _BOOLEAN pups_i_own(const pid_t);

// Set re-entry point for SIGRESTART
_PROTOTYPE _EXPORT int32_t pups_restart_enable(void);

// Reset re-entry point for SIGRESTART
_PROTOTYPE _EXPORT int32_t pups_restart_disable(void);

// Compute (psuedo) 32 bit cyclic redundancy checksum
_PUBLIC int32_t pups_crc_32(const size_t, _BYTE *);

// Generate 64 bit cyclic redundancy checksum
_PUBLIC uint64_t pups_crc_64(const size_t, _BYTE *);

// Extract CRC signature from file name
_PROTOTYPE _EXPORT uint64_t pups_get_signature(const char *, const char);

// Sign a file (with its CRC checksum)
_PROTOTYPE _EXPORT int32_t pups_sign(const uint64_t, const char *, char *, const char);

// Replace item in command tail
_PROTOTYPE _EXPORT _BOOLEAN pups_replace_cmd_tail_item(const char *, const char *);

// Insert item  int32_to the command tail
_PROTOTYPE _EXPORT int32_t pups_insert_cmd_tail_item(const char *, const char *);

// Clear command tail
_PROTOTYPE _EXPORT int32_t pups_clear_cmd_tail(void);

// Handler for backtracking (when segmentation violation occurs)
//_PROTOTYPE _EXPORT int32_t pups_segv_backtrack_handler(const int32_t);

// Set a backtrack re-entry point (in process address space)
_PROTOTYPE _EXPORT _BOOLEAN pups_backtrack(const _BOOLEAN);

// Has file been updated? [not thread safe]
_PROTOTYPE _EXPORT _BOOLEAN pups_file_updated(const char *);

// Is host reachable? [root thread]
_PROTOTYPE _EXPORT _BOOLEAN pups_host_reachable(const char *);

// Set (fcntl) file lock (on descriptor)
_PROTOTYPE _EXPORT int32_t pups_flock(const des_t, const int32_t, const off_t, const off_t, const _BOOLEAN);

// Set (fcntl) file lock (on named file)
_PROTOTYPE _EXPORT int32_t pups_flockfile(const char *, const int32_t, const off_t, const off_t, const _BOOLEAN);

// Display flock locks currently held by caller process [root thread]
_PROTOTYPE _EXPORT int32_t pups_show_flock_locks(const FILE *);

// Substitute substring
_PROTOTYPE _EXPORT int32_t strsub(const char *, const char *, const char *, char *);


#ifdef PSRP_AUTHENTICATE
// Autheticate user [root thread]
_PROTOTYPE _EXPORT _BOOLEAN pups_checkuser(const char *, const char *);

// Authenticate secure service password [root thread]
_PROTOTYPE _EXPORT _BOOLEAN pups_check_appl_password(const char *);

// Authenticate a user [root thread]
_PROTOTYPE _EXPORT _BOOLEAN pups_getpass(const char *);

// Check that application has password set [root thread]
_PROTOTYPE _EXPORT _BOOLEAN pups_check_pass_set(void);
#endif /* PSRP_ATHENTICATE */


// Get file type (wrapper function for the "file" command)
_PROTOTYPE _EXPORT char *pups_get_file_type(const char *);

// Convert string to lower case
_PROTOTYPE _EXPORT int32_t downcase(char *);

// Convert string to upper case
_PROTOTYPE _EXPORT int32_t upcase(char *);

// Get name of host exporting NFS file [root thread]
_PROTOTYPE _EXPORT int32_t pups_get_nfs_host(const char *pathname, const char *host);

// Reflect endianness of float
_PROTOTYPE _EXPORT float fconv(const FTYPE);

// Reflect endianness of int
_PROTOTYPE _EXPORT int32_t iconv(const int32_t);

// Reflect endianness of short
_PROTOTYPE _EXPORT int16_t sconv(const int16_t);

// Reflect endianness of long
_PROTOTYPE _EXPORT int64_t lconv(const  int64_t );

// Set context of non local goto valid
_PROTOTYPE _EXPORT int32_t pups_set_jump_vector(void);

// Set context of non local goto invalid
_PROTOTYPE _EXPORT int32_t pups_reset_jump_vector(void);

// TRUE if linux version information matches key
_PROTOTYPE _EXPORT _BOOLEAN pups_is_linuxversion(const char *);

// Get current process loading (and memory statistics) for host
_PROTOTYPE _PUBLIC int32_t pups_get_resource_loading(FTYPE *, uint32_t *);

// Do we have resources required?
_PROTOTYPE _EXPORT _BOOLEAN pups_have_resources(FTYPE,  int32_t);

// Peer to peer file transfer
_PROTOTYPE _EXPORT int32_t pups_scp(const char *, const char *, const char *);

// Is process P3 aware?
_PROTOTYPE _EXPORT _BOOLEAN pups_aware(const int32_t);

// Open pipe stream eturning child pid
_PROTOTYPE _EXPORT des_t pups_popen(const char *, const char *,  const int32_t, pid_t *);

// Open buffered pipe stream retruning child pid 
_PROTOTYPE _EXPORT FILE *pups_fpopen(const char *, const char *, const char *, pid_t *);

// Close pipe stream
_PROTOTYPE _EXPORT int32_t pups_pclose(const des_t, const pid_t);

// Close buffered pipe stream
_PROTOTYPE _EXPORT int32_t pups_fpclose(const FILE *, const pid_t);

// Check if process in foreground
_PROTOTYPE _EXPORT _BOOLEAN pups_is_foreground(void);

// Check if process in background
_PROTOTYPE _EXPORT _BOOLEAN pups_is_background(void);

// Is command installed on system?
_PROTOTYPE _EXPORT _BOOLEAN pups_cmd_installed(const char *);

// Get CPU utilisation on host
_PROTOTYPE _EXPORT FTYPE pups_cpu_utilisation(void);

// Get prefix
_PROTOTYPE _EXPORT int32_t pups_prefix(const char, const char *, char *);

// Get suffix
_PROTOTYPE _EXPORT int32_t pups_suffix(const char, const char *, char *);

// Get file size
_PROTOTYPE _EXPORT ssize_t pups_get_fsize(const char *);

// Descriptor associated with a pipe
_PROTOTYPE _EXPORT int32_t isapipe(const des_t);

// Descriptor associated with a file
_PROTOTYPE _EXPORT int32_t isafile(const des_t);

// Descriptor associated with a socket
_PROTOTYPE _EXPORT int32_t isasock(const des_t);

// Descriptor associated with a (data) conduit (pipe, file or socket)
_PROTOTYPE _EXPORT int32_t isaconduit(const des_t);

// Get system information for current host
_PROTOTYPE _EXPORT int32_t pups_sysinfo(char *, char *, char *, char *);

// Am I running in virtual enviroment? 
_PROTOTYPE _EXPORT _BOOLEAN pups_os_is_virtual(char *);

// Is character first one in string? 
_PROTOTYPE _EXPORT _BOOLEAN ch_is_first(const char *, const char); 

// Is character last one in string? 
_PROTOTYPE _EXPORT _BOOLEAN ch_is_last(const char *, const char); 

// Is architecture big endian?
_PROTOTYPE _EXPORT _BOOLEAN pups_bigendian(void);


#ifdef CRIU_SUPPORT
// Enable (Criu) state saving [root thread]
_PROTOTYPE _EXPORT int32_t pups_ssave_enable(void);

// Disable (Criu) state saving [root thread]
_PROTOTYPE _EXPORT int32_t pups_ssave_disable(void);
#endif /* CRIU_SUPPORT */

// Set (round robin) scheduling priority [root thread]
_PROTOTYPE _EXPORT int32_t pups_set_rt_sched(const int32_t);

// Switch to standard (time sharing) scheduler [root thread]
_PROTOTYPE _EXPORT int32_t pups_set_tslice_sched(void);

// Copy a file
_PROTOTYPE _EXPORT  int32_t pups_copy_file(const int32_t, const char *, const char *);

// Can we run command? 
_PROTOTYPE _EXPORT _BOOLEAN pups_can_run_command(const char *);

// Enable (memory) residency
_PROTOTYPE _EXPORT void pups_enable_resident(void);

// Get size of directory in bytes
_PROTOTYPE _EXPORT size_t pups_dsize(const char *);

// Show software watchdog status
_PROTOTYPE _EXPORT int32_t pups_show_softdogstatus(const FILE *);

// Start software watchdog
_PROTOTYPE _EXPORT int32_t pups_start_softdog(const uint32_t);

// Stop software watchdog
_PROTOTYPE _EXPORT int32_t pups_stop_softdog(void);

// Non-blocking poll of file descriptor 
_PROTOTYPE _EXPORT _BOOLEAN pups_fd_has_data(const des_t);


#ifdef _CPLUSPLUS
#   undef  _EXPORT
#   define _EXPORT extern "C"
#else
#   undef  _EXPORT
#   define _EXPORT extern
#endif /* _CPLUSPLUS */

#endif /* UTILS_H */
