#include "Aw.h"
#include "Reson.h"
#include "ConfigFile.h"
#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>
//#include <errno.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include <time.h>
#include <signal.h>
#include <unistd.h>
#define true 1
#define false 0

using std::string;
using std::cout;
using std::endl;

//extern int h_errno;

static int entered=0,entercode=0;
static char* level_dir="levels";
static char* greed_message="";

void handle_avatar_add (void);
void handle_chat (void);
void handle_enter (int rc);
void save_world_settings(char* name);
void load_world_settings(char* name, int session);
char* stc(string value);

main (int argc, char *argv[])
{
  char tmp[255];
  int rc;
  int i,n;
  
  // Config file laden
  sprintf(tmp, "%s.conf", argv[0]);
  ConfigFile config(tmp);
  
  // Defenitions foor config file
  char* host = stc(config.read<string>("host","auth.activeworlds.com"));
  int port = config.read("port",6670);
  int cit  = config.read("cit", 1);
  char* priff = stc(config.read<string>("priff", "geheim"));
  char* botname = stc(config.read<string>("name", "Level"));
  char* world = stc(config.read<string>("world", "AW"));
  int loc_x = config.read("loc_x", 0);
  int loc_y = config.read("loc_y", 0);
  int loc_z = config.read("loc_z", 0);
  int loc_yaw = config.read("loc_yaw", 0);
  int greed = config.read("greed", 1);
  level_dir = stc(config.read<string>("level_dir", "levels"));
  greed_message = stc(config.read<string>("greed_message", "Hallo %s"));

  /* initialize Active Worlds API */
  if(rc = aw_init(AW_BUILD)) {
	printf ("Unable to initialize API (reason %d)\n", rc);
	exit (1);
  }
  printf("AW Init called %d\n", AW_BUILD);
  
  /* create bot instance */
  if (rc = aw_create(host, port, 0 )) {
	printf ("Unable to create bot instance (reason %d) %s@%d\n", rc,  host, port);
	exit (1);
  }
  printf("AW create called %s@%d\n", host, port);
  
  /* install evend handlers */
  aw_event_set(AW_EVENT_CHAT, handle_chat);
  aw_callback_set(AW_CALLBACK_ENTER, handle_enter);

  if(greed == 1) {
	aw_event_set(AW_EVENT_AVATAR_ADD, handle_avatar_add);
	printf("Greedings ar on with '%s'\n", greed_message);
  }
  else {
	printf("Greedings ar off\n");
  }  

  /* log bot into the universe */
  aw_int_set(AW_LOGIN_OWNER, cit);
  aw_string_set(AW_LOGIN_PRIVILEGE_PASSWORD, priff);
  aw_string_set(AW_LOGIN_APPLICATION, "Hetwieg.NL Level Manager V0.0.0.1");
  aw_string_set(AW_LOGIN_NAME, botname);
  printf("aw_login call\n");
  if (rc = aw_login ()) {
	printf ("Unable to login (reason %d)\n", rc);
	exit (1);
  }
  printf("aw_login called for %s @ %d\n", botname, cit);

  /* log bot into the world  */
  printf ("Entering to world %s\n", world);
  entered=false;
  aw_bool_set(AW_ENTER_GLOBAL,true);
  if (rc = aw_enter(world)) {
	printf ("Unable to enter world (reason %d)\n", rc);
	exit (1);
  }
  while (1) {
	if (entered) break;
	aw_wait(10);
  }
  
  if (entercode) {
	printf ("Unable to enter world (reason %d)\n", rc);
	aw_destroy();
	aw_wait(0);
	aw_term();
	exit (1);
  }
  printf ("Entered to world %s \n",world);

  /* announce our position in the world */
  aw_int_set(AW_MY_X, loc_x);
  aw_int_set(AW_MY_Y, loc_y);
  aw_int_set(AW_MY_Z, loc_z);
  aw_int_set(AW_MY_YAW, loc_yaw);
  if(rc = aw_state_change()) {
	printf ("Unable to change state (reason %d)\n", rc);
	aw_destroy();
	aw_wait(0);
	aw_term();
	exit(1);
  }
  printf ("State changed \n");
  
  /* main event loop */
  printf("type q <cr> for quit\n");
  while (1) {
	fd_set rfds;
	struct timeval tv;
	int retval;
	
	/* Watch stdin (fd 0) to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	/* Wait up to 100 mseconds. */
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	
	retval = select(1, &rfds, NULL, NULL, &tv);
	/* Don't rely on the value of tv now! */
	
	if (retval) {
	  if (getchar() == 'q') {
	aw_destroy();
	aw_wait(0);
	aw_term();
	return 0;
	  }
	};
	aw_wait(10);
  }
  
  /* close everything down */
  aw_destroy ();
  aw_wait(0);
  aw_term ();
  return 0; 
}

