#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <stl.h>
#include <algorithm>

#define version_file "/etc/mandrake-release"

enum either_file_or_directory
{
   is_a_file,
   is_a_directory
};

int VersionCompare(const char *rpm_new_version, const char *rpm_old_version);
bool getversion_and_machine(string &version, string &machine);
int find_in_vector_exact(const vector<string> &the_vector, string &search_for);
int find_in_vector_name_only(const vector<string> &the_vector, string &search_for);
int get_package_name_position(const string &packagename);
bool download_file(const char *url_file, const char *tofile, either_file_or_directory filetype,
   const char *proxy, const char *proxy_usernamepassword, bool use_tunnel);
// From MandrakUpdates...
bool FindMatches(char *string, char *regex, char **results);

#endif
