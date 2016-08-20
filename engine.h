// Engine - engine.h
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

#ifndef ENGINE_H
#define ENGINE_H

#define robot_version "1.2"
#define logfile_directory "/var/log/drakupdaterobot"
#define const_update_cache_dir "/var/cache/grpmi"
#define const_mirror_list_url "http://www.linux-mandrake.com/mirrorsfull.list"
#define const_temporary_description_file "/var/log/drakupdaterobot/temporary_description"
#define kernel_recompile_command "/etc/urpmi/kernel_recompile.sh"
#define DEBUG
//#define HARD_DEBUG
#define FINAL_VERSION_CYEST

#include <string>
#include <fstream>
#include <stl.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmio.h>
#include <rpm/misc.h>

#ifndef RPMDBI_PACKAGES
   #include <rpm/dbindex.h>
#endif

#define kernel_config_file "/usr/src/linux/.config"
using namespace std;      

class drakupdate
{
   	typedef vector <string> packaging_update_info[4];
private:
        rpmdb *rpmdatabase;
	vector <string> installed_rpms; // installed rpms
	vector <string> downloaded_rpms; // in /var/cache/grpmi
	vector <string> tobe_downloaded_rpms; // needs to be downloaded
        vector <string> tobe_installed_rpms; // *MUST* be seperated from tobe_downloaded_rpms
        vector <string> mirrors_vector; // array of mirror list
        string version; // 7.1, 7.2, etc
        string machine; // cooker-i586, cooker-sparc, etc
        string current_mirror; // current mirror being used
        string current_date;
        rpmTransactionSet rpmdep;
        
        // from configuration
        bool check_gnupg_signature;
        bool use_http_proxy_tunnel;
        bool kernel_recompile;
        bool delete_rpm_cache;
        string bootloader;
        string mirror_list_url;
        string update_cache_dir;
        string mirror_list_saved;
        string temp_updatelist_file;
        string email_log_to;
        string proxy_hostname;
        string proxy_login;
        string logfile_name;
        string favorite_ftp;
        // the log file
        ofstream logfile;
        // update description
        packaging_update_info package_info;
        
public:
//	drakupdate(const char *the_mirror_list); // constructor
        drakupdate(string &the_mirror_list,
                  string &the_update_cache_dir, string &the_mirror_list_saved,
                  string &the_temp_updatelist_file, string &the_favorite_ftp,
                  string &the_proxy, string &the_proxy_login, string &the_email_to,
                  bool check_gnupg_sig, bool http_proxy_tunnel, bool recompile,
                  string &bloader, bool delete_cache);
	~drakupdate(); // destructor
	bool run();
        string &get_logfile_name();        
private:
	bool is_it_already_downloaded(string &rpm_package); // is it already stored in /var/cache/grpmi ?
	bool is_rpm_package_installed (string &rpm_package_name, string &rpm_pkg_version,
                                       string &rpm_pkg_release, string &rpm_pkg_architecture);
	bool is_rpm_package_already_uptodate (string &rpm_package_name, string &rpm_pkg_version,
                                       string &rpm_pkg_release, string &rpm_pkg_architecture);
	bool is_there_an_existing_older_rpm(string &rpm_package);
        
	bool download_mirror();
	bool do_the_actual_updates(string &path);
	bool visit_ftp_site(int retry, string &ftp_site);
	bool open_rpm_database();
	bool read_directory_var_cache_grpmi();
	bool process_mirrors_and_do_updates();
        // Jan 31, 2001
        bool download_all_selected_rpms();
        bool upgrade_all_selected_rpms();
        // Feb 4, 2001
        bool install_one_rpm_package(string &rpm_package_name);
        bool report_and_email_root_user_about_the_process();
        void update_everything();
        // Feb 22, 2001
        int  find_package_update_description(string &package_name);
        void load_update_description(const char *info_filename);

        // March 1, 2001
        bool fix_stupid_http_proxy_bug(string &the_line);
        // March 2, 2001
        bool install_kernel();
        // March 15, 2001
};

#endif
