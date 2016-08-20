// Utilities - util.cpp
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

#include <stl.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <fstream>
#include <regex.h>
#include <cstdlib>
#include <fcntl.h>

#include <rpm/rpmlib.h>
#include <rpm/rpmio.h>
#include <rpm/misc.h>
#ifndef RPMDBI_PACKAGES
#include <rpm/dbindex.h>
#endif

#include "util.h"

using namespace std;

/*************************************************************************
*  VersionCompare - modified from Mandrake Update
*  March 25, 2001 - added modification for workaround on Redhat's rpmvercmp bug
*                   and Mandrake's packaging bug
**************************************************************************/
int VersionCompare(const char *rpm_new_version, const char *rpm_old_version)
{
  int x, y;
  int return_value;
  // version and release
  string new_ver, old_ver, new_rel, old_rel;
  string NewVersion = rpm_new_version;
  string OldVersion = rpm_old_version;
  
  x = NewVersion.rfind('-');
  y = OldVersion.rfind('-');
  
  new_ver = NewVersion.substr(0,x);
  new_rel = NewVersion.substr(x+1,NewVersion.length());

  old_ver = OldVersion.substr(0,y);
  old_rel = OldVersion.substr(y+1,OldVersion.length());
  
  return_value = rpmvercmp(new_ver.c_str(), old_ver.c_str());
    
  if (return_value == 0)
  {
     return_value = rpmvercmp(new_rel.c_str(), old_rel.c_str());
     if (return_value == -1)
     {
        if ( (NewVersion.find(".1mdk") != -1) && (OldVersion.find(".1mdk")== -1) )
           return_value = 1;
     }
  }
//  cout << "Comparing " << rpm_new_version << " vs" << rpm_old_version << " = " << return_value << endl;  
  return return_value;
}

/****************************************************************************/
bool getversion_and_machine(string &version, string &machine)
// Get version from /etc/mandrake-release
{
   const int SIZE_LIMIT = 512;
   ifstream mandrake_release;
   bool machine_found = false;
   bool return_value = false;
   bool cooker = false;   
   int position = 0;
   int temp = 0;
   char *buffer = new char [SIZE_LIMIT+1];
   // March 21, 2001 - workaround with Beta 8.0
   string tempstr;
   // Read string s from file
   mandrake_release.open(version_file);
   if (mandrake_release)
   {
      for (position = 0; position < 4; position++)
      {
         mandrake_release >> version;
         if (version.find("Cooker") != -1)
         {
            cooker = true;
            temp = version.find("-");
            machine = version.substr(temp+1, version.length());
            mandrake_release >> version;
            break;
         }
      } // end for
      if (!cooker)
      {
         for (position = 0; position < 3; position++)
            mandrake_release >> machine;
      } // end if NOT cooker
      
      // END READING FILE
      mandrake_release.close();
      // March 21, 2001 - Work around with 8.0 bugs ...
      mandrake_release.open(version_file);
      mandrake_release.getline(buffer, SIZE_LIMIT);
      tempstr = buffer;
      mandrake_release.close();

      if (tempstr.find("Cooker") != -1)
      {
         cooker = true; // Can't assign value directly with cooker = mandrake_release.find...
         version = "BETA";
      }
      if (tempstr.find ("i586") != -1) machine = "i586";
      else if (tempstr.find ("alpha") != -1) machine = "alpha";
      // why do you put sparc into cookerparc? That's weird.
      if (machine == "sparc" || (tempstr.find("sparc") != -1))
         machine = "parc";         
      // If it's cooker
      if (cooker)
         machine = "cooker" + machine;
      else
         machine = "updates" + machine;
      return_value = (machine.length() > 0) && (version.length() > 0);
   }
   delete []buffer;   
   return return_value;
}


/****************************************************************************/
int find_in_vector_exact(const vector<string> &the_vector, string &search_for)
{
   bool found = false;
   int return_value = -1;
   int i;
   
   for (i = 0; i < the_vector.size() && !found; i++)
   {
      found = (search_for == (the_vector)[i]);
   }
   if (found)
      return_value = i-1;
   return return_value;
}

/****************************************************************************/
int find_in_vector_name_only(const vector<string> &the_vector, string &search_for)
// e.g: gnome-telnet-2.4 and gnome-telnet-2.3 will return found=true because
// the package name excluding version is the same
{
   bool found = false;
   int return_value = -1;
   int i;
   string tempstr1, tempstr2;
   
   tempstr1 = search_for.substr(0, get_package_name_position(search_for));
   for (i = 0; !found && i < the_vector.size(); i++)
   {
      tempstr2 = (the_vector)[i];
      tempstr2 = tempstr2.substr(0, get_package_name_position(tempstr2));
      found = (tempstr1 == tempstr2);
   }
   if (found)
      return_value = i-1;
   return return_value;
}

int get_package_name_position(const string &packagename)
// Just find the version starting from ...
// for example: "kernel-2.2.17" -> this function will
// return the string position of the first digit found in that text
// Keyword: FIX TODO REPAIR
{
   int i = 0;
   while (i < packagename.length() && !isdigit( packagename[i] ) )
   {
      i++;
   }
   if (i != packagename.length())
      i--;
   return i;
}

