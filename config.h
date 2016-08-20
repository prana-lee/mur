// Configuration - config.h
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

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <stl.h>

#define configuration_file "/etc/drakupdaterobot.conf"
#define const_mirror_list_saved "/etc/urpmi/mirrorsfull.list"
#define const_temp_updatelist_file "/etc/urpmi/ftp_directory_listing_only"
#define const_administrator_email "root@localhost"
using namespace std;

enum question_type
{
   READINESS,
   CONNECTION_TYPE_ETHERNET,
   CLIENT_OR_SERVER,
   BEHIND_A_PROXY,
   PROXY_HOST_NAME,
   PROXY_PORT,
   PROXY_TUNNEL,
   PROXY_AUTHENTICATION_REQUIRED,
   PROXY_USERNAME,
   PROXY_PASSWORD,
   MIRROR_LIST_URL,
   FAVORITE_FTP_MIRROR,
   UPDATED_RPM_PACKAGE,
   MIRROR_LIST_FILE,
   DOWNLOADED_FILE_LIST,
   GNUPG_CHECK_SIGNATURE,
   CUSTOM_KERNEL_RECOMPILE,
   DELETE_RPM_CACHE,
   EMAIL_REPORT,
};


static char *config_options[] = 
{
   "READINESS",
   "CONNECTION_TYPE_ETHERNET",
   "CLIENT_OR_SERVER",
   "BEHIND_A_PROXY",
   "PROXY_HOST_NAME",
   "PROXY_PORT",
   "PROXY_TUNNEL",
   "PROXY_AUTHENTICATION_REQUIRED",
   "PROXY_USERNAME",
   "PROXY_PASSWORD",
   "MIRROR_LIST_URL",
   "FAVORITE_FTP_MIRROR",
   "UPDATED_RPM_PACKAGE",
   "MIRROR_LIST_FILE",
   "DOWNLOADED_FILE_LIST",
   "GNUPG_CHECK_SIGNATURE",
   "CUSTOM_KERNEL_RECOMPILE",
   "DELETE_RPM_CACHE",
   "EMAIL_REPORT"
};


class drakupdateconfig
{
   private:
      vector <string> key;
      vector <string> value;
      bool read_result;
      bool write_result;
      // not nice.. but oh well
      string return_config_value;   
      int get_config_number(const char *a_key);
      
   public:
      enum {DRAKCONFIG_READ, DRAKCONFIG_WRITE};
      drakupdateconfig();
      void terminate();
      ~drakupdateconfig();
      void reset();
      void set_config(string &a_key, string &a_value);
      void set_config(const char *a_value, const char *a_value);
      string &get_config(string &a_key);
      string &get_config(const char *a_key);
      void output_to_screen();
      bool get_result(int mode);
};
#endif

