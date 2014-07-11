#!/bin/sh

echo == OpenWRT Router Service Install ==
echo ============== STEP 1 ==============

freeSpace=$(df | grep 'rootfs' | awk '{print $4}')
mailAccount=$1
mailPassword=$2

if [ $freeSpace -gt 500 ]; then
	echo + Free Space Check Passed.
else
	echo - No Enough Free Space.
	exit 1
fi	

# Start
echo + Update Package List.
opkg update

echo + Install Packages.
opkg install msmtp-nossl kmod-usb-storage kmod-fs-ext4

echo + Apply Settings.

echo "account default" > /etc/msmtprc

echo "host smtp.exmail.qq.com" >> /etc/msmtprc
echo "from $mailAccount@brainexplode.com" >> /etc/msmtprc
echo "auth login" >> /etc/msmtprc
echo "domain brainexplode.com" >> /etc/msmtprc

echo "user $mailAccount@brainexplode.com" >> /etc/msmtprc
echo "password $mailPassword" >> /etc/msmtprc

echo "syslog LOG_MAIL" >> /etc/msmtprc

# ===================
echo "#!/bin/sh" > /root/postMail.sh
echo " " >> /root/postMail.sh
echo 'echo "From: My AP <@brainexplode.com>" > /tmp/post.mail' >> /root/postMail.sh
echo 'echo "To: AP Owner" >> /tmp/post.mail' >> /root/postMail.sh
echo 'echo "Subject: AP Boot Completed" >> /tmp/post.mail' >> /root/postMail.sh
echo 'echo " " >> /tmp/post.mail' >> /root/postMail.sh
echo 'echo "TP-Link TL-WR720N with OpenWRT" >> /tmp/post.mail' >> /root/postMail.sh
echo 'echo " " >> /tmp/post.mail' >> /root/postMail.sh
echo 'echo "ifconfig -a:">>/tmp/post.mail' >> /root/postMail.sh
echo 'ifconfig -a >> /tmp/post.mail' >> /root/postMail.sh

echo "cat /tmp/post.mail | sendmail $mailAccount@brainexplode.com" >> /root/postMail.sh
chmod 755 /root/postMail.sh


echo + Download Script For Step 2.
rm -f /root/initRouter_1.sh
wget http://sv.brainexplode.com/initRouter_2.sh -q -O /root/initRouter_2.sh
chmod 755 /root/initRouter_2.sh
echo "sleep 30" > /etc/rc.local
echo "/bin/sh /root/initRouter_2.sh" >> /etc/rc.local
echo "/bin/sh /root/postMail.sh" >> /etc/rc.local
echo "exit 0" >> /etc/rc.local

echo + Reboot To Process Step 2.
reboot