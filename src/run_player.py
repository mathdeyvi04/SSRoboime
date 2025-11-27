from term.Booting import Booting
from agent.Agent import Agent
from time import sleep

boot = Booting()

player = Agent(boot.options)

sleep(60)

player.scom.close()
