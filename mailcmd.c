
/* mailcmd.c - Mail commands */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>

#include "mailcmd.h"
#include "defines.h"
#include "config.h"
#include "mbox.h"
#include "utils.h"

/* List messages */

void printhead(int i, struct message *m) {
	char ch = ' ';
	
	if ((m->m_flag & (MREAD|MNEW)) == MNEW)
		ch = 'N';
	if ((m->m_flag & (MREAD|MNEW)) == 0)
		ch = 'U';
	if ((m->m_flag & (MDELETED)) == MDELETED)
		ch = 'K';
		
	printf("%c%c%4i %25.25s%6li %.39s\n",
		(current == i+1) ? '>' : ' ',
		ch, i+1, m->from, m->m_size, m->subj);
}

int do_list(int argc, char **argv)
{
	int i;
	
	if (!(messages)) {
		printf("No messages.\n");
		return 0;
	}
	
	printf("   Num From                       Size Subject\n");
	
	for (i = 0; i < messages; i++) {
		printhead(i, &message[i]);
	}

	return 0;
}

/* Read a message (xNOS-stylish parameters) */

int do_read(int argc, char **argv)
{
	char *myargv[64];
	int myargc, argsmine;
	char *tmpbuf;
	int i;
	int msg, maxmsg;

	if (!(messages)) {
		printf("You have no messages.\n");
		return 0;
	}

	if (argc > 1) {
		argsmine = 0;
		for (i = 1; i < argc; i++)
			myargv[i] = argv[i];
		myargc = argc;
	} else {
		argsmine = 1;
		if (current >= messages) {
			printf("No more messages.\n");
			return 0;
		}
		current++;
		myargc = 2;
		myargv[1] = malloc(17);
		sprintf(myargv[1], "echo");
		sprintf(myargv[1], "%i", current);
	}

	for (i = 1; i < myargc; i++) {
		tmpbuf = strchr(myargv[i], '-');
		msg = atoi(myargv[i]);
		if (tmpbuf == NULL)
			maxmsg = msg;
		else
			maxmsg = atoi(++tmpbuf);
		if (maxmsg < msg) {
			printf("Bad message number %i.\n", maxmsg);
			continue;
		}
		for (; msg <= maxmsg; msg++) {
			if (msg < 1 || msg > messages) {
				printf("There's no message number %i.\n", msg);
				continue;
			}
			readmesg(msg, (!strncmp(argv[0], "v", 1)));
			printf("\n");
			if (dot->receipt != NULL) {
			printf("\aA receipt was asked for. Use the SR command to make one.\n");
			}
		}
	}
	
	if (argsmine)
		for (i = 1; i < myargc; i++)
			free(myargv[i]);

	return 0;
	                          if (dot->receipt != NULL) {
                                printf("Receipt is needed\n");
                                return 0;
			}

}

/* Send a message (perhaps a reply) */
 
