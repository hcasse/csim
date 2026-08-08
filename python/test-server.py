#!/usr/bin/env python3

from csim import server

print("CLIENT: starting")
driver = server.Driver()
board = driver.load_board("../samples/sample1.yaml")
print(f"DEBUG: board={board}")
driver.release()
print("CLIENT: leaving")

