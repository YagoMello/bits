# Bits
C++20 Header-only bit abstraction library

Can be used to simplify register access, break packets 

## How to Install
1. Download the source
2. Move the file `bits.hpp` to your project source folder
3. `#include "bits.hpp"` where you want to use the library

## How to use
Static (normal) bit:
```cpp
#include "bits.hpp"
// ...

// ==== creating objects ====

// For hardware registers:
// MYREGISTER:
// B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 
// To create a bit representing B2:
bits::bit b2 = bits::make_bit<MYREGISTER, 2>(); // or
bits::bit b2(bits::make_bit<MYREGISTER, 2>());

// For normal variables:
uint64_t var = 42;
bits::bit var_b27 = bits::make_bit<var, 27>(); // or
bits::bit var_b27(bits::make_bit<var, 27>());

const uint_fast16_t another_var;
bits::bit avar_b9 = bits::make_bit<another_var, 9>(); // or
bits::bit avar_b9(bits::make_bit<another_var, 9>());

// ==== assigning values ====

// before:
MYREGISTER = 0b0000'0000;

// To set b2:
b2 = true; // or
b2.set();  // or
b2.write(true);

// after:
std::cout << MYREGISTER << "\n"; // prints 4 = 0b0000'0100

// To clear b2:
b2 = false; // or
b2.clr();   // or
b2.write(false);

// ==== using values ====

// To use the value of b2:
bool var1 = b2; // or
bool var2 = b2.read();

MYREGISTER = 0b1111'1011;
if(b2) // FALSE

MYREGISTER = 0b0000'0100;
if(b2) // TRUE

MYREGISTER = 0b0101'0101;
if(b2) // TRUE

// ==== misc ====

// It is ok to create an empty object:
bits::bit new_bit;
// ...

// Assigning a variable to an object:
new_bit.same_as(b2); // correct
new_bit.same_as(bits::make_bit<MYREGISTER, 2>()); // correct
new_bit = bits::make_bit<MYREGISTER, 2>(); // INCORRECT!
bits::bit & bit_ref = b2; // correct
// ...

new_bit = false; // MYREGISTER.2 = 0

```

Dynamic bit:
```cpp
// pointer to var:
auto * reg_ptr = &REG1;

// creating bit:
dyn_bit = bits::make_bit<reg_ptr, 5>();

REG1 = 0x00;
REG2 = 0b1111'1111;

if(dyn_bit) // FALSE

// reassigning var:
reg_ptr = &REG2;

if(dyn_bit) // TRUE

std::cout << REG2 << "\n"; // prints 255 = 0b1111'1111
dyn_bit = false;
std::cout << REG2 << "\n"; // prints 223 = 0b1101'1111

```

## `operator =` vs `.same_as()`:
The `operator =` copies only the value.

The function `.same_as(obj)` copies the class parameters.
```cpp
// this is NOT operator =, and the copy
// constructor is not called due to 
// copy elision
bit_a = bits::make_bit<xxx, yyy>();
bit_b = bits::make_bit<zzz, ttt>();
bit_c = bits::make_bit<vvv, www>();

bit_a = false;
bit_b = false;
bit_c = true;

bit_a = bit_c;
bit_b.same_as(bit_c);

// bit_a = true
// bit_b = true
// bit_c = true

bit_a = false;

// bit_a = false
// bit_b = true
// bit_c = true

bit_b = false;

// bit_a = false
// bit_b = false
// bit_c = false

bit_c = true;

// bit_a = false
// bit_b = true
// bit_c = true
```

## About cv-qualifiers
`volatile`/`const` vars are supported.

A `bit` to const var has an empty write function:
```cpp
const uint32_t constvar = 0b1111'1111;
bits::bit cbit = bits::make_bit<constvar, 6>();

cbit = false; // It is not error/undefined-behavior!

std::cout << cbit << "\n"; // prints 1
std::cout << constvar << "\n"; // prints 255 = 0b1111'1111

if(cbit) // TRUE

```
A `bit` to volatile works like a normal `bit`.
