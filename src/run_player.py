from term.Booting import Booting
from agent.Agent import Agent

boot = Booting()

p = Agent(boot.options)

while True:
    p.scom.send()
    p.scom.receive()
