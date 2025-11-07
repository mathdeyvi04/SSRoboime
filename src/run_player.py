from term.Booting import Booting
from agent.Agent import Agent

boot = Booting()
if_debug_mode = boot.options[-1][1] == '1'

player = Agent(boot.options)
