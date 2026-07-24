# Template Format

## Top-level

* `comp` (string): lower-case component name
* `COMP` (string): upper-case component name
* `arch` (string): lower-case architecture name
* `arch` (string): upper-case architecture name
* `date` (string): generation date
* `events` (collection): events
* `port_count` (string): count of ports
* `ports` (collection): ports
* `register_count` (string): count of registers
* `registers` (collection): registers
* `io_comp` (boolean): true if the component perform IO.
* `update` (string): action attribute of `update` operation.

## Registers

* `count` (string): count of items in register.
* `init` (string): initial value of the register.
* `intern` (boolean): true if the register is internal.
* `ctype` (string): C type of the register.
* `is_read_only` (boolean): true of the register is read-only.
* `is_write_only` (boolean): true of the register is write-only.
* `label` (string): label to name the register.
* `name` (string): lower-case name of the register.
* `NAME` (string): upper-case name of the register.
* `multiple` (boolean): true if register count > 1.
* `offset` (string): offset of the register.
* `on_write` (string): code executed when the register is written.
* `size` (string): size in bits of the values in the registers.
* `stride` (string): stride between register items.
* `type` (string): register type.
* `update_on_write` (boolean): true to call update on register write.


## Port

* `base` (string): base index of the port in the port array.
* `count` (string): count of port instances.
* `ctype` (string): C type of the port.
* `indexes` (collection): indexes of the port items.
* `label` (string): label to name the port.
* `multiple` (boolean): true if port count > 1.
* `name` (string): lower-case name of the port.
* `on_input` (string): code when the port receives a signal.
* `type` (string): port type.
* `update_on_input` (boolean): true to call update on port input.


## Port Item

* `index` (string): index of the current item.
* `label` (string): label of the current item.

## Events

* `name` (string): event name.
* `on_trigger` (string): code executed when the event arises.

