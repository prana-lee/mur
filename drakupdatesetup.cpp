// Setup wizard - drakupdatesetup.cpp
/*****************************************************************************
/*  Mandrake Update Text                                                     *
/*  Copyright (C) 2001 Prana                              *
/*                                                                           *
/*                                                                           *
/*  This program is free software; you can redistribute it and/or modify     *
/*  it under the terms of the GNU General Public License as published by     *
/*  the Free Software Foundation; either version 2 of the License, or        *
/*  (at your option) any later version.                                      *
/*                                                                           *
/*  This program is distributed in the hope that it will be useful,          *
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
/*  GNU General Public License for more details.                             *
/*                                                                           *
/*  You should have received a copy of the GNU General Public License        *
/*  along with this program; if not, write to the Free Software              *
/*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA      *
/*****************************************************************************/
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include "engine.h"
#include "config.h"
#include "util.h"
using namespace std;

#define version "1.1"
#define macro_ask_question() (cout << endl << "Your answer:")
#define macro_get_line() (cin.getline(answer, 1024))

void display_logo()
{
   cout << "Mandrake Update Robot Setup Wizard " << version << endl
      << "Copyright (C) 2001 Prana" << endl
      << "License: Freeware (GNU Public License v2)" << endl
      << endl
      << "---------------------------------------------" << endl;
}

void say_goodbye()
{
   cout << endl << endl
        << "Thank you for using Linux Mandrake and Mandrake Update Robot!" << endl
        << "For further assistance go to the following resources:" << endl        
        << "- http://www.mandrakeexpert.com" << endl
        << "- http://www.mandrakecampus.com" << endl 
        << "- http://www.mandrakeforum.com" << endl
        << "Thanks,\nPrana <*****@*****.***>" << endl;
}

