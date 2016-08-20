// Configuration - config.cpp
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

#include <fstream>
#include "config.h"

using namespace std;

void drakupdateconfig::set_config(string &a_key, string &a_value)
// set config
{
   set_config(a_key.c_str(), a_value.c_str());
}

void drakupdateconfig::set_config(const char *a_key, const char *a_value)
// set config
{
   string the_key, the_value;
   int existing_config;
   
   existing_config = get_config_number (a_key);
   the_key = a_key;
   the_value = a_value;
   if (existing_config == -1)
   {
      key.push_back(the_key);
      value.push_back(the_value);
   }
   else value[existing_config] = the_value;
}

int drakupdateconfig::get_config_number(const char *a_key)
// return the value of the key
{
   int config_number = -1;
   for (int i = 0; i < key.size(); i++)
      if (key[i] == a_key)
         config_number = i;
   return config_number;
}

string &drakupdateconfig::get_config(string &a_key)
// return the value of the key
{
   int config_number;
   return_config_value = "";
   
   config_number = get_config_number (a_key.c_str());
   if (config_number != -1)
      return_config_value = value[config_number];
   return return_config_value;
}

string &drakupdateconfig::get_config(const char *a_key)
// return the value of the key
{
   string the_key = a_key;
   return get_config(the_key);
}


drakupdateconfig::drakupdateconfig()
// constructor - read the config file
{	        
   ifstream inputfile(configuration_file);
   char line [512+1];
   int savepos;
   char c;
   string stringline, a_key, a_value;
   
   read_result = false;
   if (inputfile) // is the file readable ?
   {
      inputfile.getline(line, sizeof(line)-1); // ignore message
      inputfile.getline(line, sizeof(line)-1); // now start
      while (inputfile)
      {
            stringline = line;
            savepos = stringline.find("=");
            key.push_back(stringline.substr(0, savepos));
            value.push_back(stringline.substr(savepos+1, stringline.length() - savepos));
            inputfile.getline(line, sizeof(line)-1);
      }
      read_result = true;
   } else
   {
      cout << "*** N O T E *** "
           << "IF THIS IS THE 1st TIME YOU RUN THIS PROGRAM, PLEASE IGNORE THE FOLLOWING MESSAGE!"
           << endl;
      cout << endl << "* ERROR: Unable to read the configuration file " << configuration_file << endl
         << "o Possible reasons: " << endl
         << "\t-You didn't finish or aborted your previous setup wizard" << endl
         << "\t-You don't have the permission to read " << configuration_file << endl
         << "\t-You're running drakupdatesetup for the 1st time (ignore this message and hit *Enter*)"
         << endl
         << endl
         << "o Tips: If the file exists but you can't read it, you have to run this program as"
         << endl
         << "\tthe 'root' user by typing the command 'su' or log in with login name: root"
         << endl << endl
         << "Press *Enter* to continue or Ctrl-C to terminate the program..."
         << endl;
         cin.get(c);
         
   }
}

drakupdateconfig::~drakupdateconfig()
// terminate
{
   terminate();
}
void drakupdateconfig::terminate()
// write the config file
{
   ofstream outputfile(configuration_file);
   char c;
   
   write_result = false;
   outputfile << "#DO NOT ALTER THIS FILE AT ALL. DO NOT EDIT BY HAND UNLESS YOU REALLY KNOW HOW!" << endl;
   if (outputfile)
   {
      for (int i = 0; i < key.size(); i++)
      {
         outputfile << key[i] << "=" << value[i] << endl;
      }
      outputfile.close();
      write_result = true;
   }   else
   {      cout << endl << "* ERROR: Unable to write the configuration file " << configuration_file << endl
         << "- Reason: Either you don't have a write permission to that file or the disk is full"
         << endl
         << "- Tips: You have to be a root user to have a write permission to /etc directory."
         << endl
         << "\tRun this program with 'su' (superuser) or 'sudo' (execute as root) or login as root"
         << endl << endl
         << "Press Enter to continue or Ctrl-C to terminate the program..."
         << endl;
         cin.get(c);
   }   
}

void drakupdateconfig::output_to_screen()
// dump config to screen
{
   cout << "-- drakupdateconfig::output_to_screen() --" << endl;
   for (int i = 0; i < key.size(); i++)
      cout << key[i] << "=" << value[i] << endl;
}

bool drakupdateconfig::get_result(int mode)
{
   if (mode == DRAKCONFIG_READ)
      return read_result;
   else 
      return write_result;
}

void drakupdateconfig::reset()
{
   string nullstring;
   
   for (int i = 0; i < value.size(); i++)
      value[i] = nullstring;
}
