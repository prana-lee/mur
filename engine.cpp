// Backend - engine.cpp
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

#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <cctype>
#include "util.h"
#include "engine.h"
using namespace std;

//#define HARD_DEBUG

drakupdate::drakupdate(string &the_mirror_list,
 string &the_update_cache_dir, string &the_mirror_list_saved, string &the_temp_updatelist_file,
 string &the_favorite_ftp, string &the_proxy, string &the_proxy_login, string &the_email_to,
 bool check_gnupg_sig, bool use_tunnel, bool recompile, string &bloader, bool delete_cache)
// constructor
{
   time_t *current_time;
   tm *time_now;
   char *filename;
   DIR *directory_for_update;
   
   // from configuration file
   rpmdep = NULL;
   use_http_proxy_tunnel = use_tunnel;
   check_gnupg_signature = check_gnupg_sig;
   mirror_list_url = the_mirror_list;
   update_cache_dir = the_update_cache_dir;
   mirror_list_saved = the_mirror_list_saved;
   temp_updatelist_file = the_temp_updatelist_file;
   email_log_to = the_email_to;
   favorite_ftp = the_favorite_ftp;
   proxy_hostname = the_proxy;
   proxy_login = the_proxy_login;
   kernel_recompile = recompile;
   bootloader = bloader;
   delete_rpm_cache = delete_cache;
   rpmdatabase = new rpmdb;
   
   current_time = new time_t;
   time(current_time);
   time_now = localtime(current_time);
   
   filename = new char[11+1];
   snprintf(filename,11, "%d-%'02d-%'02d",1900+time_now->tm_year,1+time_now->tm_mon,time_now->tm_mday);
   current_date = filename;
   directory_for_update = opendir(logfile_directory);
   // if it can't be opened then create a new directory
   if (!directory_for_update)
      mkdir(logfile_directory, S_IREAD | S_IWRITE | S_IEXEC);
   closedir(directory_for_update);
      
   logfile_name = logfile_directory;
   logfile_name += "/";
   logfile_name += filename;
   logfile_name += ".txt";

   delete current_time;
   delete []filename;
      
   logfile.open(logfile_name.c_str());   
   if (!logfile)
   {
      cout << "Can't write to the log file " << logfile_name << endl;
      cout << "Please check:"
          << "A. Your configuration - run drakupdatesetup!" << endl
          << "B. The directory and file permission in " << logfile_directory << endl
          << endl;
      exit(-1);
   }
}

drakupdate::~drakupdate()
// destructor
{  
   delete rpmdatabase;
}

bool drakupdate::run()
// Run it
{
   bool return_value = false;
   // Initializes the process
   if (open_rpm_database()) // Stage 1
   {
      logfile << "* RPM database is successfully opened!" << endl   
           << "* Now locking the database, and no other program can use RPM database while MUR is running."
           << endl;
      // Stage 2: Get version. TODO: put Exception handling if error 
      cout << "Stage 2: Collecting Mandrake Linux version information" << endl;
      logfile << endl << "Stage 2: Collecting Mandrake Linux version information from this machine " << endl;
      getversion_and_machine(version, machine);
      logfile << "* You are running Linux Mandrake version "
         << version << " with update machine type: "
         << machine << endl;
      if (proxy_hostname.length() > 0)
      {
         logfile << "* You are using a proxy server/firewall: " << proxy_hostname << " ";
         if (use_http_proxy_tunnel)
         {
            logfile << " with HTTP proxy tunnel enabled";
         }
         logfile << endl << endl;
      }
      logfile << endl;
      // Stage 3: Read files in /var/cache/grpmi
      read_directory_var_cache_grpmi();      
      // Process
      // Stage 4: Download mirror list
      download_mirror();
      // Stage 5: Process the mirror
      process_mirrors_and_do_updates();
      // Stage 6: Download all selected RPMs
      download_all_selected_rpms();
      // Stage 7: Install all RPMs
      upgrade_all_selected_rpms();
      // Stage 8a: Close the RPM Database
      rpmdbClose(*rpmdatabase);
      // Stage 8b: Install kernel
      install_kernel();
      // Stage 9: Update menu and run ldconfig
      update_everything();

      // Stage 10: Report & e-mail     
      logfile << "Done, now report the root user" << endl;
      report_and_email_root_user_about_the_process();
      return_value = true;
   } else logfile << "RPM database is locked - aborting!" << endl;
   return return_value;
}

/****************************************************************************/
bool drakupdate::open_rpm_database()
// Open the RPM database
{
   bool return_value = false;
   
   cout << endl << "Stage 1: Opening the RPM Database" << endl;
   logfile << endl << "Stage 1: Opening the RPM (Redhat Package Manager) Database" << endl;
   // Init RPM
   rpmReadConfigFiles(NULL, NULL);
   if( rpmdbOpen("", rpmdatabase, O_RDWR | O_CREAT, 0644 ) == 0 )
   {
   	return_value = true;
   }
   return return_value;
}

