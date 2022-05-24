## binstring
translates directives given in the first argument into a string

### syntax

binstring \<expression\>

### directives

* hex: `|<hexadecimal number>|`:  
translates the hexadecimal number into bytes.

* repeat: `*<count>{<content>}`:  
repeats content count times, count can also contain directives

* number: `#<size><endianess><value>#`:  
size is the size in bytes of the number, and it can be 1,2,4 or 8  
endianess can be 'b'(big endian) or 'l'(little endian)  
it represents the specified decimal value as bytes in the specified endianess

* range: `@<length>,<count>;`:  
writes a sequence of count blocks of length length,  
each block is composed of length-1 '0xAA' bytes,  
and one containing the block's number

* escape: `<character>`:  
prints the character verbatim and prevents it from opening or closing a directive

* characters outside these sequences are printed as they are

### usage examples

```
$ binstring "this|0a|output|0a|has|0a|lf|0a|newlines|0d0a|but|0d0a|these|0d0a|are|0d0a|crlf|0d0a|newlines"
this
output
has
lf
newlines
but
these
are
crlf
newlines
```

```
$ binstring "tere are many *32{words|0a|}"
there are many words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
words
```

## bin2shell

converts the bytes in its standard input into octal sequences recognised by bash