void print_curl_error(int error_code)
{
   const char *error_print_format[] = 
   {
     "Error Curl Library: UNSUPPORTED PROTOCOL",
     "Error Curl Library: FAILED INIT",
     "Error Curl Library: URL MALFORMAT",
     "Error Curl Library: URL MALFORMAT USER",
     "Error Curl Library: COULDNT RESOLVE PROXY",
     "Error Curl Library: COULDNT RESOLVE HOST",
     "Error Curl Library: COULDNT CONNECT",
     "Error Curl Library: FTP WEIRD SERVER REPLY",
     "Error Curl Library: FTP ACCESS DENIED",
     "Error Curl Library: FTP USER PASSWORD INCORRECT",
     "Error Curl Library: FTP WEIRD PASS REPLY",
     "Error Curl Library: FTP WEIRD USER REPLY",
     "Error Curl Library: FTP WEIRD PASV REPLY",
     "Error Curl Library: FTP WEIRD 227 FORMAT",
     "Error Curl Library: FTP CANT GET HOST",
     "Error Curl Library: FTP CANT RECONNECT",
     "Error Curl Library: FTP COULDNT SET BINARY",
     "Error Curl Library: PARTIAL FILE",
     "Error Curl Library: FTP COULDNT RETR FILE",
     "Error Curl Library: FTP WRITE ERROR",
     "Error Curl Library: FTP QUOTE ERROR",
     "Error Curl Library: HTTP NOT FOUND",
     "Error Curl Library: WRITE ERROR",

     "Error Curl Library: MALFORMAT USER", /* the user name is illegally specified */
     "Error Curl Library: FTP COULDNT STOR FILE", /* failed FTP upload */
     "Error Curl Library: READ ERROR", /* could open/read from file */

     "Error Curl Library: OUT OF MEMORY",
     "Error Curl Library: OPERATION TIMEOUTED", /* the timeout time was reached */
     "Error Curl Library: FTP COULDNT SET ASCII", /* TYPE A failed */

     "Error Curl Library: FTP PORT FAILED", /* FTP PORT operation failed */

     "Error Curl Library: FTP COULDNT USE REST", /* the REST command failed */
     "Error Curl Library: FTP COULDNT GET SIZE", /* the SIZE command failed */

     "Error Curl Library: HTTP RANGE ERROR", /* The RANGE "command" didn't seem to work */

     "Error Curl Library: HTTP POST ERROR",

     "Error Curl Library: SSL CONNECT ERROR", /* something was wrong when connecting with SSL */

     "Error Curl Library: FTP BAD DOWNLOAD RESUME", /* couldn't resume download */

     "Error Curl Library: FILE COULDNT READ FILE",

     "Error Curl Library: LDAP CANNOT BIND",
     "Error Curl Library: LDAP SEARCH FAILED",
     "Error Curl Library: LIBRARY NOT FOUND",
     "Error Curl Library: FUNCTION NOT FOUND",
  
     "Error Curl Library: ABORTED BY CALLBACK",
     "Error Curl Library: BAD FUNCTION ARGUMENT",
     "Error Curl Library: BAD CALLING ORDER",

     "Error Curl Library: HTTP PORT FAILED", /* HTTP Interface operation failed */

     "Error Curl Library: BAD PASSWORD ENTERED", /* when the my getpass() returns fail */
     "Error Curl Library: TOO MANY REDIRECTS", /* catch endless re-direct loops */      
     "Error CURL LAST - Unknown"
   };
   
   if (error_code > CURLE_OK && error_code < CURL_LAST)
   {
      cout << endl << error_print_format[error_code + 1] << endl;
   }
}
/****************************************************************************/
bool download_file(const char *url_file, const char *tofile, either_file_or_directory filetype,
const char *proxy, const char *proxy_usernamepassword, bool use_tunnel)
{
  CURL *curl;
  CURLcode res;
  FILE *contentfile;
  bool return_value = false;
  
  contentfile = fopen(tofile, "w");
  curl = curl_easy_init();
  if (curl)
  {
    /* what call to write: */
    if (strcmp(proxy,"") != 0)
    {
       curl_easy_setopt(curl, CURLOPT_PROXY,proxy);
       if (strcmp(proxy_usernamepassword,"") != 0)
       	  curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD,proxy_usernamepassword);
       if (use_tunnel)
          curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, TRUE);
    }
    curl_easy_setopt(curl, CURLOPT_URL, url_file);
    curl_easy_setopt(curl, CURLOPT_FILE, contentfile);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, TRUE);
    if (filetype == is_a_directory)
    {
       curl_easy_setopt(curl, CURLOPT_FTPLISTONLY, TRUE);
    }
    res = curl_easy_perform(curl);
    /* always cleanup */
    curl_easy_cleanup(curl);
    print_curl_error(res);
    return_value = (res==CURLE_OK);
  }
  fclose (contentfile);
  return return_value;
}

