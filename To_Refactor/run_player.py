from term.Booting import Booting
from agent.Agent import Agent

boot = Booting()

p = Agent(boot.options)

while True:
    p.think_and_act()
    p.scom.receive()
