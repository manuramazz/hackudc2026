#!/bin/bash
set -euo pipefail

PROXY_IP="10.64.151.154"
CLIENT_IP="10.64.151.20"
SERVER_IP="10.64.151.49"

TCP_PORT=5555
UDP_PORT=5001

echo "[*] enable ip_forward"
sudo sysctl -w net.ipv4.ip_forward=1 >/dev/null

# (Útil en entornos raros; si no quieres, comenta estas 2 líneas)
sudo sysctl -w net.ipv4.conf.all.rp_filter=2 >/dev/null
sudo sysctl -w net.ipv4.conf.default.rp_filter=2 >/dev/null

echo "[*] flush rules (solo filter/nat)"
sudo iptables -t nat -F
sudo iptables -F

# Asegura que FORWARD no corta por policy (si tu policy ya es ACCEPT, da igual)
sudo iptables -P FORWARD ACCEPT

echo "[*] DNAT: todo lo que vaya a PROXY se reenvía al peer real"

# ===== UDP 5001 (bidireccional) =====
# CLIENT -> PROXY:5001 => SERVER:5001
sudo iptables -t nat -A PREROUTING -p udp -s "$CLIENT_IP" -d "$PROXY_IP" --dport "$UDP_PORT" \
  -j DNAT --to-destination "$SERVER_IP:$UDP_PORT"

# SERVER -> PROXY:5001 => CLIENT:5001
sudo iptables -t nat -A PREROUTING -p udp -s "$SERVER_IP" -d "$PROXY_IP" --dport "$UDP_PORT" \
  -j DNAT --to-destination "$CLIENT_IP:$UDP_PORT"

# ===== TCP 5555 (bidireccional) =====
# SERVER inicia: SERVER -> PROXY:5555 => CLIENT:5555 (CLIENT escucha)
sudo iptables -t nat -A PREROUTING -p tcp -s "$SERVER_IP" -d "$PROXY_IP" --dport "$TCP_PORT" \
  -j DNAT --to-destination "$CLIENT_IP:$TCP_PORT"

# Datos/ACKs desde CLIENT (porque para él el peer es PROXY): CLIENT -> PROXY:5555 => SERVER:5555
sudo iptables -t nat -A PREROUTING -p tcp -s "$CLIENT_IP" -d "$PROXY_IP" --dport "$TCP_PORT" \
  -j DNAT --to-destination "$SERVER_IP:$TCP_PORT"

echo "[*] FORWARD allow (stateful + puertos)"
sudo iptables -A FORWARD -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT

# UDP
sudo iptables -A FORWARD -p udp -s "$CLIENT_IP" -d "$SERVER_IP" --dport "$UDP_PORT" -j ACCEPT
sudo iptables -A FORWARD -p udp -s "$SERVER_IP" -d "$CLIENT_IP" --dport "$UDP_PORT" -j ACCEPT

# TCP
sudo iptables -A FORWARD -p tcp -s "$SERVER_IP" -d "$CLIENT_IP" --dport "$TCP_PORT" -m conntrack --ctstate NEW -j ACCEPT
sudo iptables -A FORWARD -p tcp -s "$CLIENT_IP" -d "$SERVER_IP" --dport "$TCP_PORT" -m conntrack --ctstate NEW -j ACCEPT

echo "[*] SNAT: el receptor siempre verá PROXY_IP como origen"

# Hacia CLIENT: que todo lo que entregue al CLIENT parezca venir del proxy
sudo iptables -t nat -A POSTROUTING -d "$CLIENT_IP" -p tcp --dport "$TCP_PORT" -j SNAT --to-source "$PROXY_IP"
sudo iptables -t nat -A POSTROUTING -d "$CLIENT_IP" -p udp --dport "$UDP_PORT" -j SNAT --to-source "$PROXY_IP"

# Hacia SERVER: que todo lo que entregue al SERVER parezca venir del proxy
sudo iptables -t nat -A POSTROUTING -d "$SERVER_IP" -p tcp --dport "$TCP_PORT" -j SNAT --to-source "$PROXY_IP"
sudo iptables -t nat -A POSTROUTING -d "$SERVER_IP" -p udp --dport "$UDP_PORT" -j SNAT --to-source "$PROXY_IP"

echo "[*] done"