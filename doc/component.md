# How to define a component?

**CSim** components are described in [NML](NMP.md), a programming language dedicated to description of instruction set and micro-architecture. The original goal of [NML](NMP.md) was to generate a complete suite of tools (compiler, assembly, simulator) in order to explore the design of microprocessor. In **CSim**, we mainly use it as a simulator generator with [GLISS tool suite](https://sourcesup.renater.fr/projects/gliss2).

An example of component described in [NML](NMP.md) in `stm32/GPIO.md`.

Basically, an [NML](NMP.md) description is a suite of specifications:

* `let` to define constants,
* `type` to define bit-level types,
* `reg` to define a component register,
* `port`, added by **CSim**, to describe an input-output port.

This description is then processed by command `gliss-cim` to generate C sources that are compiled and can be used with **CSim** main library to simulate a whole micro-controller board. Each [NML](NMP.md) file describe a component (GPIO, USART, ADC, PWM, etc) that may be instanciated several times. The whole simulation environment is described by a [YAML file](board.md) as `stm32/board.yaml`.


# Constants used by CSim

The [NML](NMP.md) file supports the following constants:

* `arch` (optional) -- architecture the component is part of,
* `component` (mandatory) -- name of the component,
* `io_comp` (optional) -- set to 1 if the component is an i/o component,
* `copyright` (optional) -- copyrioght owner of this file,
* `date` (optional) -- date of this version,
* `license` (optional) -- license of this file,
* `version` (optional) -- current version.


# Registers

In **CSim** (in the opposite to classic [NML](NMP.md)), represents an input/output register allowing the micro-controller to control the component. They are usually mapped memory with an address relative to the base address of the component. As a component may have several instances, theur address is specified as an offset relative to the base address.

A register is described by:

```
reg ID [ COUNT, TYPE ]
	ATTRIBUTES
```

With:

* _ID_ -- identifier of the register (then used for reading/writing its content).
* _COUNT_ -- 1 for a single, more than 1 for an array of registers.
* _TYPE_ -- its type,
* _ATTRIBUTES_ -- list of pairs _ID_ `=` _VALUE_ with _VALUE_ being a constant, an expression or a brace-framed statement list.

The following attributes are usually supported:

* `label` = _STRING EXPRESSION_ -- to generate the name of an item of registe for an array .
* `offset` = _INTEGER CONSTANT_ -- define the offset relative to the component base address,
* `on_write` = { _CODE_ } -- code called the processor performs a write to the register to let the component perform some actions (written value is already stored in the register).
* `on_read` = { _CODE_ } -- code called before a processor read to setup the content of the register.
* `read_only` = _0 or 1_ -- the register is read-only,
* `write_only` = _0 or 1_ -- the register is write-only,
* `intern` = _0 or 1_ -- define if the register is accessible or not by the program,
* `init` = {_VALUE_} -- the default value of the register.

All expressions and statements used in attributes can use the pre-defined variables below:

* `__INDEX` -- index of the register when a register array is used.
* `__COM_NUM` -- number associated with the current instance of the component.

## Ports

The ports is an add-on of **CSim** to [NML](NMP.md) and aims to represent an internal or internal point of connection of microcontrollers components or of external peripherals. Basically, they are bidirectionnal but they have to be configured (internally in a component with a drection).

Basically, their nature is _electric_ supporting digital or continuous voltage but they may be used to transport other kind of energy, physical or logic for simulation performance purpose (network communication as serial, I2C, etc).

A port is described by:

```
port ID ( COUNT, TYPE )
	ATTRIBUTES
	
```

With _ID_ the identifier of the port, _COUNT_ the number of pins in the port and _TYPE_ the type of the port.

The _ATTRIBUTES_ can be:

* `label` = _STRING EXPRESSION_ -- to generate the name of a port pin for an array .
* `on_update` = { _CODE_ } -- code called each time a register is changed that may be impact the set of pins of the port.
* `on_input` = { _CODE_ } -- code called each time the value as input of the port pin is changed.

All expressions and statements used in attributes can use the pre-defined variables below:

* `__INDEX` -- index of the register when a register array is used.
* `__COM_NUM` -- number associated with the current instance of the component.

Notice that each time a port pin is changed (by assignint it), the emitted signal is send to the port connected with this one.


## Events

 The event is an add-on of **CSim** to [NML](NMP.md) and aims to give the ability to simulate an event triggering after a set amount of time. 

An event is described by : 

```
event ID
	ATTRIBUTES
```

With _ID_ the identifier of the event.

The _ATTRIBUTES_ can be: 

* `on_update` = { _CODE_ } -- code called each time a register is changed that may be impact the event.
* `on_trigger` = { _CODE_ } -- code called when the event trigger.

An event can be schedule using `schedule ID in TIME` with _ID_ the identifier of the event and _TIME_ the time in which the event should trigger.
An event can be cancelled using `cancel ID` with _ID_ the identifier of the event.

Event supports also included the support of the `now` variable, accessible from anywhere in the code, that is the current internal time of the board (it will be the time of the component when it is implemented.)