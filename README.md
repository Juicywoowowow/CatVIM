# Camel

A high-level programming language with explicit, verbose syntax. Written in pure C with minimal dependencies. Compiles to bytecode executed by **CemVM** (Camel Virtual Machine).

## Features

- **Static Typing**: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32`, `f64`, `string`, `bool`, `void`
- **Explicit Syntax**: Heavy use of `define` keyword for clear, self-documenting code
- **Bytecode Compilation**: Compiles to stack-based VM bytecode
- **REPL**: Interactive read-eval-print loop
- **Pure C**: Written in 100% C with only stdlib dependencies

## Syntax Example

```camel
-- Every program must start with a namespace
namespace define myprogram

-- Variable declarations
define variable{"x"}: i64 = 100
define variable{"name"}: string = "Camel Language"

-- Function definitions
define public function{"add"}
    define parameter{"a"}: i64
    define parameter{"b"}: i64
    define return_type: i64
    
    define return a + b
define end public function{"add"}

-- Control flow
define if x > 50
    define call print{"Large number"}
define else
    define call print{"Small number"}
define end if

-- Loops
define while x > 0
    define set x = x - 1
define end while

define for{"i"} from 1 to 10
    define call print{i}
define end for
```

## Building

```bash
make
```

The executable will be created at `build/camel`.

## Usage

**REPL Mode:**
```bash
./build/camel
```

**Execute .cam file:**
```bash
./build/camel program.cam
```

## Project Structure

```
Camel/
├── src/              # All source files (flat structure)
├── build/            # Build output
├── Makefile
└── README.md
```

## Development

**Clean build artifacts:**
```bash
make clean
```

**Debug build with trace:**
```bash
make debug
```

## Implementation Status

- ✅ Lexer (tokenization)
- ✅ Bytecode infrastructure (chunks, opcodes)
- ✅ CemVM (stack-based virtual machine)
- ✅ CLI and REPL
- 🚧 Parser (in progress - stub implementation)
- 🚧 Compiler (in progress - stub implementation)

## License

TBD
