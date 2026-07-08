# Language syntax

The embedded language in LORD is a small, indentation-based scripting language. It supports assignments, arithmetic, comparisons, booleans, function calls, and simple control flow. It does not support defining new functions.

## Statements

A script is made of statements. The most common ones are:

- assignment: `name = expression`
- expression: a value, arithmetic expression, comparison, or function call
- conditional block: `if ...:` followed by an indented block
- loop block: `while ...:` followed by an indented block

## Expressions

Expressions can use:

- numbers, such as `0`, `3`, `12.5`
- booleans, written as `True` and `False`
- identifiers, such as `x` or `systems_loaded`
- arithmetic operators: `+`, `-`, `*`, `/`
- comparison operators: `==`, `!=`, `<`, `>`, `<=`, `>=`
- parentheses for grouping: `(a + b) * 2`

## Function calls

Builtins are called using the usual function-call syntax:

```text
print(42)
exit(0)
print_layout(1)
```

## Reserved keywords

The following words are reserved by the language and should not be used as variable or function names:

- `if`
- `else`
- `while`
- `True`
- `False`
- `None`

## Control flow

Blocks use indentation and require a colon after the condition:

```text
x = 2
if x > 0:
    print(x)
else:
    print(0)
```

```text
count = 0
while count < 3:
    print(count)
    count = count + 1
```

## Notes

- Indentation is significant.
- `if` and `while` statements must be followed by a colon and an indented block.
- The language has a small builtin set; only those builtins can be called directly.
