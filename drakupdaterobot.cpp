// Main program - drakupdatetxt.cpp
// Note: not 100% working yet...
/*****************************************************************************
/*  Mandrake Update Text (daemon for crontab)                                *
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
#include "engine.h"
#include "config.h"
#include "util.h"
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <sys/times.h>
#include <time.h>
using namespace std;

/****************************************************************************/
#define mirror_list "http://www.linux-mandrake.com/mirrorsfull.list"

void display_logo()
{
  cout << "Mandrake Update Robot " << robot_version << endl
      << "Copyright (C) 2001 Prana" << endl
      << "License: Freeware (GNU Public License v2)" << endl
      << "-------------------------------------------------------" << endl;
}

bool readconfig(string &mirror_list_url, string &update_cache_dir, string &mirror_list_saved,
                string &temp_updatelist_file, string &favorite_ftp, string &proxy_hostname,
                string &proxy_login, string &email_log_to, bool &gnupg_sign_check,
                bool &use_tunnel, bool &recompile, string &bloader, bool &delete_cache)
                
{
   drakupdateconfig config;
   string temp;
   bool return_value = false;
   
   if (config.get_result(config.DRAKCONFIG_READ))
   {      
        proxy_hostname = config.get_config(config_options[PROXY_HOST_NAME]);
        if (proxy_hostname.length() > 0)
        {
           proxy_hostname += ":" + config.get_config(config_options[PROXY_PORT]);
           temp = config.get_config(config_options[PROXY_TUNNEL]);
           use_tunnel = (temp== "Y");
           if (config.get_config(config_options[PROXY_AUTHENTICATION_REQUIRED]) == "Y")
           {
              proxy_login = config.get_config(config_options[PROXY_USERNAME]);
              if (proxy_login.length() > 0)
           	  proxy_login += ":" + config.get_config(config_options[PROXY_PASSWORD]);
           } // end if proxy auth required
        } // end if it uses proxy
        mirror_list_url = config.get_config(config_options[MIRROR_LIST_URL]);
        favorite_ftp = config.get_config(config_options[FAVORITE_FTP_MIRROR]);
        update_cache_dir = config.get_config(config_options[UPDATED_RPM_PACKAGE]);
        mirror_list_saved = config.get_config(config_options[MIRROR_LIST_FILE]);
        temp_updatelist_file = config.get_config(config_options[DOWNLOADED_FILE_LIST]);
        gnupg_sign_check = config.get_config(config_options[GNUPG_CHECK_SIGNATURE])=="Y";
        delete_cache = config.get_config(config_options[DELETE_RPM_CACHE])=="Y";        
        email_log_to = config.get_config(config_options[EMAIL_REPORT]);
        if (config.get_config(config_options[GNUPG_CHECK_SIGNATURE]).length()==0)
        {
           cerr << endl << "* ERROR: the setup was not configured properly." << endl
           << "* HINT: Please run 'drakupdatesetup' again!" << endl;
           exit(-1);
        }
        recompile = config.get_config(config_options[CUSTOM_KERNEL_RECOMPILE])=="Y";
        bloader = config.get_config("BOOT_LOADER");
        return_value = true;
   } // end if config can be opened
   else cerr << "Can't access the configuration file: " << configuration_file << endl;
   return return_value;
}

int main(int argc, char *argv[])
//Main
{
   bool gnupg_check = false;
   bool use_tunnel = false;
   bool recompile = false;
   bool delete_cache = false;
   drakupdate *update_tool;
   int return_value = -1;
   
   string mirror_list_url;
   string update_cache_dir;
   string mirror_list_saved;
   string temp_updatelist_file;
   string email_log_to;
   string favorite_ftp;
   
   string proxy_hostname;
   string proxy_login;
   string arguments;
   string bloader;

   display_logo();
   cout << "Now running the update... please wait....." << endl
      << "* Tips: "
      << endl
      << "1) You can put an entry to /etc/cron.weekly or /etc/cron.daily to run it periodically" << endl
      << "   and automatically without user interaction."
      << endl << endl
      << "2) You can replicate the file " << configuration_file << " across all Mandrake servers in your LAN"
      << endl << endl
      << "3) There are several ways to share the updates to other Mandrake servers/workstations in your LAN."
      << endl
      << "   One way is by sharing the directory /var/cache/grpmi with NFS or Samba. That directory"
      << endl
      << "   is the default place for RPM cache unless you have changed it from drakupdatesetup"
      << endl      
      << "   sample configuration for NFS client is available in /etc/urpmi/fstab.sample - you"
      << endl      
      << "   can add the line from that file to /etc/fstab, and for the NFS server configuration"
      << endl      
      << "   file is located in /etc/urpmi/exports.sample, and you can add it to /etc/exports"
      << endl << endl
      << "4) You can create your own local mirror by using rsync or ftp mirror from other Mandrake"
      << endl
      << "   update mirrors for your primary update server in your LAN"
      << endl << endl
      << "5) MUR is backward compatible with MandrakeUpdate 7.1 and 7.2 and with the new"
      << endl
      << "   rpmdrake because it uses the same cache update directory /var/cache/grpmi"
      << endl
      << "   by default"
      << endl;
   if (argc > 1)
   {
      cout << "To setup this program, type 'drakupdatesetup' - you need to log in as root first" << endl
         << "To run this program, type 'drakupdaterobot' without any parameters" << endl << endl
      exit(0);
   }
   if (readconfig(mirror_list_url, update_cache_dir, mirror_list_saved, temp_updatelist_file,
              favorite_ftp, proxy_hostname, proxy_login, email_log_to, gnupg_check,
              use_tunnel, recompile, bloader, delete_cache))
   {      
      update_tool = new drakupdate(mirror_list_url, update_cache_dir, mirror_list_saved, temp_updatelist_file,
                 favorite_ftp, proxy_hostname, proxy_login, email_log_to, gnupg_check,
                 use_tunnel, recompile, bloader, delete_cache);
      cout << endl << "Initializing ... please wait...." << endl 
         << "*** NOTE ***" << endl
         << "No messages will be displayed here while the program is running (except during the download)"
         << endl
         << "Every messages goes directly to " << update_tool->get_logfile_name() << endl;
                 
      update_tool->run();
      delete update_tool;
      return_value = 0;
   }
   else
   {
      if (getuid==0)
      {
         cerr << endl << "Please run drakupdatesetup prior to running drakupdaterobot. Thanks." << endl;
      } else
      {
         cerr << endl << "Could it be because you don't have the permission to access the file " << configuration_file << "?" << endl;
      }
   }
   return return_value;
}
