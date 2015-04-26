/* command.c - Commands which have nothing to do with mail... */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include "defines.h"
#include "command.h"
#include "config.h"
#include "mailcmd.h"
#include "mbox.h"
#include "utils.h"
#include "quit.h"

struct cmd Mailcmds[] = {
	{ "?",		do_help		},
	{ "Autofwd",	do_forward	},
	{ "Bye",	do_exit		},
	{ "Cancel",	do_quit		},
	{ "Delete",	do_kill		},
	{ "Exit",	do_exit		},
	{ "Help",	do_help		},
	{ "Info",	do_help		},
	{ "Kill",	do_kill		},
	{ "List",	do_list		},
	{ "Name",	do_name		},
	{ "Quit",	do_exit		},
	{ "Read",	do_read		},
	{ "Send",	do_send		},
	{ "SFax",	do_fax		},
	{ "SReply",	do_send		},
	{ "STatus",	do_status	},
	{ "Unkill",	do_unkill	},
	{ "Verbose",	do_read		},
	{ NULL,		NULL		}
};

int do_status(int argc, char **argv)
{
	printf("Messages: %i\n", messages);
	return 0;

}

int do_name(int argc, char **argv)
{
	char name[32];
	
	getname(name);
	
	if (strlen(name) == 0)
		printf("Name not changed. ");
	else {	/* Okay, save it */
		strcpy(fullname, name);
	}
	
	printf("You're now known as %s.\n", fullname);
	
	return 0;
}

/* int do_forward(int argc, char **argv) */
int do_forward(int argc, char **argv)
{
	char *email;
	char *fwdfile;
	char fwd[79];
	char forward[79];

	getaddy(fwd);
	FILE *fptr;

	sprintf(forward, "%s/.forward", homedir);
	fwdfile = strdup(forward);
	fptr = fopen(fwdfile, "w+");
        
//        if (strlen(fwd) == 0);
//                printf("Auto Forwarding stopped.\n");
//        else  { /* Okay, save it */
                fprintf(fptr, "%s", fwd);
//        }
        
        printf("You're mail will be sent to:\n%s\nThank you.\n", fwd);
        fclose(fptr); 
        return 0;
}

int do_help(int argc, char **argv)
{
	FILE *fp;
	char fname[80], line[256];
	struct cmd *cmdp;
	int i = 0;

	if (*argv[0] == '?') {				/* "?"		*/
		printf("Commands:\n");
		for (cmdp = Mailcmds; cmdp->name != NULL; cmdp++) {
			printf("%s%s", i ? ", " : "", cmdp->name);
			if (++i == 10) {
				printf("\n");
				i = 0;
			}
		}
		if (i) printf("\n");
		return 0;
	}
	strcpy(fname, DATA_AXMAIL_HELP_DIR);
	if (*argv[0] == 'i') {				/* "info"	*/
		strcat(fname, "info.hlp");
		printf("%s - %s\n", VERSION, COPYRIGHT);
		printf("%s", LICENSE);
	} else if (argc == 1) {				/* "help"	*/
		strcat(fname, "help.hlp");
	} else {					/* "help <cmd>"	*/
		if (strchr(argv[1], '/') == NULL) {
			strlwr(argv[1]);
			strcat(fname, argv[1]);
			strcat(fname, ".hlp");
		}
	}
	if ((fp = fopen(fname, "r")) == NULL) {
		if (*argv[0] != 'i')
			printf("No help for command %s.\n", argv[1] ? argv[1] : "help");
		return 0;
	}
	if (*argv[0] != 'i')
		printf("Help for command %s:\n", argv[1] ? argv[1] : "help");
	while (fgets(line, 256, fp) != NULL) {
		printf("%s", line);
		}
	fclose(fp);
	return 0;
}

int do_quit(int argc, char **argv)
{
	quit(0, 0);
	return 0;
}

int do_exit(int argc, char **argv)
{
	quit(1, 0);
	return 0;
}

