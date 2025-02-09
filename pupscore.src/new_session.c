/*--------------------------------------
    Purpose: sets up a new process group

    Author:  M.A. O'Neill
             Tumbling Dice Ltd
             Gosforth
             Newcastle upon Tyne
             NE3 4RT
             United Kingdom

    Version: 2.02 
    Dated:   10th Decemeber 2024
    E-mail:  mao@tumblingdice.co.uk
--------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>


/*---------*/
/* Defines */
/*---------*/
/*---------*/
/* Version */
/*---------*/

#define NS_VERSION  "2.02"

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#ifdef TRUE 
#undef TRUE 
#endif
#define TRUE  255



/*--------------------------------*/
/* Relay signals to process group */
/*--------------------------------*/

static int32_t signal_relay(const int32_t signum)

{   static int32_t caught = FALSE;


    /*-----------------------*/
    /* Send to process group */
    /*-----------------------*/

    if(caught == FALSE)
    {  caught = TRUE;
       (void)kill(0,signum);
    }


    /*-------------------------------------*/
    /* Ignore signal sent to process group */
    /*-------------------------------------*/

    else
       caught = FALSE;

    if(signum == SIGTERM)
       exit(0);

    (void)signal(signum,(void *)&signal_relay);
}



/*------------------*/
/* main entry point */
/*------------------*/

int32_t main(int32_t argc, char *argv[])

{   int32_t pgrp;


    /*--------------------*/
    /* Parse command line */
    /*--------------------*/

    if(argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-usage") == 0))
    {  
       (void)fprintf(stderr,"\nnew_session version %s, (C) Tumbling Dice 2003-2024 (gcc %s: built %s %s)\n\n",NS_VERSION,__VERSION__,__TIME__,__DATE__);
       (void)fprintf(stderr,"NEW_SESSION is free software, covered by the GNU General Public License, and you are\n");
       (void)fprintf(stderr,"welcome to change it and/or distribute copies of it under certain conditions.\n");
       (void)fprintf(stderr,"See the GPL and LGPL licences at www.gnu.org for further details\n");
       (void)fprintf(stderr,"NEW_SESSION comes with ABSOLUTELY NO WARRANTY\n\n");

       (void)fprintf(stderr,"\nUsage: new_session [-help | -usage]\n\n");
       (void)fflush(stderr);

       exit(1);
    }
    else if(argc > 1)
    {  (void)fprintf(stderr,"\nnew_session: option(s) not recognised\n\n");
       (void)fflush(stderr);

       exit(1);
    }


    (void)setsid();
    
    (void)fprintf(stderr,"%d",getpid());
    (void)fflush(stderr);


    /*---------------------------------------------*/
    /* Process group leader must continue to exist */
    /* as long as its process group does           */
    /*---------------------------------------------*/

    (void)signal(SIGTERM,(void *)&signal_relay);
    (void)signal(SIGTSTP,(void *)&signal_relay);


    /*-------------------------------------*/
    /* Wait for process group to terminate */
    /*-------------------------------------*/

    while(1)
         (void)sleep(1);

    exit(0);
}
