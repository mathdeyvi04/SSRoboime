from term.Booting import Booting
from agent.Agent import Agent
from time import sleep

boot = Booting()

players = []
for i in range(0, 11):
    players.append(Agent(boot.options))
    boot.options[4][1] += 1
    sleep(1)

sleep(30)

for p in players:
    p.scom.close()
