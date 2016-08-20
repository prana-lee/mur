Mandrake Update Robot
------------------------------------------------------------------------------
NOTE: This tool is outdated and is no longer developed. Valid from 2000 - 2002, 
deprecated by urpmi. Uploaded to GitHub on August 20, 2016.

License: Freeware (GNU Public License v2) - read file "COPYING"
Author: Prana

CREDITS:
- Patches from Pixel <pixel@mandrakesoft.com>, Geoffrey Lee <snailtalk@linux-mandrake.com>
- Beta testers for corporate proxy/firewall: Kim Schulz <kim@schulz.dk>
- curl-lib (libcurl) by Daniel Stenberg <daniel@haxx.se> - good library!

Mandrake Update Robot is a text-based version of Mandrake Update.
It is very useful if you run a very large network. With this tool, you 
don't have to be worried about missing security updates. After running
the update tool, this robot will e-mail a report to the root user.

See the file "ChangeLog" for release details :-)

Installation:
1) Login as root user
2) type "make"
3) type "make install"
4) run "drakupdaterobot"

Setup:
Run the program "drakupdatesetup" as root user.

Requirements:
* Mandrake Linux 7.2 or higher
* T3/T1/Cable/DSL connection recommended (this drakupdaterobot is not for modem users)
* curl-devel (http://curl.sourceforge.net)
