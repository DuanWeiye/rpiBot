#!/bin/sh

echo == OpenWRT Router Service Install ==
echo ============== STEP 3 ==============

freeSpace=$(df | grep 'rootfs' | awk '{print $4}')

if [ $freeSpace -gt 2048 ]; then
	echo + Free Space Check Passed.
else
	echo - No Enough Free Space.
	exit 1
fi	

# Start
echo + Update Package List.
opkg update

echo + Install Packages.
rm /usr/bin/scp
rm /usr/bin/ssh

opkg install privoxy bash coreutils-base64 openssh-client autossh

echo + Apply Settings.

#/etc/init.d/autossh enable
/etc/init.d/privoxy enable


echo "config autossh"> /etc/config/autossh
echo "	option ssh      '-i /root/.ssh/id_rsa -o StrictHostKeyChecking=no -N -T -D 8124 ubuntu@sv.brainexplode.com'">> /etc/config/autossh
echo "	option gatetime '0'">> /etc/config/autossh
echo "	option monitorport      '20000'">> /etc/config/autossh
echo "	option poll     '600'">> /etc/config/autossh


echo "confdir /etc/privoxy"> /etc/privoxy/config
echo "logdir /var/log">> /etc/privoxy/config
echo "logfile privoxy">> /etc/privoxy/config
echo "forward / .">> /etc/privoxy/config
echo "actionsfile /root/gfw_online.action">> /etc/privoxy/config
echo "actionsfile /root/gfw_custom.action">> /etc/privoxy/config
echo "actionsfile /root/gfw_default.action">> /etc/privoxy/config
echo "listen-address :8123">> /etc/privoxy/config
echo "enable-edit-actions 1">> /etc/privoxy/config
echo "forwarded-connect-retries  3">> /etc/privoxy/config
echo "keep-alive-timeout 5">> /etc/privoxy/config
echo "permit-access 172.0.0.0/24">> /etc/privoxy/config
echo "debug 8192">> /etc/privoxy/config
echo "admin-address admin@brainexplode.com">> /etc/privoxy/config


echo '{+forward-override{forward-socks5 127.0.0.1:8124 .}}' > /root/gfw_custom.action
echo '.google.' >> /root/gfw_custom.action
echo '.googleapis.' >> /root/gfw_custom.action
echo '.googlevideo.' >> /root/gfw_custom.action
echo '.youtube.' >> /root/gfw_custom.action
echo '.facebook.' >> /root/gfw_custom.action
echo '.twitter.' >> /root/gfw_custom.action


echo '{+forward-override{forward .}}' > /root/gfw_default.action
echo '.brainexplode.' >> /root/gfw_default.action


echo '{+forward-override{forward-socks5 127.0.0.1:8124 .}}' > /root/gfw_online.action


wget http://sv.brainexplode.com/checkSSHServer.sh -q -O /root/checkSSHServer.sh
wget http://sv.brainexplode.com/updateGFWList.sh -q -O /root/updateGFWList.sh


echo '0 */6 * * * /root/updateGFWList.sh' > /tmp/crontab.temp
echo '0 */1 * * * /root/checkSSHServer.sh' >> /tmp/crontab.temp
crontab /tmp/crontab.temp


echo 20140701 > /root/ver


chmod 755 /root/*.sh
rm -f /root/initRouter_3.sh


echo "sleep 30" > /etc/rc.local
echo "/bin/sh /root/postMail.sh" >> /etc/rc.local
echo "exit 0" >> /etc/rc.local

echo + Finished.
reboot