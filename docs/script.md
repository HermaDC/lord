A script is a sequence of tokens separated by space, comma, or semicolon.

A token is either:

- a positive integer

- a switch expression

The script must begin and end with a number.

A switch expression has the form:

`SW( <content> )`

Where <content>:

* is a sequence of tokens
* must start with a number
* must end with a number
* may contain nested switches

Nested switches must be fully closed before the enclosing switch is closed

Example of a valid script:

`1, SW( 2, SW(3, 4), 5), 6`

Comments can be made interchangeably with `#`or `//`. anything followed won't be tokenized.
