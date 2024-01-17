sudo sysctl -w net.ipv4.ip_forward=1
sudo iptables -t nat -A POSTROUTING -s 10.45.0.0/16 ! -o ogstun -j MASQUERADE
sudo ufw disable
sudo iptables -I INPUT -i ogstun -j ACCEPT
sudo iptables -I INPUT -s 10.45.0.0/16 -j DROP
sudo ip6tables -I INPUT -s 2001:db8:cafe::/48 -j DROP

sudo ip ro add 10.45.0.0/16 via 10.53.1.2

sudo docker compose up 5gc gnb ue --force-recreate