int do_send(int argc, char **argv)
{
	FILE *f;
	FILE *g;
	char str[LINESIZE + 1];
	int i;

	int reply = 0;

	if (!strncmp(argv[0], "sr", 2)) {
		reply = 1;
		if (argc == 1) {
			if (current == 0) {
				printf("No current message to reply to.\n");
				return 0;
			}
			i = current;
		} else
			i = atoi(argv[1]);

		i--;
		if ((i < 0) || (i >= messages)) {
			printf("There's no message %s.\n", argv[1]);
			return 0;
		}

		dot = &message[i];
	}
	
	if ((f = fopen(tempMesg, "w")) == NULL) {
		printf("Could not create temporary file.\n");
		syslog(LOG_NOTICE, "do_send: Could not create temporary file.\n");
		return 0;
	}

	fprintf(f, "From: %s <%s@%s>\n", fullname, username, hostname);
       
	str[0] = '\0';
	if (argc != 1)	/* Recipient on command line */
		for (i = 1; i < argc; i++) {
			if (i > 1)
				strcat(str, " ");
			strncat(str, argv[i], LINESIZE - strlen(str));
		}
	else {
		if (reply) {
			strncpy(str, dot->from, LINESIZE);
			printf("To: %s\n", str);
		} else {
			getstr(str, LINESIZE, "To: ");
			if (str[0] == '\0') {
				printf("No recipients, message cancelled.\n");
				fclose(f);
				remove(tempMesg);
				return 0;
			}
		}
	}
	
	fprintf( f, "To: %s\n", str);
	fprintf( f, "X-Mailer: %s\n", VERSION );
	fprintf( f, "X-Origin: Amateur Radio Services\n" );
	goto prio;;
	
	/* adding priority receive rule */

prio:
	getstr(str, LINESIZE, "Is this message emergency or urgent? (y/N/?): ");
	
	/* try to bullet-proof end-user responces a bit... */
		
	if (!strcmp(str, "?")) {
		printf("\nAnswering \"Y\" or \"yes\" here will flag the message as of being highest\n");
		printf("priority in nature and with most mail client software will present\n");
		printf("your message as an urgent read communication. By entering \"N\" or \"no\" or\n");
		printf("by hitting the enter key will send your mail message via normal delivery.\n\n");
		goto prio;
	}
				
	if (!strcasecmp(str, "Y") || !strcasecmp (str, "YES") || !strcasecmp (str, "YE")) {
		fprintf( f, "X-Priority: 1 (Highest)\n" );
		}
		else
		{ fprintf( f,"X-Priority: 3 (Normal)\n" );
		}

receipt:
	getstr(str, LINESIZE, "Read receipt requested? (y/N/?): ");

        if (!strcmp(str, "?")) {
                printf("\nAnswering \"Y\" or \"yes\" here will request a confirmation of \n");
                printf("your message being opened by the remote user. By entering \"N\" or \"no\" or\n");
                printf("by hitting the enter key will not request a confirmation receipt.\n\n");
                goto receipt;
        }

        if (!strcasecmp(str, "Y") || !strcasecmp (str, "YES") || !strcasecmp (str, "YE")) {
                fprintf( f, "Disposition-Notification-To: %s <%s@%s>\n", fullname, username, hostname);
                }
                else
                { fprintf( f,"" );
                }
		
	if (reply) {
		if (strncasecmp(dot->subj, "Re: ", 3))
			snprintf(str, LINESIZE, "Re: %s", dot->subj);
		else
			snprintf(str, LINESIZE, "%s", dot->subj);
		printf("Subject: %s\n", str);
	} else
		getstr(str, LINESIZE, "Subject: ");
		
	fprintf(f, "Subject: %s\n", str);
	
	if (reply)
		fprintf(f, "In-Reply-To: %s\n", dot->id);
	
	printf("Enter message text (end with \"/ex\" or \".\" on a line by itself):\n");
	fflush(stdout);

cont:
	do {
		fgets(str, LINESIZE, stdin);
		if ( strcmp( str, ".\n") && strcmp( str, "/ex\n")) fputs(str, f);
	} while (strcmp(str, ".\n") && strcmp(str, "/ex\n"));

retry:
	getstr(str, LINESIZE, "Deliver (Y/n/c/?): ");
	if (!strcmp(str, "?")) {
		printf("Answering \"N\" here will cancel the message. Answering \"C\" will\n");
		printf("let you continue writing the message. Answering anything else will\n");
		printf("proceed with delivering the message to the recipient.\n");
		goto retry;
	}
	if (!strcasecmp(str, "c")) {
		printf("Continue entering message text\n(end with \"/ex\" or \".\" on a line by itself):\n");
		fflush(stdout);
		goto cont;
	}
	
	if (fclose(f)) {
		printf("Ouch, could not close temporary file.\n");
		syslog(LOG_NOTICE, "do_send: Could not close temporary file.\n");
		return 0;
	}
	
	if (strcasecmp(str, "n")) {
		getstr(str, LINESIZE, "Request a delivery receipt? (y/N): ");
			if (!strcasecmp(str, "y")) {
			sprintf(str, "%s -N success,delay,failure -oem -t < %s", BIN_AXMAIL_SENDMAIL, tempMesg);
			system(str);
			printf("Message sent, delivery notification activated.\n");
		} else {
			sprintf(str, "%s -oem -t < %s", BIN_AXMAIL_SENDMAIL, tempMesg);
			system(str);
			printf("Message sent.\n");
		}
	} else
		printf("Message canceled.\n");
	
	if (remove(tempMesg)) {
		printf("Ouch, could not remove temporary file.\n");
		syslog(LOG_NOTICE, "do_send: Could not remove temporary file.\n");
		return 0;
	}

	return 0;
}

/* Kill a message */
 
int do_kill(int argc, char **argv)
{
	int i, msg = 0, cnt = 0;
	char *myargv[64];
	int myargc, argsmine;

	if (!(messages)) {
		printf("You have no messages.\n");
		return 0;
	}

	if (argc > 1) {
		argsmine = 0;
		for (i = 1; i < argc; i++)
			myargv[i] = argv[i];
		myargc = argc;
	} else {
		if (current == 0) {
			printf("No current message to kill.\n");
			return 0;
		}
		argsmine = 1;
		myargc = 2;
		myargv[1] = malloc(17);
		sprintf(myargv[1], "%i", current);
	}

	for (i = 1; i < myargc; i++) {
		msg = atoi(myargv[i]) - 1;

		if ((msg < 0) || (msg >= messages)) {
			printf("There's no message %s.\n", myargv[i]);
			continue;
		}

		dot = &message[msg];

		if ((dot->m_flag & MDELETED) == MDELETED) {
			printf("Message %i is already dead.\n", msg + 1);
			continue;
		}
		
		dot->m_flag |= MDELETED;
		cnt++;
	}
	
	if (cnt == 1) /* GCC warns here, but what the heck! 8-) */
		printf("Message %i killed.\n", msg + 1);
	else if (cnt > 1)
		printf("%i messages killed.\n", cnt);

	if (argsmine)
		for (i = 1; i < myargc; i++)
			free(myargv[i]);

	return 0;
}

/* Unkill a message */
 
