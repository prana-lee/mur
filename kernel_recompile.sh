#!/bin/sh
###########################################################################
# This is the default kernel recompilation command from Mandrake Update
# Robot v1.1. Please modify when necessary, otherwise leave it intact
###########################################################################
cd /usr/src/linux
echo Mandrake Update Robot automatic kernel recompilation > /var/log/drakupdaterobot/kernelcompile_`date -I`.txt
date -I +%c >> /var/log/drakupdaterobot/kernelcompile_`date -I`.txt
make dep >> /var/log/drakupdaterobot/kernelcompile_`date -I`.txt && \
make bzImage >> /var/log/drakupdaterobot/kernelcompile_`date -I`.txt && \
make modules >> /var/log/drakupdaterobot/kernelcompile_`date -I`.txt && \
make modules_install >> /var/log/drakupdaterobot/kernelcompile_`date -I`.txt && \
make install >> /var/log/drakupdaterobot/kernelcompile_`date -I`.txt
