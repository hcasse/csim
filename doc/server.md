# Server

**CSIM** can be run as server behind socket or pipes. It nehaves as server and
only the client is issuing any action. It can work with in lock-step mode
(typically for debugging) or in running mode (when the application is launched).
In any case, in order to reduce througput, it is able to transmit only things
that have changed.

## Message passing

In the following, we use the notation:
* 'c' for an explicit ASCII character,
* BYTE to transmit a 8-bit byte or a boolean (0 or 1),
* HALF to transmit a 16-bit word,
* WORD to transmit a 32-bit word.
* STRING to transmit a character string ended with '\0'.

HALF and WORD are transmitted in the format of the machine running the server.

The client commands starts with a single character following with one or several
commands with the format defined above.

The answer starts with one byte that may be:
* '0' -- all is ok and the results are put after,
* '!' STRING -- if there is an error and the string follows.

In the following, we describe the message and the corresponding answers.



## Command Summary

| Code	| Command 			|
|-------|-------------------|
| ','	| step				|
| ';'	| run				|
| '!'	| stop				|
| '+'	| add_bp			|
| '-'	| remove_bp			|
| '#'	| count_components	|
| '>'	| get_io_state		|
| '<'	| set_io_state		|
| '@'	| get_pc			|
| '?'	| observe_register	|
| '/'	| ignore_register	|
| '~'	| update_output		|
| '0'	| reset_board		|
| '1'	| get_byte			|
| '2'	| get_half			|
| '4'	| get_word			|
| 'B'	| load_bin			|
| 'C'	| get_core			|
| 'G'	| get_register_val	|
| 'I'	| get_ios			|
| 'L'	| load_board		|
| 'M'	| ignore_memory		|
| 'O'	| observe			|
| 'Q'	| quit				|
| 'R'	| get_register		|
| 'S'	| set_register_val	|
| 'c'	| get_component		|
| 'i'	| get_inst			|
| 'm'	| observe_memory	|
| 'v'	| version			|




## Board Messages


### load_board

	command: 'L' path: STRING
	answer:

Load the board description from _path_.


### reset_board

	command: '0'
	answer

Reset the board.


### count_components

	command: '#'
	answers: count: HALF

To get the count of components.

### get_core

	command: 'C'
	answers: index: HALF

Get the index of the core component.

### get_ios

	command: 'I'
	answers: count: HALF (index: HALF)*

Get list of _indexes_ of the _count_ components performing IO.

### get_component

	command: 'c' index: HALF
	answers: name: STRING inst: STRING type: BYTE version: WORD reg_count: HALF port_count: HALF

Get the description of component with _index_. Returns its component _name_, _inst_'ance name, its _type_, the count of registers _reg_count_ and the count of ports _port_cout_.

_Type_ may be:
	* _CSIM_SIMPLE_ = 1
	* _CSIM_CORE_ = 2
	* _CSIM_IO_ = 3

### quit

	command: 'Q'
	answer:

Quit the server.

### get_register

	command: 'R' comp: HALF reg: HALF
	answers: name: STRING offset: WORD size: HALF count: HALF stride: WORD flags: WORD type: HALF

Get the description of a register at index _reg_ from component _comp_: its _name_, its _offset_ relative, the _size_ in bits of each register, the _count_ of registers, the _stride_ in bytes separing each registers, its _fags_ and _type_.

_flags_ are unused for now but _type_ may be:

* _CSIM_NORTYPE_ = 0,
* _CSIM_BITS_ = 1,
* _CSIM_INT_ = 2,
* _CSIM_ADDR_ = 3

### version

	command: 'v'
	answer: version: STRING

Get the version of the server.


## Program Messages

### load_bin

	command: 'B' path: STRING
	answer:

Load the binary program from _path_.

### get_inst

	command: 'i' addr: WORD
	answer: size: HALF disasm: STRING

Load the description of instruction at _addr_ address. Returns its _size_ in bytes and its disassembly form _disasm_.



## State Access Messages


### get_register_val

	command: 'G'	comp: HALF	reg : HALF	index: HALF
	answer: value: WORD

Get the _value_ of register _reg_ at _index_ in component _comp_.


### set_register_val

	command: 'S'	comp: HALF	reg : HALF	index: HALF value: WORD
	answer:

Set the _value_ of a register _reg_ at _index_ in component _comp_.


### get_io_state

	command: '>' comp: HALF size: HALF
	answer:	 state: WORD*

Read _size_ words from component _comp_ _state_.


### set_io_state

	command: '<' comp: HALF size: HALF state: WORD*
	answer:

Send _size_ words of _state_ of component _comp_.


### get_pc

	command: '@'
	answer: pc: WORD

Get the value of the PC.


### get_byte

	command: '1' addr: WORD
	answer: data: BYTE

Get byte _data_ in memory at address _addr_.


### get_half

	command: '2' addr: WORD
	answer: data: HALF

Get half-word _data_ in memory at address _addr_.


### get_word

	command: '4' addr: WORD
	answer: data: WORD

Get word _data_ in memory at address _addr_.



## Debugging Messages

### step

	command: ','
	answer

Execute one instruction.

### run

	command: ';'
	answer:

Execute the code in an asynchronous way until stopped or reaching a BP.

### stop

	command: '!'
	answer:

Stop the execution.

### add_bp

	command: '+' addr: WORD
	answer:

Add a break-point at address _addr_.

### remove_bp

	command: '-' addr: WORD
	answer:

Remove a break-point at address _addr_.

### observe_register

	command: '?' comp: HALF reg: HALF index: HALF
	answer:

Add an observer for register _reg_ at _index_ in component _comp_.

### ignore_register

	command: '/' comp: HALF reg: HALF index: HALF
	answer:

Stop observing register _reg_ at _index_ in component _comp_.

### observe_memory

	command: 'm'  type: BYTE addr: WORD size: WORD
	answer:

Start observing memory at address _addr_ on _size_. _type_ provides the size
of the observed items (1 for byte, 2 for half-word, 4 for word).

### ignore_memory

	command: 'M'  type: BYTE addr: WORD size: WORD
	answer:

Stop observing memory at address _addr_ on _size_. _type_ provides the size
of the observed items (1 for byte, 2 for half-word, 4 for word).

### observe

	command: 'O'
	answer: rcount: HALF (comp: HALF reg: HALF index: HALF value: WORD)* mcount: HALF (addr: WORD value: WORD)*

Returns the changes in the observed data : _rcount_ registers and _mcount_ memories.

For a register, returns the component _comp_, the register _reg_, the _index_ and the new _value.

For a memory, returns the changed address _addr_ and the new _value_.

### update_output

	command: '~'
	answer: count: HALF (comp: HALF wcount: HALF, data: WORD*)*

Ask to report the output updates made of _count_ entries. Each entry is made of the component _comp_, the count of words _wcount_ and the corresponding _data_.


