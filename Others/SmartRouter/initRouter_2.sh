#!/bin/sh

echo == OpenWRT Router Service Install ==
echo ============== STEP 2 ==============

freeSpace=$(df | grep 'rootfs' | awk '{print $4}')

if [ $freeSpace -gt 200 ]; then
	echo + Free Space Check Passed.
else
	echo - No Enough Free Space.
	exit 1
fi	

# Start
echo + Update Package List.
opkg update

echo + Install Packages.
opkg install block-mount

echo + Apply Settings.

echo "config 'global'" > /etc/config/fstab
echo "	option	anon_swap	'0'">> /etc/config/fstab
echo "	option	anon_mount	'1'">> /etc/config/fstab
echo "	option	auto_swap	'0'">> /etc/config/fstab
echo "	option	auto_mount	'1'">> /etc/config/fstab
echo "	option	delay_root	'5'">> /etc/config/fstab
echo "	option	check_fs	'0'">> /etc/config/fstab
echo " ">> /etc/config/fstab
echo "config 'mount'">> /etc/config/fstab
echo "	option	target	'/overlay'">> /etc/config/fstab
echo "	option	device	'/dev/sda1'">> /etc/config/fstab
echo "	option	fstype	'ext4'">> /etc/config/fstab
echo "	option	options	'rw,sync'">> /etc/config/fstab
echo "	option	enabled	'1'">> /etc/config/fstab

rm -f /root/initRouter_2.sh

echo "sleep 30" > /etc/rc.local
echo "/bin/sh /root/postMail.sh" >> /etc/rc.local
echo "exit 0" >> /etc/rc.local

/etc/init.d/fstab enable
mount /dev/sda1 /mnt/
mkdir /tmp/root
mount -o bind / /tmp/root
cp /tmp/root/* /mnt -a
umount /tmp/root
echo OpenWRT Booted From USB Device > /mnt/etc/banner

echo + Download Script For Step 3.
wget http://sv.brainexplode.com/initRouter_3.sh -q -O /mnt/root/initRouter_3.sh
chmod 755 /mnt/root/initRouter_3.sh
echo "sleep 30" > /mnt/etc/rc.local
echo "/bin/sh /root/postMail.sh" >> /mnt/etc/rc.local
echo "/bin/sh /root/initRouter_3.sh" >> /mnt/etc/rc.local
echo "exit 0" >> /mnt/etc/rc.local

echo + Reboot To Process Step 3.
reboot