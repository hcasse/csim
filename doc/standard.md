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

