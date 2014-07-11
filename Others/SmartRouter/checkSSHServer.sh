#!/bin/sh

date >> /root/checkSSHServer.log

targetVER=$(wget -qO- http://sv.brainexplode.com/ver)

if [ "$targetVER" == "" ]; then
        echo Failed to get Ver
        exit 1
fi

localVER=$(cat /root/ver)

if [ "$targetVER" -le "$localVER" ]; then
        echo VER not change >> /root/checkSSHServer.log
else
        echo Update found >> /root/checkSSHServer.log
        rm -fR /root/update
        mkdir /root/update
        wget "http://sv.brainexplode.com/$targetVER.tar.gz" -O /root/update/update.tar.gz
        tar -xzf /root/update/update.tar.gz -C /root/update/
        /bin/sh /root/update/update.sh
        echo $targetVER > /root/ver
        rm -fR /root/update
        echo Update Completed >> /root/checkSSHServer.log
        reboot
fi
