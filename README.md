# hackudc2026
Entorna para executar Doom en un satélite orbitando con arquitectura ARM

SUPOSICIONES:
📏 RTT físico ideal en LEO (600 km)

Distancia aproximada:

Tierra → satélite: ~600–1.200 km

Ida y vuelta total aproximada: ~2.000 km

Velocidad luz ≈ 300.000 km/s

2000
/
300000
≈
0.0067
𝑠
=
6.7
𝑚
𝑠
2000/300000≈0.0067s=6.7ms

Eso sería el RTT físico ideal mínimo.

🎯 RTT realista en LEO

En un sistema real se añade:

Procesamiento digital

Codificación FEC

Buffering

Routing IP

Entonces el RTT realista suele estar en:

15–40 ms

Si quieres ser conservador:
👉 25 ms RTT medio es una suposición muy razonable.

🟢 Microsatélite académico

512 kbps – 2 Mbps razonable

media de tamaño de frame: 4kb
se pueden asumir: 512k / 32 = 16 fps