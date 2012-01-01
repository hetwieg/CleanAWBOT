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
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

using namespace std;
using std::string;

// Algemene bot info (Later naar config.h)
string version = "1.0";

// Functions voor algemene zaken
char* stc(string value);

// Login benodigtheden
static int entered=0,entercode=0;
void handle_enter (int rc);

// Global bot info
struct {
	char* host;
	int   port;
	char* world;

	int   citnumber;
	char* priffpassword;

	char* name;
	int   pos_x;
	int   pos_y;
	int   pos_z;
	int   look;
	int   avatar;

	void* instance; // More Info @ http://wiki.activeworlds.com/index.php?title=SDK_Multiple_Instances
} bot;

// Handlers
void handle_avatar_add (void);

// Start main program
main (int argc, char *argv[])
{
	int rc;
	char tmp[255], bot_version[255];
	string strtmp;

	// Introductie bericht
	sprintf(bot_version, "CleanBot by Sebastiaan (hetwieg.nl) - Version %s", version.c_str());
	printf("%s\n\n",bot_version);

	// Config file laden
	sprintf(tmp, "%s.conf", argv[0]);  // {programmanaam}.conf
	ConfigFile config(tmp);

	// Start reading config file
	printf("Reading config file: ");
	
	bot.host  = stc(config.read<string>("host","auth.activeworlds.com"));
	bot.port  = config.read("port",6702);
	bot.world = stc(config.read<string>("world", "AWEBots"));

	bot.citnumber     = config.read("citnumber", 1);
	bot.priffpassword = stc(config.read<string>("password", "geheim"));

	bot.name = stc(config.read<string>("botname", "BotName"));
	bot.pos_x   = config.read("pos_x", 0);
	bot.pos_y   = config.read("pos_y", 0);
	bot.pos_z   = config.read("pos_z", 0);
	bot.look    = config.read("look", 0);
	bot.avatar  = config.read("avatar", 0);
	
	printf ("[OK]\n"); 

	// Start login proceduren AW	
	printf("Initializing AW API: ");
	if(rc = aw_init(AW_BUILD)) {
		printf("[ERROR: %d]\n", rc);
		return rc;
	}
	printf("[OK]\n");

	printf("Connecting and creating bot instance: ");
	if(rc = aw_create(bot.host, bot.port, &bot.instance)) {
		printf("[ERROR: %d]\n", rc);
		return rc;
	}
	printf("[OK]\n");
  
	// Handlers koppelen
	aw_event_set(AW_EVENT_AVATAR_ADD, handle_avatar_add);
	aw_callback_set(AW_CALLBACK_ENTER, handle_enter);

	printf("Logging in: ");
	aw_int_set(AW_LOGIN_OWNER, bot.citnumber);
	aw_string_set(AW_LOGIN_PRIVILEGE_PASSWORD, bot.priffpassword);
	aw_string_set(AW_LOGIN_APPLICATION, bot_version);
	aw_string_set(AW_LOGIN_NAME, bot.name);
	if(rc = aw_login()) {
		printf("[ERROR: %d]\n", rc);
		aw_term();
		return rc;
	}
	printf("[OK]\n");

	// World Enter
	printf("Entering world (%s): ", bot.world);
	entered=false;
	aw_bool_set(AW_ENTER_GLOBAL,true);
	if(rc = aw_enter(bot.world)) {
		printf("[ERROR: %d]\n", rc);
		return rc;
	}

	while (1) {
		if(entered) break;
		aw_wait(10);
	}

	if (entercode) {
		printf("[ERROR: %d]\n", entercode);
		aw_destroy();
		aw_wait(0);
		aw_term();
		return rc;
	}
	printf ("[OK]\n");

	// Login postie
	printf ("Change location in world and update state: ");
	aw_int_set (AW_MY_X, bot.pos_x);
	aw_int_set (AW_MY_Y, bot.pos_y);
	aw_int_set (AW_MY_Z, bot.pos_z);
	aw_int_set (AW_MY_YAW, bot.look);
	aw_int_set (AW_MY_TYPE, bot.avatar);
	if(rc = aw_state_change()) {
		printf("[ERROR: %d]\n", rc);
		return rc;
	}
	printf("[OK]\n");

	// Bot is ingelogt crear update loop (en afsluit controle)
	printf("type q <cr> for quit\n");
	while(1) {
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
	
		if(retval) {
			if(getchar() == 'q') {
				break;
			}
		}

		aw_wait(10);
	}

	printf("Close everything down: ");
	aw_destroy();
	aw_wait(0);
	aw_term();
	printf ("[OK]\n");
	return 0; 
}

void handle_enter(int rc)
{
	entered=1;
	//printf("Enter code=%d\n",rc);
	entercode=rc;
}

// Handlers
void handle_avatar_add (void)
{
	char message[AW_MAX_ATTRIBUTE_LENGTH+1];
	//  printf ("avatar_add by %s: %s\n",(char *)aw_user_data(), aw_string (AW_AVATAR_NAME));
	if(aw_string(AW_AVATAR_NAME)[0]=='[') return;  // don't greet bots
  
	sprintf(message, "Hello %s", aw_string(AW_AVATAR_NAME));
	aw_say(message);
	printf("Bot %s\n", message);
}

// Functions voor algemene zaken
char* stc(string value)
{
  //printf("%s\n",value);

  char* cstr;
  cstr = new char [value.size()+1];
  strcpy(cstr, value.c_str());
  
  return cstr;
}