char *questions[] = {

// READINESS
"Welcome to MUR (Mandrake Update Robot Setup) Wizard. MUR is a program that\n\
automatically upgrades security patches for Linux Mandrake 7.1 or higher\n\
installed on your computer. This program is very useful to maintain the\n\
security of a very large network settings, and it can also be used to maintain\n\
patches for a single personal computer with an ADSL/Cable connection.\n\
\n\
NOTE: This program is *NOT* designed for 56k modem dial-up users.\n\
* NOTE: Currently SOCKS5 is not yet supported. We will provide instruction on\n\
how to run this tool on a network environment that uses SOCKS5 in v1.0. To\n\
accept the default setting for each question, just hit *Enter* and continue\n\
for the next question. If you want to abort the setup wizard, you can just\n\
always *Ctrl-C*\n\n\
Now, are you ready to continue the setup wizard? [Y/N]",
// CONNECTION_TYPE_ETHERNET
"Are you using a LAN/Cable modem/ADSL/T1/T3 for your internet connection? [Y/N]",
// CLIENT_OR_SERVER
"How do you want to setup MUR this computer? Please choose from the following\n\
menu:\n\
\tA. I want to setup this computer as the 'Primary Mandrake Update Robot LAN\n\
\t   server' that provide an NFS/Samba update share to other clients in the\n\
\t   LAN or provide an FTP mirror to other computers.\n\
\tB. I want to setup this computer just as the local/generic Mandrake\n\
\t   server or workstation that will update RPM packages from the 'Primary\n\
\t   Mandrake Update Robot LAN server'\n\
\tC. I only have one computer equipped with an ADSL/cable modem connection\n",
// BEHIND_A_PROXY
"Are you connected to the internet behind a proxy server? Answer: [Y/N]",
// PROXY_HOST_NAME
"What is your proxy server host name? (DON'T answer with a prefix ftp:// or\n\
http://!) [e.g: 10.0.0.1 or proxyserver.somewhere.com]",
// PROXY_PORT
"What port does your proxy server use? Ask your system administrator for\n\
details. Usually it's port 3128, 8080, 1080, etc.",
// PROXY_TUNNEL
"Does your proxy server requires HTTP tunnelling? Usually it's no, but ask\n\
your network administrator or Internet Service Provider if you don't know.\n\
Answer: [Y/N]",
// PROXY_AUTHENTICATION_REQUIRED
"Does your proxy server require you to authenticate with login/password in\n\
order to connect to the internet? Answer: [Y/N]",
// PROXY_USERNAME
"What is your USER NAME or LOGIN ID for your proxy server? [e.g: loginname]",
// PROXY_PASSWORD
"What is your PASSWORD for your proxy server? [e.g: wga98cs02sx]\n\
*Note: this password (max. 128 characters) will be stored as a clear text in\n\
the main configuration file, which is only readable by the 'root' user, NOBODY\n\
else can read it, so don't worry!\n\n***NOTE*** :What you type right now won't\n\
appear in the screen!\n",
// MIRROR_LIST_URL
"Do you want to get the default *Official Mirror List* URL from Linux\n\
Mandrake's website or do you have your own mirror? If you want to set up your\n\
own custom mirror server, you can specify a different URL. To see what the\n\
file mirrorsfull.list file looks like, take a look at the sample file\n\
/etc/urpmi/mirrorsfull.list.sample and put your it in your local LAN's FTP\n\
or HTTP server. If you choose the default, just hit *Enter*, otherwise enter\n\
your custom mirror list.\n\n\
Tips: If you want to make your own Mandrake mirror, you can use either of the\n\
following commands:\n\
* 'mirrordir' (RPM package: mirrordir) (manual: type 'man mirrordir')\n\
* 'fmirror' (RPM package: fmirror) (manual: type 'man fmirror')\n\
* 'rsync' (RPM package: rsync) (manual: type 'man rsync')\n\
All of them are available from your Mandrake installation CD. If you don't have\n\
your own preference of a custom mirror, this program will set it to the default\n\
official mirror list file from Mandrake's website. Just hit *Enter* if you want\n\
to accept the default setting!\n\
[Default: http://www.linux-mandrake.com/mirrorsfull.list]\n",
// FAVORITE_FTP_MIRROR
"If you already know a specific FTP mirror that you want to fetch the RPM\n\
packages from, please specify it below. The format must be like\n\
'ftp://ftp.local.server.com/pub/Mandrake/updates'.\n\
For example: 'ftp://rpmfind.net/linux/Mandrake/updates'. This will be your\n\
primary FTP mirror (not mirror list, don't get confused) for RPM updates. This\n\
is useful if you server your own FTP server in your LAN for updating other\n\
Mandrake servers and workstations. If you don't know, just leave it blank and\n\
hit *Enter*.",
// UPDATED_RPM_PACKAGE
"Where do you want to save the downloaded the update RPM packages? Note that\n\
the partition where this directory resides requires around 300 Megabytes of\n\
hard drivespace (for future updates!). If you're not sure, go to another\n\
console (try shortcuts like Ctrl-Alt-F1, Ctrl-Alt-F2, Ctrl-Alt-F3, etc), and\n\
then login, and type \"df -h /var\".\n\n\
Tips:\n\
* You can use this directory and export it with NFS or Samba and have other\n\
Mandrake servers and workstations with identical RPM packages to fetch only\n\
from this directory. 
* The directory /var/cache/grpmi is also used by the GUI MandrakeUpdate,\n\
so in case you want to run the GUI version of MandrakeUpdate, it will be\n\
backward compatible.\n\n\
Hit *Enter* to accept the default settings ... [Default: /var/cache/grpmi]",
// MIRROR_LIST_FILE
"Where do you want to store the saved mirror list?\n\
[Default: /etc/urpmi/mirrorsfull.list]",
// DOWNLOADED_FILE_LIST
"Where do you want to store the downloaded file list?\n\
[Default: /etc/urpmi/ftp_directory_listing_only]",
// GNUPG_CHECK_SIGNATURE
"If there is an update RPM package which has a signature that cannot be\n\
verified/trusted from Linux Mandrake Security Team's GNU Privacy Guard\n\
signature, would you prefer to install the package manually and verify it?\n\
If you choose YES then MUR will check the GnuPG signature. If there is an\n\
unverified RPM package updates, MUR will e-mail you a comprehensive report\n\
about it. This is useful in the case that your primary mirror is poisoned.\n\
Although it is very rare (unless the system administrator of that server\n\
isn't worth to be paid), it can happen. It is wise if you take an extra step\n\
checking the security by saying \"YES\" [Answer: Y/N]",
// CUSTOM_KERNEL_RECOMPILE
"This question is for those who have a custom Linux kernel configuration:\n\
The file /usr/src/linux/.config (from configuration) must be present if\n\
you wish to do an automatic kernel recompilation. Note that development\n\
packages such as kernel-header and compilers such as egcs or gcc must be\n\
installed in order to perform this process. Since automatic kernel\n\
recompilation is one of the strongest feature in Mandrake Update Robot,\n\
it is recommended that you enable this. If you want to modify the script,\n\
please edit the file /etc/urpmi/kernel_recompile.sh. Note that unless\n\
you're an expert Linux user, you're not recommended to edit it. Do you want\n\
me Mandrake Update Robot to automatically recompile your kernel whenever there\n\
is an update? [Answer: Y/N]",
// DELETE_RPM_CACHE
"This question is for those who runs MUR only as a client, not the primary\n\
update server. Do you want to save disk space by letting MUR to automatically\n\
delete the updated RPM packages in the cache directory? 'NO' is probably good\n\
for you, in case you still share it for other computers via NFS/Samba.\n\
[Answer: Y/N]",
// EMAIL_REPORT
"Where should I report all the security updates?\n\
NOTE: Postfix or Sendmail (both are mail transfer agent) must be properly\n\
configured in order to receive/send e-mails. If you have Postfix, don't forget\n\
to set the setting 'myhostname' to this machine's fully qualified domain name.\n\
Otherwise, you won't be able to receive any e-mail report. Now, please enter\n\
the e-mail address where MUR should send the full disclosure of security update\n\
report. [Default: root@localhost]"
};