bool drakupdate::read_directory_var_cache_grpmi()
// Read /var/cache/grpmi (the downloaded files)
{
   bool return_value = false;
   DIR *var_cache_grpmi;
   struct dirent *downloaded_files;
   
   cout << "Stage 3: Reading downloaded RPMS files" << endl;
   if ((var_cache_grpmi = opendir (update_cache_dir.c_str())) != NULL)
   {
      logfile << "Stage 3: Reading downloaded files in directory " << update_cache_dir << "..." << endl;	
      while((downloaded_files = readdir(var_cache_grpmi)) != NULL)
      {
         if (strncmp(downloaded_files->d_name,".",1)  != 0 &&
             strncmp(downloaded_files->d_name,"..",2) != 0)
         // Reject directory entry "." and ".."
         {
            downloaded_rpms.push_back (string(downloaded_files->d_name));
            logfile << downloaded_files->d_name << endl;
         } // end if strncmp
      } // end while
      closedir(var_cache_grpmi);
      return_value = true;
   } // end if var_cache_grpmi = ...
   logfile << endl << "Number of files in /var/cache/grpmi = " << downloaded_rpms.size() << endl << endl;
   return return_value;
}

/****************************************************************************/

bool drakupdate::download_mirror()
// Download the mirror and put it there to *mirrors
{
   bool return_value = false;
   int colon_location;
   ifstream saved_mirror(mirror_list_saved.c_str());
   string ftp_site;

   logfile << "Stage 4: Downloading the mirror" << endl;
   // Put the favorite FTP ...
   if (favorite_ftp.length() > 0)
      mirrors_vector.push_back(string(favorite_ftp));
   if (download_file(mirror_list_url.c_str(), mirror_list_saved.c_str(), is_a_file,
                     proxy_hostname.c_str(), proxy_login.c_str(), use_http_proxy_tunnel))
   // Can the file be downloaded?
   {
      cout << "Stage 4: Mirror list has been successfully downloaded." << endl;
      // Then open it..
      saved_mirror >> ftp_site;
      while (!saved_mirror.eof())
      {
      // Process
      colon_location = ftp_site.find(":");
      if (ftp_site.substr(0,colon_location) == machine)
      // Is it the right update?      
         {     
#ifdef HARD_DEBUG
      	     logfile << ftp_site.substr(0,colon_location)
             	<< ftp_site.substr(colon_location+1, ftp_site.length())
                << endl;
#endif
             if (ftp_site.find("http://")==-1)
             	mirrors_vector.push_back(string( ftp_site.substr(colon_location+1, ftp_site.length())) );
             else
                logfile << "Skipping " << ftp_site << endl;
            }  // end if it's the right description of our machine (e.g: updatesi586 for i586)         
            saved_mirror >> ftp_site; // Read again from the saved mirror list
         } // end while reading file
        logfile << endl;   
        saved_mirror.close();
      	return_value = true;
      } else cout << "Unable to download mirror list... maybe there is a problem... " << endl;
   return return_value;
}

inline void get_rpm_package_name_only(string &rpm_package_name)
{
   int last_letter = rpm_package_name.length()-1;
   for (int i =0; i < 2; i++)
   {
      last_letter = rpm_package_name.rfind('-',last_letter);
      rpm_package_name = rpm_package_name.substr(0,last_letter);
   }
}

inline void remove_the_i586_dot_rpm_thingy(string &rpm_package_name)
{
   int last_letter = rpm_package_name.length()-1;
   
   for (int i =0; i < 2; i++)
   {
      last_letter = rpm_package_name.rfind('.',last_letter);
      rpm_package_name = rpm_package_name.substr(0,last_letter);
   }
}

/****************************************************************************/
bool drakupdate::is_rpm_package_installed (string &rpm_package, string &version,
                                           string &release, string &architecture)