void handle_avatar_add (void)
{
  char message[AW_MAX_ATTRIBUTE_LENGTH+1];
//  printf ("avatar_add by %s: %s\n",(char *)aw_user_data(), aw_string (AW_AVATAR_NAME));
  if (aw_string(AW_AVATAR_NAME)[0]=='[') return;  // don't greet bots
  
  sprintf(message, greed_message, aw_string (AW_AVATAR_NAME));
  aw_say(message);
  printf("Bot %s\n", message);
  /* log the event to the console */

}
void handle_enter(int rc)
{
  entered=1;
  printf("Enter code=%d\n",rc);
  entercode=rc;
}

static char* chat_type[] = {"Chat ", "Broadcast ", "Whisper "};

void handle_chat(void)
{
  char message[AW_MAX_ATTRIBUTE_LENGTH+1];
  int session;
 
  char tmp[AW_MAX_ATTRIBUTE_LENGTH+1];
  
  sprintf(message, "%s", aw_string(AW_CHAT_MESSAGE));
  session = aw_int(AW_AVATAR_SESSION);
  
  printf("%s %s: %s\n",chat_type[aw_int (AW_CHAT_TYPE)], aw_string(AW_AVATAR_NAME),message);

  sprintf(tmp, "%s version", aw_string(AW_LOGIN_NAME)); //TODO: CAPS CONTROLE
  if(strcmp(message, "\\version") == 0 || strcmp(message, "/version") == 0 || strcmp(message, tmp) == 0) {
	aw_whisper(session, "Ik ben level manager V0.0.0.1");
  }
  
  char* tmp2 = strtok(message, " ");
  if(strcmp(tmp2, "\\load") == 0 || strcmp(tmp2, "/load") == 0) {
	aw_whisper(session, "World config woord geladen");
	load_world_settings(strtok (NULL, " ,.-"), session);
  } 

  if(strcmp(tmp2, "\\save") == 0 || strcmp(tmp2, "/save") == 0) {
	aw_whisper(session, "World config woord opgeslagen");
	save_world_settings(strtok (NULL, " ,.-"));
  }  
}

void save_world_settings(char* name)
{
  FILE* fp;
  int   rc;
  int   i;
  int   read_only;
  char  string[AW_MAX_ATTRIBUTE_LENGTH + 1];
  
  sprintf(string, "%s/%s.txt",level_dir, name);
  fp = fopen (string, "w");
  
  for (i = 0; i < 200; i++) {
	rc = aw_world_attribute_get(i, &read_only, string);
	if (rc == RC_INVALID_ATTRIBUTE)
	  continue;
	if (rc != RC_SUCCESS)
	  break;
	
	if (read_only)
	  continue;
	
	fprintf(fp, "%d %s\n", i, string);
  }  
  fclose (fp);
}

void load_world_settings(char* name, int session)
{
  FILE *fp;
  int   rc;
  int   id;
  char  string[4096];

  sprintf(string, "%s/%s.txt",level_dir, name);
  fp = fopen (string, "r");

  if(fp == NULL) {
	printf ("Unable to open %s/%s.txt\n", level_dir, name);
	return;
  }

  for(;;) {
	rc = fscanf(fp, "%d%[^\n]", &id, string);
	
	if (rc == EOF)
	  break;
	
	if(rc != 2) {
	  printf("Unable to load attributes: invalid format\n");
	  fclose(fp);
	  return;
	}
  
	/* string + 1 skips the space delimiter */
	rc = aw_world_attribute_set(id, string + 1);
	if (rc != RC_SUCCESS) {
	  printf("Unable to set attribute (reason %d)\n", rc);
	  fclose (fp);
	  return;
	}
  }
  
  fclose (fp);

  rc = aw_world_attributes_send(session);
  if(rc != RC_SUCCESS) {
	printf("Unable to load attributes (reason %d)\n", rc);
  }
  else {
	printf("Attribute load complete for %d\n", session);
  }
}

char* stc(string value)
{
  //printf("%s\n",value);

  char* cstr;
  cstr = new char [value.size()+1];
  strcpy(cstr, value.c_str());
  
  return cstr;
}
