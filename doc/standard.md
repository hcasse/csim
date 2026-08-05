# Standard Components

## LED

Implemetns a simple LED.

**Type:** `led`

**Ports:**
* `IN`: displayed value (0 switched off, 1 switched on)

**Registers:**
* `IN`: displayed value

**Output:**
* code 1: { 0, 1} -- LED state


## Button

**Type:** `button`

**Ports:**
* `OUT`: button state (0 released, 1 pushed)

**Registers:**
* `OUT`: button state

**Input:**
* code 1: {0, 1} -- button state


## 7-segment Displayer

**Type:** `sseg`

**Ports:**
* `UP`: up segment control (0 off, 1 on)
* `UPLEFT`: up-left segment control (0 off, 1 on)
* `UPRIGHT`: up-right segment control (0 off, 1 on)
* `MID`: middle segment control (0 off, 1 on)
* `DOWNLEFT`: down-left segment control (0 off, 1 on)
* `DOWNRIGHT`: down-right segment control (0 off, 1 on)
* `DOWN`: down segment control (0 off, 1 on)

**Registers:**

**Outputput:**
* code 1: {0, 1} -- 7-segment state 1-bit/segment
	(0: up, 1: up left, 2: up right, 3: middle, 4: down left, 5: down right, 6: down)

