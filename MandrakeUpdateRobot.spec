%define name MandrakeUpdateRobot
%define version 1.2
%define release 1mdk

Name:		%{name}
Summary:	Console-based Mandrake update tool for automatic upgrade daemon in a large corporate network
Version:	%{version}
Release:	%{release}
License:	GPL
Group:		System/Base
BuildRoot:	%{_tmppath}/%{name}-%{version}
BuildRequires:  libcurl2-devel >= 7.8
Requires:	libcurl2 >= 7.8, gnupg >= 1.0.3-2mdk

%description
Mandrake Update Robot is a must-have automatic upgrade tool daemon for
every system administrator that maintains security upgrade for Linux
Mandrake workstations and servers in a large network. MUR automatically
fetches RPM update packages from Mandrake Linux's trusted ftp mirrors
and installs the RPM packages, including the dependencies. MUR will
also save your time fetching the RPM packages manually with a graphical
configuration tool.

With this tool, noone have to be worried about missing frequent security
updates. To run it manually, type "drakupdatetxt", and to run the setup
wizard, type "drakupdatesetup". You can also click the icon from DrakConf
to run it.

After running the update tool, this robot will e-mail a report
to the root user (administrator). You can also run it as a daemon
by putting an entry to /etc/cron.daily to check for upgrade packages
everyday.
If you want to set it up as a daemon, don't forget to install the package
anacron

%prep
%setup -q

%build

### Note to Pixel - Uhmm.. I can't use this, exception handling must be enabled - try - catch
#OPTIMIZE="$RPM_OPT_FLAGS"
### Use sed to fix the OPTIMIZE flags
OPTIMIZE=$(echo $RPM_OPT_FLAGS | sed -e s/fno-exceptions/fexceptions/)
%make

%install
[ -n "%{buildroot}" -a "%{buildroot}" != / ] && rm -rf %{buildroot}

mkdir -p %{buildroot}%{_sbindir}
mkdir -p %{buildroot}%{_iconsdir}
mkdir -p %{buildroot}/etc/DrakConf
mkdir -p %{buildroot}/etc/urpmi
mkdir -p %{buildroot}/var/lib/urpmi
mkdir -p %{buildroot}/var/cache/grpmi
mkdir -p %{buildroot}/var/log/drakupdaterobot

install drakupdaterobot drakupdatesetup drakupdatesetup-DrakConf %{buildroot}%{_sbindir}
install drakupdatesetup.xpm %{buildroot}%{_iconsdir}/drakupdatesetup.xpm
install drakupdaterobot.cron %{buildroot}/etc/drakupdaterobot.cron
install drakupdatesetup.rc %{buildroot}/etc/DrakConf/drakupdatesetup.rc
install exports.sample %{buildroot}/etc/urpmi/exports.sample
install fstab.sample %{buildroot}/etc/urpmi/fstab.sample
install kernel_recompile.sh %{buildroot}/etc/urpmi/kernel_recompile.sh
install mirrorsfull.list.sample %{buildroot}/etc/urpmi/mirrorsfull.list.sample

%clean
[ -n "%{buildroot}" -a "%{buildroot}" != / ] && rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc COPYING README INSTALL AUTHORS
%{_sbindir}/*
%{_iconsdir}/drakupdatesetup.xpm
/etc/urpmi/fstab.sample
/etc/urpmi/mirrorsfull.list.sample
/etc/urpmi/exports.sample
/etc/drakupdaterobot.cron
/etc/DrakConf/drakupdatesetup.rc

%changelog
* Wed Mar 06 2002 Prana <*****@*****.***> 1.2-1mdk
- Change requirement from curl-lib to libcurl2
- Change requirement from curl-devel to libcurl2-devel
- Fix hanging at stage 4
- Fix segmentation fault due to headerFree function - stupid Redhat!
- This release is intended only for Mandrake 8.1 or higher

* Mon Oct 08 2001 Thierry Vignaud <tvignaud@mandrakesoft.com> 1.1-3mdk
- s!Linux Mandrake!Mandrake Linux!g

* Fri Jul 27 2001 Geoffrey Lee <snailtalk@mandrakesoft.com> 1.1-2mdk
- Finally got it to compile. (I got no idea whether it works or not.)

* Tue Apr 03 2001 Prana <*****@*****.***> 1.0-1mdk
- Fixed the slowness in downloading mirror

* Sun Mar 25 2001 Prana <*****@*****.***> 1.0-1mdk
- Final mature version, both for Mandrake 7.x and 8.x
- Fixed everything
- Custom kernel automatic recompilation

* Mon Mar 19 2001 Prana <*****@*****.***> 0.9-1mdk
- Stable version of Mandrake Update Robot v0.9
- Added custom FTP mirror option
- A much better and fool-proof setup configuration wizard in 15 steps

* Fri Mar 02 2001 Prana <*****@*****.***> 0.8-4mdk
- BETA 3 - very stable
- Yet another workaround with Squid proxy server
- Icon for DrakConf for Linux Mandrake 7.2 is fixed now

* Thu Mar 01 2001 Prana <*****@*****.***> 0.8-3mdk
- Workaround with Squid proxy server
- Fix bug in the http-tunnelling option

* Sun Feb 25 2001 Geoffrey Lee <snailtalk@mandrakesoft.com> 0.8-2mdk
- Put a fix so that it acutally compiles on Mandrake 7.2 and compiles
  on cooker if you remove -Werror. This is a known problem as up till
  line 591 in engine.cpp it does not return for a function of bool type.
- Use our own build flags, but enable exception handling.

* Sat Feb 24 2001 Pixel <pixel@mandrakesoft.com> 0.8-1mdk
- 0.8
- Merge Pixel's fix
- Try again with RPM 4 build

* Sun Feb 11 2001 Pixel <pixel@mandrakesoft.com> 0.7-2mdk
- cleanup, fix build

* Mon Feb 10 2001 Prana <*****@*****.***> 0.7-1mdk
- Stabilize everything.
- Includes setup wizard.

* Mon Feb 05 2001 Prana <*****@*****.***> 0.5-1mdk
- Initial release from day 4 of development.