bool test_valid_ftp_mirror (const char *ftphostname)
// not yet ..
{
   return true;
}

bool test_write_file (const char *filename)
// Test if the file can be written ..
{
   ofstream outputfile(filename);
   bool write_result = false;
   
   if (outputfile)
   {
      outputfile.close();
      unlink(filename);
      write_result = true;
   }
   return write_result;
}

bool test_valid_mirror_file (const char *filename)
// Test if the file is a valid mirror list
// This test is just to prevent stuff like http 404 error, 403 error
// that results in an HTML file with tags.. now we check the tag
{
   ifstream inputfile(filename);
   bool return_result = true;
   string line;
   
   inputfile >> line;
   while (inputfile && return_result)
   {
      if (line.find("<") != -1) return_result = false;
      inputfile >> line;
   }
   inputfile.close();   
   return return_result;
}

bool set_time_out(int descriptor, int sec, int usec)
/*
   This code is from my program SubNetwork Explorer
*/
{
   struct timeval timeout;
   fd_set fread;

   fcntl(descriptor,F_SETFL,O_ASYNC);   
   FD_ZERO(&fread);
   FD_SET(descriptor,&fread);
   timeout.tv_sec = sec;
   timeout.tv_usec = usec;
   return (select (descriptor+1, &fread, NULL, NULL, &timeout) != -1);
}
bool detect_network(struct hostent *host_info, int port)
/*
   This code is from my program SubNetwork Explorer
*/
{
   int scan_result;
   int sock_descriptor;
   struct sockaddr_in sock;

   if ((sock_descriptor = socket(PF_INET, SOCK_STREAM, 0)) < 0)
     return false;
   bzero(&sock, sizeof(sockaddr_in));
   sock.sin_family = AF_INET;			// set the family to AF_INET
   sock.sin_port = htons(port);			// set the port number
   sock.sin_addr = *((struct in_addr *) host_info->h_addr_list[0]);
   set_time_out(sock_descriptor, 0, 0);
   scan_result = connect(sock_descriptor, (struct sockaddr *) &sock, sizeof(sock));
   // if connect is successful
   if (scan_result < 0)
	switch(errno)
	{
           case ENETUNREACH:
              cout << "* ERROR: Network can't be reach - maybe it doesn't exist." << endl;
              break;        
           case ETIMEDOUT:
              cout << "* ERROR: Connection timed out :(" << endl;
              break;
        }
   if (scan_result = !scan_result)
   // This is correct -> "=!" not "!=". It's not wrong.
   {
      cout << "--== The network setting is okay ==--" << endl;
   }
   close(sock_descriptor);
   return scan_result;
}

