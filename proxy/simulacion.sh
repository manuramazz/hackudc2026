#!/bin/bash

sudo tc qdisc del dev wlp1s0 root 2>/dev/null || true

sudo tc qdisc add dev wlp1s0 root handle 1: htb default 10
sudo tc class add dev wlp1s0 parent 1: classid 1:10 htb rate 500kbit ceil 500kbit

sudo tc qdisc add dev wlp1s0 parent 1:10 handle 10: netem delay 10ms loss 10%

sudo tc qdisc change dev wlp1s0 parent 1:10 netem delay 100ms 20ms distribution normal