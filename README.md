Homogeneous vectors for Lua 5.2
===============================

Use luarocks to compile.

Supported C-types:
------------------
 * *int8_t* `hv.i8`
 * *int16_t* `hv.i16`
 * *int32_t* `hv.i32`
 * *int64_t* `hv.i64`
 * *uint8_t* `hv.u8`
 * *uint16_t* `hv.u16`
 * *uint32_t* `hv.u32`
 * *uint64_t* `hv.u64`
 * *float* `hv.f32`
 * *double* `hv.f64`

Example usage:
--------------
```
local hv = require 'hv'
local x = hv.i32( 10 )    -- 10 is the size of vector
local y = hv.i32( 10, 2 ) -- 2 is fill value ( by default 0 so x elements are zeroes )	
x[1] = 5 -- indexing is 1-based
x[2] = 6 
local z = x + y
print( z[1], z[2], z[3] ) -- prints 7, 8, 2
```
Supported operations:
---------------------
	+,-,*,/
	.. -- concat two vectors
	== -- equality check, yields true only if all elements are equal

Limitations:
------------

It's impossible to initialize int64 and uint64 with large numbers ( more than 2^52 if lua_Number is 64-bit floating point )
