# Eigenmath with a Pratt parser

## Overview

I took George Weigt's Eigenmath project and replaced the recursive descent parser
in `scan.cpp` with a Vaughan Ronald Pratt parser in `pratt.cpp`.

## Building

To build the project, you need GCC under Linux and the following command in the
project's directory:
```bash
$ make
```  
This will produce object files `*.o` and an executable `math` in the project's directory.

## Testing

To test the program, you can use the provided `test-script.txt`:
```bash
$ ./math test-script.txt
```  
You can also run the command `selftest`:
```bash
$ ./math
> selftest
```
To quit the program, enter Ctrl-C:
```bash
$ ./math
> ^C
$
```

## References

In this implementation of Pratt's parser I made use of the material presented in this article:

- _Top Down Operator Precedence_, Vaughan R. Pratt, 1973 available at
  [http://tdop.github.io/](http://tdop.github.io/).  Original description of
  Pratt's algorithm.
  
The concept of nbp (next binding power) is from:

- http://www.engr.mun.ca/~theo/Misc/pratt_parsing.htm