void ask_questions()
{
   char *answer = new char[1024+1];
   DIR *var_cache_grpmi;
   int proxy_port_number;
   bool behind_a_proxy = false;
   bool proxy_login_necessary = false;
   bool invalid_proxy = true;
   bool valid_answer = false;
   bool valid_password = false;
   bool valid_mirror_url = false;
   bool proxy_check_port = false;
   bool proxy_check_number = false;
   bool autocompile = false;
   struct hostent *host_info;
   drakupdateconfig config;
   string sTemp;
   unsigned int nTemp;
   char cTemp = ' ';
   string bootload;
   const char *password_question = "Proxy password: ";
   const char *verify_password_question = "Verify the proxy password again:";
   string sProxy, sAuth, pass1, pass2, mailhost;
   FILE *Pipe;
   char *read_buffer;
   char the_buffer[1024];
   string read_buffer_string;   
   string exec_string;
   
   try
   {
      for (int i = 0; i < sizeof(questions)/sizeof(char *); i++)
      {
         cout << endl << i+1 << ". " << questions[i] << endl;       
         if (i != PROXY_PASSWORD || !proxy_login_necessary)
         {
            cout << "Your answer:";
            cin.getline(answer, 1024);
         }
         else
         {
            pass1 = getpass(password_question);
            pass2 = getpass(verify_password_question);
         }
         switch (i)
         {
            case READINESS:
               if (!(toupper(answer[0]) == 'Y' || answer[0]==0))
                  throw string("It appears that you're not ready yet. Please read the instructions carefully");
               if (toupper(answer[0]) == 'Y')
               {
                  cout << endl << "*NOTE: Your configuration will be reset now. Please don't abort the setup" << endl;
                  config.reset();
               }
               break;
            case CONNECTION_TYPE_ETHERNET:
               if (!(toupper(answer[0]) == 'Y' || answer[0]==0))
                  throw string("You have to have a permanent internet connection, not a 56k dial-up modem.");
               break;
            case CLIENT_OR_SERVER:
               cTemp = toupper(answer[0]);
               sTemp = cTemp;
               while (cTemp != 'A' && cTemp != 'B' && cTemp != 'C')
               {
                   cout << "* ERROR: Invalid answer! Please select only A, B, or C!";
                   macro_ask_question();
                   macro_get_line();
                   cTemp = toupper(answer[0]);
                   sTemp = cTemp;                   
               }
               config.set_config(config_options[CLIENT_OR_SERVER], sTemp.c_str());
               break;
            case BEHIND_A_PROXY:
               cTemp = toupper(answer[0]);
               sTemp = cTemp;
               while (cTemp != 'Y' && cTemp != 'N')
               {
                   cout << "* ERROR: Invalid answer! Please select only yes/no!";
                   macro_ask_question();
                   macro_get_line();
                   cTemp = toupper(answer[0]);
                   sTemp = cTemp;
               }
               behind_a_proxy = (sTemp[0] == 'Y');
               config.set_config(config_options[BEHIND_A_PROXY], sTemp.c_str());
               if (!behind_a_proxy)
                  i+=6;
               break;
            case PROXY_HOST_NAME:
                  host_info = gethostbyname(answer);
                  while (invalid_proxy = (host_info == NULL))
                  {
                      cout << endl
                      	  << "* ERROR: Unable to resolve proxy host name!" << endl
                          << "Please try again and check your network settings and re-enter your proxy server name below!" << endl;
                      macro_ask_question();
                      macro_get_line();
                      host_info = gethostbyname(answer);
                  } // end while
                  config.set_config(config_options[PROXY_HOST_NAME], answer);
                  break;
            case PROXY_PORT:
               	  nTemp = atoi(answer);
                  sTemp = answer;
                  proxy_check_number = nTemp <= 0 || nTemp > 0xFFFF-2;
                  proxy_check_port = detect_network(host_info, nTemp);                  
                  while (!proxy_check_port)
                  {
                     if (proxy_check_number)
                        cout << "* ERROR: "
                           << "\tYour input " << sTemp << " is invalid."
                           << "\tThe proxy's port number must be between 0 and 65535!"
                           << "\tPlease try again!" << endl;
                     else if (!proxy_check_port)
                        cout << "* ERROR: The proxy server you specified doesn't run on port "
                             << sTemp << "!" << endl
                             << "\t Please check your proxy settings again!";
   		     macro_ask_question();
                     macro_get_line();
                     nTemp = atoi(answer);
                     sTemp = answer;
                     proxy_check_number = nTemp <= 0 || nTemp > 0xFFFF-2;
                     proxy_check_port = detect_network(host_info, nTemp);                     
                   }
               config.set_config(config_options[PROXY_PORT], sTemp.c_str());               
               break;

            case PROXY_TUNNEL:
                  cTemp = toupper(answer[0]);
                  sTemp = cTemp;
                  while (cTemp != 'Y' && cTemp != 'N')
                  {
                     cout << "* ERROR: Invalid answer! Please select only yes/no!";
                     macro_ask_question();
                     macro_get_line();
                     cTemp = toupper(answer[0]);
                     sTemp = cTemp;                   
                  }
               config.set_config(config_options[PROXY_TUNNEL], sTemp.c_str());
               break;               
            case PROXY_AUTHENTICATION_REQUIRED:
            // FIX - skip user/pass ask
                  cTemp = toupper(answer[0]);
                  sTemp = cTemp;
                  while (cTemp != 'Y' && cTemp != 'N')
                  {
                     cout << "* ERROR: Invalid answer! Please select only yes/no!";
                     macro_ask_question();
                     macro_get_line();
                     cTemp = toupper(answer[0]);
                     sTemp = cTemp;
                  }               
               proxy_login_necessary = (cTemp == 'Y');
               if (!proxy_login_necessary)
                  i+=2;
               config.set_config(config_options[PROXY_AUTHENTICATION_REQUIRED], sTemp.c_str());
               break;
            case PROXY_USERNAME:
                  sTemp = answer;
                  while (sTemp.length()==0)
                  {
                     cout << "* ERROR: Zero-length username is not allowed when proxy authentication is required";
                     macro_ask_question();
                     macro_get_line();
                     sTemp = cTemp;                   
                  }
               config.set_config(config_options[PROXY_USERNAME], sTemp.c_str());
               break;
               
            case PROXY_PASSWORD:
                  valid_answer = (pass1.length() > 0);
                  valid_password = (pass1 == pass2);
                  while (!valid_answer || !valid_password)
                  {
                     if (!valid_answer)
                     cout << "* ERROR: Zero-length password is not allowed when proxy authentication is required"
                          << endl;
                     else if(!valid_password)
                     cout << "* ERROR: Password can't be verified. Please re-enter your password!" << endl;
                     pass1 = getpass(password_question);
                     pass2 = getpass(verify_password_question);
                     valid_answer = pass1.length() > 0;
                     valid_password = pass1 == pass2;
                  }
               config.set_config(config_options[PROXY_PASSWORD], pass1.c_str());
               pass1 = "";
               pass2 = "";
               break;
            case MIRROR_LIST_URL:
               sTemp = answer;
               if (sTemp.length() == 0)
                  sTemp = const_mirror_list_url;
               if (proxy_login_necessary)
               {
                  sProxy = config.get_config(config_options[PROXY_HOST_NAME]) + ":" +
                	   config.get_config(config_options[PROXY_PORT]);
                  sAuth =  config.get_config(config_options[PROXY_USERNAME]) + ":" +
                  	   config.get_config(config_options[PROXY_PASSWORD]);
               } else
               { // clear it - a must - if the user steps back
                  sProxy = "";
                  sAuth = "";
               }
               cout << "Checking mirror validity... Please wait .... " << endl;               
               valid_mirror_url = download_file(sTemp.c_str(), const_mirror_list_saved, is_a_file,
               sProxy.c_str(), sAuth.c_str(), config.get_config(config_options[PROXY_TUNNEL])=="Y" )
               && test_valid_mirror_file(const_mirror_list_saved);
               
               while (!valid_mirror_url)
               {
               // fix junk here!
                  cout 	<< "* ERROR: Either the mirror list you specified is invalid or the "
                     	<< "file that you've specified is"
                        << endl
                        << "\t incorrect (does not exist/permission restriction)."
                        << endl
                        << "\t Please check your network and/or proxy server settings too!"
                     	<< endl
                     	<< "\t Please also check that the mirror list URL you enter is correct!"
                        << endl
                        << "\t Now, what do you want to do?"
                        << endl
                        << "\t\tA. Change your proxy settings ...or..." << endl
                        << "\t\tB. Change the URL where you want to get the mirror list?"
                        << endl;
                  macro_ask_question();
                  macro_get_line();
                  if (toupper(answer[0]) == 'A')
                  {
                     i = BEHIND_A_PROXY-1;
                     valid_mirror_url = false;
                  } // endif toupper
                  else
                  {
                     cout << "Please re-enter the mirror list URL that you want to retrieve."
                          << endl
                          << "[Hit *Enter* to accept the default official value from Mandrake]"
                          << endl;
                     macro_ask_question();
                     macro_get_line();
                     sTemp = answer;
                     if (sTemp.length() == 0)
                        sTemp = const_mirror_list_url;
                     if (proxy_login_necessary)
                     {
                        sProxy = config.get_config(config_options[PROXY_HOST_NAME]) + ":" +
                	   config.get_config(config_options[PROXY_PORT]);
                        sAuth =  config.get_config(config_options[PROXY_USERNAME]) + ":" +
                  	   config.get_config(config_options[PROXY_PASSWORD]);
                     }   else
                     { // clear it - a must - if the user steps back
                        sProxy = "";
                        sAuth = "";
                     }
                     cout << "Checking mirror validity... Please wait .... " << endl;
                     valid_mirror_url = download_file(sTemp.c_str(), const_mirror_list_saved, is_a_file,
                     sProxy.c_str(), sAuth.c_str(), config.get_config(config_options[PROXY_TUNNEL])=="Y" )
                     && test_valid_mirror_file(const_mirror_list_saved);
                     
    } // end else
               } // end while
               if (i != BEHIND_A_PROXY-1)
                  cout << "Mirror list has been successfully retrieved!" << endl;
               else
                  cout << "Going back to step 4..." << endl;
               config.set_config(config_options[MIRROR_LIST_URL], sTemp.c_str());
               break;
            case FAVORITE_FTP_MIRROR:            
               sTemp = answer;
               while (!test_valid_ftp_mirror(sTemp.c_str()))
               {
                  cout << "* FTP mirror "<< sTemp << "has an invalid host name or directory path."
                     << "Please re-enter it again in format ftp://hostname.com/pub/Mandrake or "
                     << "just leave blank for default.";
                  macro_ask_question();
                  macro_get_line();
                  sTemp = answer;
               }
               config.set_config(config_options[FAVORITE_FTP_MIRROR],sTemp.c_str());
               break;
                
            case UPDATED_RPM_PACKAGE:
               sTemp = answer;
               if (sTemp.length() == 0)
                  sTemp = const_update_cache_dir;
               
               var_cache_grpmi = opendir(sTemp.c_str());
               if (!var_cache_grpmi)
               {
                  mkdir(sTemp.c_str(), S_IREAD | S_IWRITE | S_IEXEC);
                  
                  // try again after creating a directory
                  var_cache_grpmi = opendir(sTemp.c_str());
               }
               while (!var_cache_grpmi)
               {
                  cout << "* Error opening/creating directory "
                       << sTemp
                       << "!"
                       << endl
                       << "Please specify a valid new/existing directory to save RPM packages!";
                  macro_ask_question();
                  macro_get_line();
                  sTemp = answer;
                  if (sTemp.length()==0) sTemp = const_update_cache_dir;                  
                  var_cache_grpmi = opendir(sTemp.c_str());
                  if (!var_cache_grpmi)
                  {
                     mkdir(sTemp.c_str(), S_IREAD | S_IWRITE | S_IEXEC);
                     var_cache_grpmi = opendir(sTemp.c_str());
                  }
               }
               closedir(var_cache_grpmi);               
               config.set_config(config_options[UPDATED_RPM_PACKAGE],sTemp.c_str());
               break;
            case MIRROR_LIST_FILE:
               sTemp = answer;
               if (sTemp.length()==0) sTemp = const_mirror_list_saved;
               while (!test_write_file(sTemp.c_str()))
               {
                  cout << "* Error creating the file "<< sTemp << ". Please specify a valid file!";
                  macro_ask_question();
                  macro_get_line();
                  sTemp = answer;
                  if (sTemp.length()==0) sTemp = const_mirror_list_saved;
               }
               cout << "File " << sTemp << " status is OK!" << endl;
               config.set_config(config_options[MIRROR_LIST_FILE],sTemp.c_str());
               break;
            case DOWNLOADED_FILE_LIST:
               sTemp = answer;
               if (sTemp.length()==0) sTemp = const_temp_updatelist_file;
               while (!test_write_file(sTemp.c_str()))
               {
                  cout << "* Error creating the file "<< sTemp << ". Please specify a valid file!";
                  macro_ask_question();
                  macro_get_line();
                  sTemp = answer;
                  if (sTemp.length()==0) sTemp = const_temp_updatelist_file;
               }
               cout << "File " << sTemp << " status is OK!" << endl;
               config.set_config(config_options[DOWNLOADED_FILE_LIST],sTemp.c_str());
               break;               
            case GNUPG_CHECK_SIGNATURE:
                  cTemp = toupper(answer[0]);
                  sTemp = cTemp;
                  while (cTemp != 'Y' && cTemp != 'N')
                  {
                     cout << "* ERROR: Invalid answer! Please select only yes/no!";
                     macro_ask_question();
                     macro_get_line();
                     cTemp = toupper(answer[0]);
                     sTemp = cTemp;                   
                  }
                  config.set_config(config_options[GNUPG_CHECK_SIGNATURE], sTemp.c_str());
                  break;
            case DELETE_RPM_CACHE:
                  cTemp = toupper(answer[0]);
                  sTemp = cTemp;
                  while (cTemp != 'Y' && cTemp != 'N')
                  {
                     cout << "* ERROR: Invalid answer! Please select only yes/no!";
                     macro_ask_question();
                     macro_get_line();
                     cTemp = toupper(answer[0]);
                     sTemp = cTemp;                   
                  }
                  config.set_config(config_options[DELETE_RPM_CACHE], sTemp.c_str());
                  break;                  
            case CUSTOM_KERNEL_RECOMPILE:
                  cTemp = toupper(answer[0]);
                  sTemp = cTemp;
                  while (cTemp != 'Y' && cTemp != 'N')
                  {
                     cout << "* ERROR: Invalid answer! Please select only yes/no!";
                     macro_ask_question();
                     macro_get_line();
                     cTemp = toupper(answer[0]);
                     sTemp = cTemp;                   
                  }
                  cout << "Please wait....." << endl;                  
                  autocompile = (cTemp == 'Y');
                  exec_string  = "detectloader 2>&1";   
                  Pipe = popen(exec_string.c_str(), "r");
                  if (Pipe)
                  {
                     read_buffer = fgets(the_buffer, 1000, Pipe);
                     read_buffer_string = read_buffer;
                     if (read_buffer_string.find("LILO") != -1)
                     {
                        bootload = "lilo";
                     } else
                     if (read_buffer_string.find("SILO") != -1)
                     {
                        bootload = "silo";
                     }
                     else if (read_buffer_string.find("GRUB") != -1)
                     {
                        bootload = "grub";
                     }
                     else cout << "Cannot find the appropriate boot loader" << endl;
                     pclose(Pipe);
                  } else
                  {
                     cout << "Unexpected error while detecting boot loader...aborting" << endl;
                     exit(-1);
                  }
                  cout << "* Your boot loader is: " << bootload << endl;
                  config.set_config(config_options[CUSTOM_KERNEL_RECOMPILE], sTemp.c_str());
                  config.set_config("BOOT_LOADER", bootload.c_str());
                  break;            
            case EMAIL_REPORT:
               sTemp = answer;
               if (sTemp.length()==0) sTemp = const_administrator_email;
               while ( sTemp.find("@")==-1)
               {
                  cout << "* ERROR: E-mail address '"
                     << sTemp
                     << "' is invalid."
                     << endl << "Please specify a valid e-mail address! [e.g: user@host.net]";
                  macro_ask_question();
                  macro_get_line();
                  sTemp = answer;
                  if (sTemp.length()==0) sTemp = const_administrator_email;
               }

               cout << "All report will be sent to " << sTemp << endl;
               config.set_config(config_options[EMAIL_REPORT],sTemp.c_str());
               break;               
               
         } // end switch
      } // end for loop
   } // try block
   catch (string the_error)
   {
      cout << endl << "Aborting setup wizard :-(" << endl << "Reason: " << the_error << endl;
      config.terminate();
      unlink(configuration_file);
      delete[] answer;
      exit(-1);
   } // catch block
   delete[] answer;
}