int do_unkill(int argc, char **argv)
{
	int i, msg = 0, cnt = 0;
	char *myargv[64];
	int myargc, argsmine;

	if (!(messages)) {
		printf("You have no messages.\n");
		return 0;
	}

	if (argc > 1) {
		argsmine = 0;
		for (i = 1; i < argc; i++)
			myargv[i] = argv[i];
		myargc = argc;
	} else {
		if (current == 0) {
			printf("No current message to unkill.\n");
			return 0;
		}
		argsmine = 1;
		myargc = 2;
		myargv[1] = malloc(17);
		sprintf(myargv[1], "%i", current);
	}

	for (i = 1; i < myargc; i++) {
		msg = atoi(myargv[i]) - 1;

		if ((msg < 0) || (msg >= messages)) {
			printf("There's no message %s.\n", myargv[i]);
			continue;
		}

		dot = &message[msg];

		if ((dot->m_flag & MDELETED) != MDELETED) {
			printf("Message %i is not dead.\n", msg + 1);
			continue;
		}
		
		dot->m_flag ^= (dot->m_flag & MDELETED);
		cnt++;
	}
	
	if (cnt == 1) /* GCC warns here, but what the heck! 8-) */
		printf("Message %i unkilled.\n", msg + 1);
	else if (cnt > 1)
		printf("%i messages unkilled.\n", cnt);

	if (argsmine)
		for (i = 1; i < myargc; i++)
			free(myargv[i]);

	return 0;
}

/* Send a Fax */ 
int do_fax(int argc, char **argv)
{
        FILE *f;
	char str[LINESIZE + 1];
	int i;

        int reply = 0;

        if (!strncmp(argv[0], "sr", 2)) {
                reply = 1;
                if (argc == 1) {
                        if (current == 0) {
                                printf("No current message to reply to.\n");
                                return 0;
                        }
                        i = current;
                } else
                        i = atoi(argv[1]);

                i--;
                if ((i < 0) || (i >= messages)) {
                        printf("There's no message %s.\n", argv[1]);
                        return 0;
                }

                dot = &message[i];
        }

        if ((f = fopen(tempMesg, "w")) == NULL) {
                printf("Could not create temporary file.\n");
                syslog(LOG_NOTICE, "do_send: Could not create temporary file.\n");
                return 0;
        }

        fprintf(f, "From: %s <%s@%s>\n", fullname, username, hostname);

        str[0] = '\0';
        if (argc != 1)  /* Recipient on command line */
                for (i = 1; i < argc; i++) {
                        if (i > 1)
                                strcat(str, " ");
                        strncat(str, argv[i], LINESIZE - strlen(str));
                }
        else {
        }

	printf("To: Fax Gateway\n");
	fprintf( f, "To: %s\n", faxgate );
        fprintf( f, "X-Mailer: %s\n", VERSION );
        fprintf( f, "X-Origin: Amateur Radio Services\n" );
        goto prio;;

        /* adding priority receive rule */

prio:
/*         if (reply) {
                if (strncasecmp(dot->subj, "Re: ", 3))
                        snprintf(str, LINESIZE, "Re: %s", dot->subj);
                else
                        snprintf(str, LINESIZE, "%s", dot->subj);
                printf("Subject: %s\n", str);
        } else */
		getstr(str, LINESIZE, "Enter a header <firstname@fullphone Brief note here>\nEx: john@16195551212 Hi From Packet\nHeader: ");

	fprintf(f, "Subject: %s\n", str);

        if (reply)
                fprintf(f, "In-Reply-To: %s\n", dot->id);

        printf("Enter fax message (end with \"/ex\" or \".\" on a line by itself):\n");
        fflush(stdout);

cont:
        do {
/*                fprintf( f, "X-Mailer: %s\n", VERSION );
		fprintf( f, "X-Origin: Amateur Radio Services\n" ); */
		fgets(str, LINESIZE, stdin);
                if ( strcmp( str, ".\n") && strcmp( str, "/ex\n")) fputs(str, f);
        } while (strcmp(str, ".\n") && strcmp(str, "/ex\n"));

retry:
        getstr(str, LINESIZE, "Deliver (Y/n/c/?): ");
        if (!strcmp(str, "?")) {
                printf("Answering \"N\" here will cancel the message. Answering \"C\" will\n");
                printf("let you continue writing the message. Answering anything else will\n");
                printf("proceed with delivering the message to the recipient.\n");
                goto retry;
        }
        if (!strcasecmp(str, "c")) {
                printf("Continue entering message text\n(end with \"/ex\" or \".\" on a line by itself):\n");
                fflush(stdout);
                goto cont;
        }

        if (fclose(f)) {
                printf("Ouch, could not close temporary file.\n");
                syslog(LOG_NOTICE, "do_send: Could not close temporary file.\n");
                return 0;
        }

        if (strcasecmp(str, "n")) {
                sprintf(str, "%s -oem -t < %s", BIN_AXMAIL_SENDMAIL, tempMesg);
                system(str);
                printf("Message sent.\n");
        } else
                printf("Message canceled.\n");

        if (remove(tempMesg)) {
                printf("Ouch, could not remove temporary file.\n");
                syslog(LOG_NOTICE, "do_send: Could not remove temporary file.\n");
                return 0;
        }

        return 0;
}
