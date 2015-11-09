/*
 * ora_kstackprofiler - a simple kernel stack profiler extended with 
 *                      the option to add Oracle wait event profiling from SGA
 *
 *                      Author: Luca.Canali@cern.ch
 *                      Created: November 2015
 *                      */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static int pid;
static int delay_in_ms;
static int num_samples;
static int *ksuseopc_addr;
static int *ksusetim_addr;

static void usage(const char *name)
{
    fprintf(stderr,
"usage: %s --pid <pid> [--count <num samples>] [--delay <delay in ms>] [--ksuseopc <pointer>] [--ksusetim <pointer>]\n\n", name);
}

static int parse_options(int argc, char **argv)
{
  int c;

    /* parameter defaults */
    pid = 0;
    delay_in_ms = 500;
    num_samples = 20;
  
    while (1) {
      static struct option long_options[] =
        {
          {"pid",       required_argument, 0, 'a'},
          {"count",     required_argument, 0, 'b'},
          {"delay",     required_argument, 0, 'c'},
          {"ksuseopc",  required_argument, 0, 'd'},
          {"ksusetim",  required_argument, 0, 'e'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "", long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c) {

        case 'a':
            pid = atoi(optarg);
          break;

        case 'b':
            num_samples = atoi(optarg);
            break;

        case 'c':
            delay_in_ms = atoi(optarg);
            break;

        case 'd':
            ksuseopc_addr = (int *) atol(optarg);
            break;

        case 'e':
            ksusetim_addr = (int *) atol(optarg);
            break;

        case '?':
            /* getopt_long already printed an error message. */
            return -1;
            break;

        default:
            abort ();
        }
    }

    if ( pid <= 0) {
        fprintf (stderr, "Invalid value for pid: %d\n", pid);
        return -1;
    }

    if ( num_samples <= 0) {
        fprintf (stderr, "Invalid value for num_samples: %s\n", num_samples);
        return -1;
    }

    if ( delay_in_ms <= 0) {
        fprintf (stderr, "Invalid value for num_samples: %s\n", delay_in_ms);
        return -1;
    }


    fprintf(stderr, "pid = %d\n", pid);
    fprintf(stderr, "delay_in_ms = %d\n", delay_in_ms);
    fprintf(stderr, "num_samples = %d\n", num_samples);

    if ((ksuseopc_addr != NULL) &&  (ksusetim_addr != NULL)) {
        fprintf(stderr, "Oracle wait event tracing: ksuseopc_addr = %lu, ksusetim_addr = %lu\n", ksuseopc_addr, ksusetim_addr);
    } else {    
        fprintf(stderr, "No Oracle wait event tracing: ksuseopc_addr = 0, ksusetim_addr = 0\n");
    }
    printf("\n");
    
    return 0;
}


int main(int argc, char **argv)
{

    fprintf(stderr, "\nora_kstackprofiler: a kernel stack profiler extended with Oracle wait event data collection\n");
    if (parse_options(argc, argv) < 0) {
        usage(argv[0]);
        exit(-1);
    }
     
    if (stack_profile(pid, delay_in_ms*1000, num_samples, ksuseopc_addr, ksusetim_addr) <  0) {
        printf("Error in stack profiling\n");
        exit(-1);
    }

    exit(0);

}