void pause_keyboard()
// For compatibility when running under X terminal under gnome/kde
{
   char c;
   
   cout << endl << "Congratulation! Your configuration has been saved in "
      << configuration_file << ". You can replicate this file and put it"
      << "to other Mandrake workstations/servers across your network!"
      << endl; 

   cout << endl << "Now hit *Enter* to continue for more messages..." << endl;
   
   cin.get(c);
      
   cout << endl << "If you want to test how this program works, type \"drakupdaterobot\" from the shell prompt"
   << endl << "(you have to be logged in as the root user first. If you want to run this program as a daemon,"
   << endl << "copy the file /etc/drakupdaterobot.cron to /etc/cron.daily. If you don't know how to copy it,"
   << endl << "just type: \"cp /etc/drakupdaterobot.cron /etc/cron.weekly\""
   << endl << endl << "You can now replicate the file /etc/drakupdaterobot.conf "
   << endl << "to all your servers and workstations!" << endl;

   cout << endl << "Hit *ENTER* for the final message ..." << endl;
   
   cin.get(c);
   
   cout << endl << endl << "Have fun and now you can hit *ENTER* to close this setup wizard "
        << endl << "and try running 'drakupdaterobot'" << endl;
   
   cin.get(c);
}

int main(int argc, char **argv)
// Main
{
   display_logo();
   ask_questions();
   say_goodbye();
   chmod(configuration_file,S_IRUSR | S_IWUSR);
   pause_keyboard();
 }
