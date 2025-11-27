from term.Booting import Booting
from agent.Agent import Agent
from time import sleep

boot = Booting()

players = []
for i in range(0, 11):
    players.append(Agent(boot.options))
    boot.options[4][1] += 1

for p in players:
    p: Agent
    p.beam()
    p.scom.send()

for p in players:
    p.scom.receive()



sleep(60)

for p in players:
    p.scom.close()