// Is the package with this name already installed?
// Returns version & release by pointer
{
   bool return_value = false;
#ifndef RPMDBI_PACKAGES   
   dbiIndexSet matches;
#else
   rpmdbMatchIterator iterator;
#endif
   
   Header rpm_header;
   struct headerTagTableEntry *tags;
   char *pkg_name, *pkg_version, *pkg_release, *pkg_architecture;
   string rpm_package_name = rpm_package;
   
//   cout << "Is " << rpm_package << " installed? " << endl;
   get_rpm_package_name_only (rpm_package_name);
   // Process it here
#ifndef RPMDBI_PACKAGES   
   if (rpmdbFindPackage(*rpmdatabase,rpm_package_name.c_str(), &matches) == 0)
   { 
      if (matches.count > 0)
      {
        rpm_header = rpmdbGetRecord(*rpmdatabase, matches.recs[0].recOffset);
#else // if RPM_VERSION_4
   iterator = rpmdbInitIterator(*rpmdatabase, RPMTAG_NAME, rpm_package_name.c_str(), 0);
   if (iterator)
   {
      if (iterator)
      {
         rpm_header = rpmdbNextIterator(iterator);
#endif
        if (rpm_header)
        {
           headerGetEntry(rpm_header, RPMTAG_NAME, NULL, (void **) &pkg_name, NULL);
           headerGetEntry(rpm_header, RPMTAG_VERSION, NULL, (void **) &pkg_version, NULL);
           headerGetEntry(rpm_header, RPMTAG_RELEASE, NULL, (void **) &pkg_release, NULL);
           headerGetEntry(rpm_header, RPMTAG_ARCH, NULL, (void **) &pkg_architecture, NULL);
           version = pkg_version;
           release = pkg_release;
           architecture = pkg_architecture;
   	   return_value = true;
           // Prana - March 6, 2002
           // The following headerFree(rpm_header) is removed because
           // it has been causing a segmentation fault on Mandrake 8.1
           //headerFree(rpm_header);
        } // end (if rpm_header)
#ifndef RPMDBI_PACKAGES
   	dbiFreeIndexRecord(matches);
#else // if RPM_VERSION_4
   	rpmdbFreeIterator(iterator);
#endif        
      } // end if (matches.count > 0)
    }    // end if (rpmdbFindPackage == 0) -- RPM4: end if iterator
   // End RPM
#ifdef HARD_DEBUG
   logfile << "is_rpm_package_installed (" << rpm_package << ")";   
   if (return_value)
      logfile << "=true" << endl;
   else
      logfile << "=false" << endl;
#endif
   return return_value;
}

/****************************************************************************/
bool drakupdate::is_rpm_package_already_uptodate (string &rpm_package, string &versions, string &release, string & arch)
// Is it already updated? If yes, don't download!
{
   bool installed;
   string rpm_name;
   string installed_version;
   string installed_release;
   string installed_arch;
   
   rpm_name = rpm_package;
   installed = is_rpm_package_installed(rpm_name, installed_version, installed_release, installed_arch);
   get_rpm_package_name_only(rpm_name);
   rpm_name +=  "-" + installed_version + "-" + installed_release + "." + installed_arch + ".rpm";
   return VersionCompare(rpm_name.c_str(), rpm_package.c_str()) >= 0;
}

/****************************************************************************/
bool drakupdate::is_it_already_downloaded(string &rpm_package)
// Jan 30, 2001 - problem is here - segmentation fault @ rpmReadPackageHeader
// Check if the rpm_package has already been downloaded
// If it hasn't, return false, else
// If there's a file, check if it is corrupt
// If it corrupts, delete it right away and replace with a new one
// Then return false
{
   // Copycat from MandrakeUpdate GUI version
   FD_t fd;   
   Header the_header; // RPM package header
   int isSource, major;
   int last_letter = 0;
   int already_downloaded = 0;
   bool return_value = false;
   bool is_it_installed;
   string complete_path_name = "/" + rpm_package;
   string currently_installed_rpm, downloaded_not_installed_rpm;
   string the_rpm_version, the_rpm_release, the_rpm_architecture;
   complete_path_name = update_cache_dir + complete_path_name;
string strA, strB;
#ifdef HARD_DEBUG
   logfile << "is_it_already_downloaded(\"" << rpm_package << "\")";
#endif
   already_downloaded = find_in_vector_exact(downloaded_rpms,rpm_package);
   if (already_downloaded != -1)
   {
      // Check if the downloaded file is corrupt
      // If it corrupts, delete it right away and replace with a new one and return false   
      // Copycat from MandrakeUpdate GUI version
      fd = fdOpen(complete_path_name.c_str(), O_RDONLY, 0);
      if (fdFileno(fd) > 0)
      {
        switch (rpmReadPackageHeader(fd, &the_header, &isSource, &major, NULL))
        {
          case 0:
             // add it here..
             currently_installed_rpm = rpm_package;
             downloaded_not_installed_rpm = rpm_package;
             remove_the_i586_dot_rpm_thingy(downloaded_not_installed_rpm);
             
             if (is_rpm_package_installed (currently_installed_rpm, 
                	the_rpm_version, the_rpm_release, the_rpm_architecture) == true)
             {
   		get_rpm_package_name_only(currently_installed_rpm);
                currently_installed_rpm +=  "-" + the_rpm_version + "-" + 
                                            the_rpm_release + "." +the_rpm_architecture + ".rpm";
                
                if (VersionCompare(downloaded_not_installed_rpm.c_str(), currently_installed_rpm.c_str()) > 0)
                // if it's already downloaded but not yet installed
                {
                   downloaded_not_installed_rpm += "." +the_rpm_architecture + ".rpm";
                   logfile << "Replace "
                        << currently_installed_rpm
                        << " immediately with the new downloaded "
                        << downloaded_not_installed_rpm
                        << endl;
                   tobe_installed_rpms.push_back(downloaded_not_installed_rpm);
                }
             }
             return_value = true;
             break;
          case 1:
             logfile << "Package is corrupted" << endl;
             // skip on to default: without break
          default:
             logfile << "Package validity can't be verified - deleting "
                  << complete_path_name.c_str() << endl;
             unlink(complete_path_name.c_str());
             // Now delete the file
             break;
        } // end switch
      } // end if
      fdClose(fd);
   }
#ifdef HARD_DEBUG
   if (return_value)
      logfile << "=true" << endl;
   else
      logfile << "=false" << endl;
#endif
   
   return return_value;
}

bool drakupdate::is_there_an_existing_older_rpm(string &rpm_package)
// Is there an older RPM package in /var/cache/grpmi ??
// *ALWAYS* returns true for the sake of && ^-^
// eg: if there's a new gnome-telnet-2.4-1mdk.i586.rpm,
// delete the already downloaded gnome-telnet-2.3-1mdk.i586.rpm
// in directory /var/cache/grpmi
{

   int older_rpm;
   older_rpm = find_in_vector_name_only(downloaded_rpms,rpm_package);
   string tobe_deleted;
   
   
//   logfile << "is_there_an_existing_older_rpm(" << rpm_package << ")" << endl;
   if ((older_rpm != -1) && VersionCompare(rpm_package.c_str(), downloaded_rpms [older_rpm].c_str()) > 0)
   // If found
   {
       tobe_deleted = update_cache_dir + "/" + downloaded_rpms [older_rpm];
       logfile << "Old package: " << tobe_deleted
           << " will be deleted - replace with: "
           << rpm_package << endl;
       unlink(tobe_deleted.c_str());
   }
   return true; // always true
}

bool drakupdate::fix_stupid_http_proxy_bug(string &the_line)
// Convert <A HREF="...."> to normal name .. bad boy Squid proxy
{
   bool return_value = false;
   int find_rpm_slash;
   int find_href;
   int end_tag;
   int temp_enter;
   string temp;

   end_tag = the_line.find(".rpm");
   if (proxy_hostname.length() <= 0)
   {
      temp_enter = the_line.find("\r");
      if (temp_enter != -1)
         the_line = the_line.substr(0, temp_enter);
      return_value = true;
   }
   else
   if (end_tag != -1)
   {
      find_href = the_line.find("HREF=");   
      find_rpm_slash = the_line.find("/RPMS/");      
      // Update it again -- workaround with some weird Squid proxy server config
      if (find_rpm_slash == -1)
         find_rpm_slash = find_href;
      if (find_href != -1 && find_rpm_slash != -1 && end_tag != -1)
      {
         temp = the_line.substr(find_rpm_slash + 6, end_tag-(find_rpm_slash+2));
         the_line = temp;         
         return_value = true;
      } // end if there's HREF...
   }
   return return_value;
}

/****************************************************************************/
bool drakupdate::do_the_actual_updates(string &path)
// Perform the updates here ...
{
   string rpm_package;
   string rpm_package_version;
   string rpm_package_release;
   string rpm_package_arch;
   string new_path;
   string quick_hack_rpm;
   ifstream update_listing;
   bool valid_http_tag;
   char *read_buffer;
   
//   read_buffer = new char[512+1];
      
   cout << "       : Trying to upgrade RPM package" << endl;
   logfile << "Upgrading RPM packages from mirror:" << path << endl;
   update_listing.open(temp_updatelist_file.c_str());
/*   update_listing.getline(read_buffer,512);
   rpm_package = read_buffer;*/
   
   update_listing >> rpm_package;
   while (!update_listing.eof())
   {
   
      valid_http_tag = fix_stupid_http_proxy_bug(rpm_package);
      quick_hack_rpm = rpm_package;
   
      // Pass 1: Fix for proxy bug
      if (valid_http_tag && rpm_package.find(".rpm") != -1 &&
      // Pass 2: Is the package already installed?
      // This function will return version and release to rpm_package_version & rpm_package_release
         is_rpm_package_installed(rpm_package, rpm_package_version, rpm_package_release, rpm_package_arch))
      {	 
      cout << "       : Pass 3" << endl;
      // Pass 3: Is it already up to date? If yes, skip ... kinda inefficient.. cleanup later
   if (!is_rpm_package_already_uptodate(quick_hack_rpm, rpm_package_version, rpm_package_release, rpm_package_arch) &&
      // Pass 4: Has this file been downloaded to /var/cache/grpmi?
      //      4a: If yes, is the downloaded file corrupt?
      //      4a-i : If corrupt, delete the file
      //      4a-ii: If not corrupt, okay go on
      //      4b: If not, then okay go on

         !is_it_already_downloaded(rpm_package) &&
         
      // Pass 5: Is there an existing old file?
      //         For example: gnome-telnet-2.3-1mdk.i586.rpm vs gnome-telnet-2.4-1mdk.i586.rpm
         is_there_an_existing_older_rpm(rpm_package) && 
         
         (find_in_vector_name_only(tobe_downloaded_rpms, rpm_package) == -1)
      )
      {
         cout << "       : Okay" << endl;
         new_path = rpm_package;
         tobe_downloaded_rpms.push_back(new_path);
#ifdef HARD_DEBUG
         logfile << "Update: " << path << rpm_package << endl;
#endif
      } /// end if Pass 2
     } // end if
      // read again from the file until eof
   update_listing >> rpm_package;      
   } // end while (!update_listing.eof())
   update_listing.close();
#ifdef DEBUG
   logfile << "Finish collecting information for file downloads " << endl;
#endif   
//   delete []read_buffer;
   return true;
}
/****************************************************************************/

bool drakupdate::visit_ftp_site(int retry, string &ftp_site)
{
   bool get_description_okay;
   bool return_value = false;
   string description_file;

   cout << "       : Visiting " << ftp_site << endl;
   logfile << "Visiting " << ftp_site << ", Retry count=" << retry << endl;
   current_mirror += ftp_site + "/" + version;
   current_mirror += "/descriptions";

   // Download the description file
   if (machine.find("cooker")!=-1)
   {
      cout << "Package descriptions will not be displayed since this is a BETA distribution - Mandrake Cooker" << endl;
   } else
   {
      
      if (download_file(current_mirror.c_str(), const_temporary_description_file, is_a_file,
         proxy_hostname.c_str(), proxy_login.c_str(), use_http_proxy_tunnel))
         {
            cout << "       : Package descriptions file have been successfully downloaded" << endl;
            load_update_description(const_temporary_description_file);
            unlink(const_temporary_description_file);
         }
      else logfile << "* Error: Unable to fetch the description of update! NOT a fatal error :-)" << endl;   
   }
   // List the directory ... 
   current_mirror = ftp_site + "/";
   if (machine.find("cooker")== -1) // if it's not Mandrake Cooker
      current_mirror = ftp_site + "/" + version + "/RPMS/";
   cout << "       : Listing directory: " << current_mirror << endl;
   logfile << "Listing directory: " << current_mirror << endl;
   
   if (download_file(current_mirror.c_str(),temp_updatelist_file.c_str(),
       is_a_directory, proxy_hostname.c_str(), proxy_login.c_str(),
       use_http_proxy_tunnel))
   {
      cout << "       : Directory listing OK!" << endl;
      return_value = do_the_actual_updates(current_mirror);
   } else
   {
      cout << "No updates available at the moment." << endl;
      // Distinguish them.
      // If we have a favorite ftp mirror, then just exit right away since
      // the assumption is that Favorite ftp is for those who have their own mirror
      if (favorite_ftp.length() > 0)
         exit(-1);
      else return_value = false;
   }
   
   return return_value;
}

/****************************************************************************/

bool drakupdate::process_mirrors_and_do_updates()
// Go through the list of mirror one by one, if it fails, it hops
// to the next mirror in the list
{
   bool return_value = false;
   int ftp_site;
   int random_ftp = 0;
   double factor = mirrors_vector.size();
   time_t *the_time = new time_t;

   time (the_time);
   srand(*the_time);
   cout << "Stage 5: Processing mirrors and trying to perform updates" << endl;
   for (ftp_site = 0; return_value==false && ftp_site < mirrors_vector.size(); ftp_site++)
   {
      if (favorite_ftp.length() == 0)
      {
         random_ftp = (int) (factor*rand()/(RAND_MAX+1.0));
      } else // if use a favorite ftp mirror
      {
         if (ftp_site > 0)
            random_ftp = 1 + (int) ((factor-1.0)*rand()/(RAND_MAX+1.0));
         else
            random_ftp = 0;
      }
      return_value = visit_ftp_site(ftp_site, mirrors_vector[random_ftp]);
   }
   delete(the_time);
   return return_value;
}

bool drakupdate::download_all_selected_rpms()
// Download the RPM packages to /var/cache/grpmi
{
   string download_file_place;
   string source_file;

   // Write log file...
   cout << "Stage 6: Downloading RPMS packages" << endl;
   logfile << endl << "Stage 6: Downloading "
      	   << tobe_downloaded_rpms.size()
           << " RPM package(s) from " << current_mirror << endl;
   
   for (int i = 0; i < tobe_downloaded_rpms.size(); i++)
   {
      logfile << "Downloading: " << current_mirror << (tobe_downloaded_rpms)[i] << endl;
      cout << "Downloading: " << current_mirror << (tobe_downloaded_rpms)[i] << endl;
// TODO: add code to automatically jump to another mirror if unsuccessful
      // Source URL
      source_file = current_mirror + (tobe_downloaded_rpms)[i];
      
      // Destination file: /var/cache/grpmi/filename.i586.rpm
      download_file_place = update_cache_dir + "/" + (tobe_downloaded_rpms)[i];
#ifdef HARD_DEBUG
      logfile << source_file.c_str() << " to " << download_file_place.c_str() << endl;
#endif
      if (download_file(source_file.c_str(), download_file_place.c_str(), is_a_file,
          proxy_hostname.c_str(), proxy_login.c_str(), use_http_proxy_tunnel))
      {
         // if it can be downloaded then move it to the todo-install list
         tobe_installed_rpms.push_back( (tobe_downloaded_rpms)[i] );
      }
      else logfile << "Failed downloading " << tobe_downloaded_rpms[i] << endl;
   } // end for loop
   return true; // well.. 
}


#ifdef RPMDBI_PACKAGES
void *update_install_progress(const void *h, const rpmCallbackType what,
                              const unsigned long amount, const unsigned long total,
                              const void *pkgKey, void *data)
#else
void *update_install_progress(const Header h, const rpmCallbackType what,
                              const unsigned long amount, const unsigned long total,
                              const void *pkgKey, void *data)
#endif
// Call function for drakupdate::install_one_rpm_package(string &rpm_package_name)
// CREDITS: Authors of RPM 4.0
{
      static FD_t fd;
      const char *filename = (const char *) pkgKey;

      switch (what)
      {
        case RPMCALLBACK_INST_OPEN_FILE:
	  fd = Fopen(filename, "r.ufdio");
          fd = fdLink(fd, "persist (showProgress)");
          return fd;
          
        case RPMCALLBACK_INST_CLOSE_FILE:
          fd = fdFree(fd, "persist (showProgress)");
          if (fd)
          {
	    Fclose(fd);
	    fd = NULL;
          }
          break;
        case RPMCALLBACK_TRANS_PROGRESS:
        case RPMCALLBACK_TRANS_START:
        case RPMCALLBACK_TRANS_STOP:
        case RPMCALLBACK_UNINST_PROGRESS:
        case RPMCALLBACK_UNINST_START:
        case RPMCALLBACK_UNINST_STOP:
        default:
        break;
      } // end switch
// Keyword: Should I really return NULL?? FIX UPDATE MAYBE?
   return NULL;
}

bool drakupdate::install_one_rpm_package(string &rpm_package_name)
// This is a recursive function to handle automatic dependencies
// some of the code were modified, but some of them are
// copycat from Mandrake's grpmi, which was copycatted from KPackage,
// but then I finally copycat from RPM 4.0, however since the code
// isn't suitable for some reasons, I changed quite a bit.
// CREDITS: Authors of RPM 4.0 too
{
   const int probFilter  = RPMPROB_FILTER_REPLACEPKG | 
      			   RPMPROB_FILTER_REPLACEOLDFILES |
                           RPMPROB_FILTER_REPLACENEWFILES |
                           RPMPROB_FILTER_OLDPACKAGE;

   rpmProblemSet problems = NULL;
   bool already_installed;
   bool upgrade_yes = true;
   bool return_value = false;
   bool okay_to_install = true;
   int trans_status;
   int isSource = 0;
   int major = 0;
   int i;
   int download_number;
   int package_info_number;
   Header the_header = NULL;
   FD_t fd;
   string install_this_file;
   string currently_installed_rpm, rpm_version, rpm_release, rpm_arch;
   int interfaceFlags = INSTALL_UPGRADE;
   int signature_check_okay = true;
   FILE *Pipe;
   char *read_buffer;
   char gpg_buf[1024]; // not my code!! 
   bool gpg_sig = false;
   string read_buffer_string;   
   string exec_string;
   string delete_path = update_cache_dir + "/";
     
   download_number = find_in_vector_exact(tobe_installed_rpms,rpm_package_name);
   if (download_number == -1)
   {
      // e.g: Zope-zpublisher instead of Zope-zpublisher-2.2.4-1.1mdk.i586.rpm
      download_number = find_in_vector_name_only(tobe_installed_rpms,rpm_package_name);
   }
   
   // Is it really there in /var/cache/grpmi
   if (download_number != -1)
   {
      install_this_file = update_cache_dir + "/" + (tobe_installed_rpms)[download_number];
   }
   else
   {
      logfile << "Can't find the downloaded RPM with the name " << rpm_package_name << endl;
      return true;
   } // end if & else download_number != -1
   // Check install pass 1
   already_installed = is_rpm_package_installed( tobe_installed_rpms[download_number], rpm_version,
   rpm_release, rpm_arch );
   // Keyword: ABUSE! Should clean up later ;)
   currently_installed_rpm = (tobe_installed_rpms)[download_number];
   get_rpm_package_name_only(currently_installed_rpm);
   // Feb 24, 2001
   package_info_number = find_package_update_description(currently_installed_rpm);

   currently_installed_rpm += "-" + rpm_version + "-" + rpm_release + "." + rpm_arch + ".rpm";
   
   // Check install pass 2
  already_installed = (VersionCompare(tobe_installed_rpms[download_number].c_str(), currently_installed_rpm.c_str()) < 0);

   // Print header for nice output
   logfile << endl;
   for (int j = 0; j < 70; j++)
      logfile << "-";
   logfile << endl;
   
   // Now check signature

//////////////////////////////////////////////////////////////////////   
   // Leftover from Mandrake Update
   exec_string  = "rpm --checksig " + install_this_file;
   exec_string += " 2>&1";
   
   Pipe = popen(exec_string.c_str(), "r");
   if (Pipe)
   {
      read_buffer = fgets(gpg_buf, 1000, Pipe);
      read_buffer_string = read_buffer;
      signature_check_okay = (read_buffer_string.find("md5 gpg OK") != -1);
      pclose(Pipe);
   }
//////////////////////////////////////////////////////////////////////
   // Now install
   // if check signature is required, but signature check result is bad, refuse to install
   if (check_gnupg_signature)
   {
      okay_to_install = signature_check_okay;
   }
   if (!already_installed && okay_to_install)
   {
      logfile << "Upgrading: " << currently_installed_rpm << " to " << (tobe_installed_rpms)[download_number];
      // Find package info number
      rpmdep = rpmtransCreateSet(*rpmdatabase, NULL);         
      fd = Fopen(install_this_file.c_str(), "r.ufdio");
      
      if (fdFileno(fd) > 0)
      {
      	switch (rpmReadPackageHeader(fd, &the_header, &isSource, &major, NULL))
         {
            case 1:
               logfile << "Downloaded file is corrupted" << endl;
               break;
            case 0:
             	trans_status = rpmtransAddPackage(rpmdep, the_header, NULL, install_this_file.c_str(), 1, NULL);
                switch(trans_status)
                { 
                   case 1: logfile << " error" << endl;break;
                   case 2: logfile << " error, newer RPM required" << endl;break;
                   default:                   
                      logfile << " okay " << endl << endl;
                      logfile << "* Checking digital signature: " << endl;
                      if (signature_check_okay)
                         logfile << "  - Linux Mandrake Security Team's GnuPG signature is found" << endl
                            	 << "  - MD5 checksum is valid" << endl;
                      else
                         logfile << "  - WARNING! SIGNATURE CAN'T BE VERIFIED" << endl
                            	 << "  - Either this package has been tampered by a malicious cracker or it is corrupted" << endl;
                      
                      if (package_info_number != - 1)
                      // If the description of vulnerability is found then
                      {      
                         logfile << endl << "* Description of the update:" << endl;
                         for (i = 1; i < 3; i++)
                            logfile << package_info[i][package_info_number];
                         logfile << endl << "* About this package: " << endl;
                         logfile << package_info[3][package_info_number];                            
                         logfile << endl;
                      }
                      break;                      
   		}
                return_value = true;
                break;
            default:
         	break;       
            } // end switch
	Fclose(fd);
      } // end if fdFileno(fd) > 0)
      
      // NOW RUN TRANSACTION
     
      trans_status = rpmRunTransactions(rpmdep, update_install_progress, NULL, NULL, &problems, (rpmtransFlags)0, (rpmprobFilterFlags)probFilter);
      if (trans_status < 0)
         logfile << endl << "RPM run transaction *NOT* okay" << endl;
      else if (trans_status > 0)
      {
         rpmProblemSetPrint(stderr, problems);
         //////////////////////////////////
         logfile << endl << "RPM run transaction..." << endl;  
         logfile << "Found: " << problems->numProblems << " problems" << endl << endl;
         for (i = 0; i < problems->numProblems; i++)
         {
            if (!problems->probs[i].ignoreProblem)
            {
                char *msg = (char *)rpmProblemString(&(problems->probs[i]));
               logfile << msg << endl;
               free(msg);
            } // end if there's problem
         } // end for
         if (problems) rpmProblemSetFree(problems);
         return_value = true;
         }
         // NO MEM Leak
         rpmtransFree(rpmdep);
  }
  else
  {
     logfile << "Refusing to install " << tobe_installed_rpms[download_number] << "!" << endl;
  }   
  
  if (delete_rpm_cache)
  {
     delete_path += (tobe_installed_rpms)[download_number];
     logfile << endl << "Deleting: " << delete_path << endl;
     unlink(delete_path.c_str());
  }
  return return_value;  
}

bool drakupdate::upgrade_all_selected_rpms()
// Install the downloaded RPMS
{
   logfile << endl <<"Stage 7: Upgrading " << tobe_installed_rpms.size() << " RPM package(s)" << endl;

   sort(tobe_installed_rpms.begin(), tobe_installed_rpms.end());   
   for (int i = 0; i < tobe_installed_rpms.size(); i++)
   {
      install_one_rpm_package(tobe_installed_rpms[i]);
   }
   return true;
}


void drakupdate::update_everything()
// Update menu entries /usr/lib/menu and symlinks ldconfig
{
   logfile << endl << "Stage 9: Updating Mandrake's global menu, and library symlinks" << endl;
   system("ldconfig");
   system("update-menus");
}

bool drakupdate::report_and_email_root_user_about_the_process()
// E-mail the root user
{
   string command;

   logfile << endl << "Stage 10: Report it to " << email_log_to << endl;
   
   logfile << endl << endl
      << "--" << endl
      << "Mandrake Update Robot " << robot_version << endl
      << "Copyright (C) 2001 Prana" << endl
      << "----------------------------" << endl << endl
      << "Developed by:" << endl
      << "* Prana <*****@*****.***> - main module" << endl
      << "* Pixel <pixel@mandrakesoft.com> - patches" << endl
      << "* Geoffrey Lee <snailtalk@linux-mandrake.com> - Mandrake 8.0 compatiblity" << endl
      << "* Kim Schulz - Beta tester (firewall, Squid proxy server)" << endl
      << "* Charles Nepote - Beta tester (firewall, Squid proxy server)" << endl
      << endl
      << endl << "Special thanks to Daniel Stenberg for his curl-lib <http://curl.haxx.se>" << endl
      << endl;
   logfile.close();
   
   // Execute the command "mail"
   command = "mail -s \"Mandrake Update Report " + current_date;
   command += "\" " + email_log_to;
   command += " < " + logfile_name;
   system(command.c_str());
   return true;
}

void drakupdate::load_update_description(const char *info_filename)
// Load package description
{
  const int READ_BUFFER = 512;
  char *line;
  int i = 0;
  int package_number = 0;
  int field_num = 0;  
  string read_line, previous_line;
  ifstream description_file;
  string current_package, previous_package;
  
  // init - open file and new mem
  // NO 1-byte buffer ov'flow because... just because
  line = new char[READ_BUFFER+1];
  description_file.open(info_filename);
  // process
  description_file.getline(line, READ_BUFFER);
  read_line = line;
  
  while (!description_file.eof())
  {
     if (read_line.find("%")==0)
     {
        if (previous_line != "")
        {
           field_num++;
           package_info[field_num].push_back(previous_line);
        }
        if (read_line.find("%package")==0)
        {
           field_num = 0;
   	   package_info[field_num].push_back(read_line.substr(9,read_line.length()));
           previous_line = "";
        }
        previous_line = "";
        description_file.getline(line, READ_BUFFER);
        read_line = line;
     }
     // Read again
     previous_line += "\n" + read_line;
     description_file.getline(line, READ_BUFFER);
     read_line = line;
  }
  description_file.close();
  if (previous_line != "")
  {
     field_num++;
     package_info[field_num].push_back(previous_line);
  }
  // no memory leak
  delete []line;
}

int drakupdate::find_package_update_description(string &package_name)
// Find packages information
{
  int lastfound = -1;
  int k;
  
  for (k = 0; k < package_info[0].size(); k++)
  {
     if ( package_info[0][k].find (package_name)  != -1)
        lastfound = k;
  }
#ifdef HARD_DEBUG
  logfile << "find_package_update_description(" << *package_name << ") = " << k << endl;
#endif
  return lastfound;
}

bool drakupdate::install_kernel()
{
// Stage 8: Checking if install kernel is required...

   bool require_kernel_recompile;
   string kernel_source_package = "kernel-source";
   
   ifstream kernel_config(kernel_config_file);
   logfile << endl << "Stage 8: Checking if there is a custom kernel that needs to be recompiled..." << endl;
   require_kernel_recompile = (find_in_vector_name_only(tobe_installed_rpms,kernel_source_package) != -1);
   if (kernel_config)
   {
   if (require_kernel_recompile)
   {
      if (kernel_config_file) // if kernel config /usr/src/linux/.config
         {
            system(kernel_recompile_command);
         }
      } // endif kernelconfigrecompile
      else logfile << "Currently, there is no kernel upgrade required..." << endl;
   }
   else logfile << "* MUR ignores automatic kernel upgrade since you don't have a custom configured Linux kernel" << endl;
   return true;
}

string &drakupdate::get_logfile_name()
// Return the log file name
{
   return logfile_name;
}

