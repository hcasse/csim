# NML

SimNML or NML is an _Architecture Description Language_ allowing to description an _Instruction Set Architecture_ and then do derive the infrastrucrture to simulate or synthetize it.

In **CSim**, we mainly use the behavioral part of the language to define the behaviour of the components. It is bascially a _Register Transfer Language_ as it aims to represent architecture elements working at bit level with a mix of construction in favour of functional programming while keeping synthesis possible.

SimNML is mostly statically typed providing expressions to compute side-effect-free values and statement to perform state assignment and to manage control flow.


## Typing

SimNML types works at bit-level but mainly cover simple types:

* `bool` -- 0 or 1,
* `int`(_N_) -- signed integer of _N_ bits,
* `card`(_N_) -- unsigned integer of _N_ bits.

Type may also be named with specification level syntax:

`type` _ID_ `=` _TYPE_


## Expressions

As types, expressions are designed to make easier working atbit level.

First, constants may be expression using classic C notation:

* decimal integer,
* 0[xX]-prefixed hexadecimal integer,
* 0[bB]-prefixed binary integer,
* floating-point integer.

The expressions can be:

* _ID_ refering to a single register, a single port or a constant.
* _ID_ [ _EXPRESSION_ ] to access a multiple register or port.
* `+` _EXPRESSION_ for the identity operation,
* `-` _EXPRESSION_ for negation,
* `~` _EXPRESSION_ for bit inversion,
* `!` _EXPRESSION_ for logic negation,
* _EXPRESSION_ `+` _EXPRESSION_ for addition,
* _EXPRESSION_ `-` _EXPRESSION_ for subtraction,
* _EXPRESSION_ `*` _EXPRESSION_ for multiplication,
* _EXPRESSION_ `/` _EXPRESSION_ for division,
* _EXPRESSION_ `%` _EXPRESSION_ for modulo,
* _EXPRESSION_ `<<` _EXPRESSION_ for left shift,
* _EXPRESSION_ `>>` _EXPRESSION_ for right shift,
* _EXPRESSION_ `<<<` _EXPRESSION_ for left rotation,
* _EXPRESSION_ `>>>` _EXPRESSION_ for right rotation,
* _EXPRESSION_ `&` _EXPRESSION_ for bit-to-bit AND,
* _EXPRESSION_ `|` _EXPRESSION_ for bit-to-bit OR,
* _EXPRESSION_ `^` _EXPRESSION_ for bit-to-bot XOR,
* _EXPRESSION_ `==` _EXPRESSION_ for equality test,
* _EXPRESSION_ `!=` _EXPRESSION_ for inequality test,
* _EXPRESSION_ `<` _EXPRESSION_ for greater-than test,
* _EXPRESSION_ `<=` _EXPRESSION_ for greator-equal test,
* _EXPRESSION_ `>` _EXPRESSION_ for lesser-than test,
* _EXPRESSION_ `>=` _EXPRESSION_ for less-or-equal test,
* _EXPRESSION_ `&&` _EXPRESSION_ for logic AND,
* _EXPRESSION_ `||` _EXPRESSION_ for logic OR.
* _EXPRESSION_ `<` _EXPRESSION_ `..` _EXPRESSION_ `>` -- to access a bit range in the first expression,
* _EXPRESSION_ `..` _EXPRESSION_ -- concatenates the bits of the two sub-expressions.
* `if` _EXPRESSION_ `then` _EXPRESSION_ `else` _EXPRESSION_ `endif` for conditional expression.
* `switch(` _EXPRESSION_ `) {` (`case` _CONSTANT_ `:` _EXPRESSION_)* `default:` _EXPRESSION_ `}` for a switch-case expression.
* `coerce(` _TYPE_ `,` _EXPRESSION_ `)` to convert an expression.


## Statements

The following statements are available:

* _LOCATION_ `=` _EXPRESSION_ `;` for perform an assignment to a register or to a port.
* `if` _EXPRESSION_ `then` _STATEMENT_ (`else` _STATEMENT_)? `endif;` for a condition.
* `switch(` _EXPRESSION_ `) {` (`case` _CONSTANT_ `:` _STATEMENT_)* (`default:
` _STATEMENT_)? `};` for a switch-case.
* `let` _ID_ (`:` TYPE_)? (`=` _EXPRESSON_)? `;` to define a local variable.
* `for` _ID_ `in` _CONSTANT_ `..` _CONSTANT_ `do` _STATEMENT_ `enddo;` -- for a bounded repetition.
* `interrupt` _CODE_ -- interrupt the program with the given code.

The _LOCATION_ may be one of specification that may be assigned:

* _ID_ for a single register or port,
* _ID_ [ _EXPRESSION_ ] for a multiple register or port with index expression.


## Strings

In addition to integer and floatting-point values, string expression may be useful to give meaningful names to registers and ports (typically for user display).

This means that there is an implicit type representing string and specially supported by two type of expressions:

* `"` _CHARACTER_* `"` defines a string constant,
* `format(` _STRING_, _EXPRESSION_* `)` to define a formatted a-la C formatted string with usual escapes: %d, %x, %c, %s, etc.










