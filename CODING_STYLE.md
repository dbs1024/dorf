# C++ Coding Style

## File Header

Every source file must begin with a copyright notice using the following format:

```cpp
// Copyright (c) Darrin Stewart. All rights reserved.
```

## Naming

### Macros
All upper-case with underscores separating words.

```cpp
#define MY_MACRO(x) doSomething(x)
#define MAX_BUFFER_SIZE 1024
```

### Classes and Types
Mixed case, starting with an upper-case letter (PascalCase).

```cpp
class MyClass {};
struct MyStruct {};
typedef unsigned int MyUint;
using MyAlias = std::vector<int>;
```

### Variables
Mixed case, starting with a lower-case letter (camelCase).

```cpp
int myVariable;
float deltaTime;
```

### Member Variables
- **Class** members use an `m_` prefix.
- **Struct** members do not use a prefix.

```cpp
class MyClass
{
    int m_count;
    float m_deltaTime;
};

struct MyStruct
{
    int count;
    float deltaTime;
};
```

### Member Initialization
Prefer in-class initialization for member variables over constructor initializer lists. This keeps the default value at the declaration site, visible alongside the type and name.

```cpp
// Correct
class MyClass
{
    int   m_count = 0;
    bool  m_active = false;
    void* m_handle = nullptr;
};

// Wrong
class MyClass
{
    int   m_count;
    bool  m_active;
    void* m_handle;

    MyClass() : m_count(0), m_active(false), m_handle(nullptr) {}
};
```

Use a constructor initializer list only when initialization depends on a constructor parameter or requires logic that cannot be expressed inline.

### Acronyms
Acronyms are treated as ordinary words — only the first letter is capitalized when the word would be capitalized, and the rest are lower-case. This applies to all identifiers: filenames, class names, variable names, etc.

```cpp
// Correct
class MyHlslCompiler {};
class GpuDevice {};
int myHlslSource;

// Wrong
class MyHLSLCompiler {};
class GPUDevice {};
int myHLSLSource;
```

## Function Parameters

### Out Parameters
Out parameters (parameters passed by non-const reference or pointer that the function writes to) must come first in a function declaration, before all input parameters.

```cpp
// Correct
ErrCode createSomething(SomethingHandle& outSomething, int foo, int bar);
ErrCode getResult(Result& outResult, const Config& config);

// Wrong
ErrCode createSomething(int foo, int bar, SomethingHandle& outSomething);
```

Exceptions are allowed when a different order is necessary (e.g., to match an external API or a well-established convention), but must be documented with a comment explaining why.

```cpp
// Exception: matches the external LibFoo API signature which cannot be changed.
void libFooCallback(int status, Result& outResult);
```

## Forward Declarations

Forward declarations should appear near the top of a header, just after any `#include` directives and before any other declarations.

```cpp
// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include <some/header.h>

class MyForwardDeclared;
struct AnotherForwardDeclared;

class MyClass
{
    // ...
};
```

## Formatting

### Indentation
Use **tabs**, sized to **4 spaces**.

### Braces — Allman Style
Opening and closing braces each appear on their own line, aligned vertically with the associated control statement.

```cpp
if (condition)
{
    doSomething();
}

for (int i = 0; i < count; ++i)
{
    process(i);
}

class MyClass
{
public:
    void myMethod()
    {
        doWork();
    }
};
```

### Single-Statement Blocks
Braces may be omitted for single-statement bodies (optional).

```cpp
if (condition)
    doSomething();

for (int i = 0; i < count; ++i)
    process(i);
```
