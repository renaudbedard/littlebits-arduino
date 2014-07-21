# C Coroutines Library for Arduino

Created by Renaud Bédard.

Version 1.0 released on July 17th, 2014 into the public domain.

## Overview

The basic idea is to be able to create blocks of code that execute sequentially, but can choose to stop temporarily and resume later. This is similar to threads, but in the case of coroutines, they never get pre-empted and will only give away focus if they explicitely yield.

The library provides a coroutine manager (the `Coroutines<N>` class) that pre-allocates and updates coroutines, as well as provides wait, suspend and resume constructs to make their usage more convenient.

A simple coroutine is declared like this :

```
// flashes a LED attached to analog pin 5 for 100ms
void flashOnce(COROUTINE_CONTEXT(coroutine))
{
    BEGIN_COROUTINE;

    analogWrite(5, 255);

    coroutine.wait(100);
    COROUTINE_YIELD;

    analogWrite(5, 0);

    END_COROUTINE;
}
```

Here, the `wait` call adds 100ms to the timer that will prevent the coroutine from resuming on the next update. The `COROUTINE_YIELD` macro exits the function, and records the state of the coroutine so it can be resumed later. The `BEGIN_COROUTINE` and `END_COROUTINE` macros do the rest required for this to work.

The `COROUTINE_CONTEXT()` macro defines the name of the context argument to the coroutine, which has the hidden type `Coroutine&` (a reference type). You may not use a regular parameter definition, since the coroutine needs to know the name you choose for it, and using this macro was the most straightforward way.

There are some preconditions that the Arduino sketch must meet to use coroutines :

1. Declare a `Coroutines<N>` object, where `N` is the number of preallocated coroutines required; in other words, the number of coroutines you expect your program to "concurrently" run.
2. In your sketch's `loop()` function, call the `update()` function on that `Coroutines<N>` object.

Declared coroutines will not be started automatically. The sketch needs to start them with a function call :

```
// where "coroutines" is a Coroutine<N> instance,
// and "flashOnce" is the name of a declared coroutine
coroutines.start(flashOnce);
```

This fires the coroutine, which will begin in the next update. The return type of the function must be `void`, and it must be defined with the `COROUTINE_CONTEXT()` macro as only parameter.

## Other features

### Coroutine Locals

You may also use *coroutine locals*, which are variables local to the coroutine and whose state will be preserved after a yield and recovered when resuming :

```
void flashThrice(COROUTINE_CONTEXT(coroutine))
{
    COROUTINE_LOCAL(int, i);

    BEGIN_COROUTINE;

    for (i = 0; i < 3; i++)
    {
        analogWrite(5, 255);

        coroutine.wait(100);
        COROUTINE_YIELD;

        analogWrite(5, 0);

        coroutine.wait(50);
        COROUTINE_YIELD;
    }

    END_COROUTINE;
}
```

Notice that the `for(;;)` loop does not declare `i` since it already has been. However, its value is undefined, like any other variable, until it's first set. 

Since it's declared as `COROUTINE_LOCAL`, after returning from the `COROUTINE_YIELD`, its value will be restored to what it was prior to yielding.

`COROUTINE_LOCAL` declarations must be done before `BEGIN_COROUTINE`.

Coroutine locals live on the heap, so their size must be kept in check. Also, there is a maximum amount of them which defaults to 8, but can be tweaked in the header file. (see `CoroutineImpl::MaxLocals`)

### Looping Coroutines

Coroutines may also loop instead of evaluate once, using the `loop()` function :

```
void flashForever(COROUTINE_CONTEXT(coroutine))
{
    BEGIN_COROUTINE;

    analogWrite(5, 255);

    coroutine.wait(100);
    COROUTINE_YIELD;

    analogWrite(5, 0);

    coroutine.wait(50);
    COROUTINE_YIELD;

    coroutine.loop();

    END_COROUTINE;
}
```

If the `Coroutine::loop()` function is not called in one of its iterations, the loop stops and the coroutine will end its execution normally.

### External Manipulation

You can keep a reference to the coroutine object via the return value of `Coroutines<N>::start()`, but since these objects are recycled, one must be careful to only use the reference while the coroutine it initially referred to is still alive. One way to do this would be to declare the coroutine reference as a pointer in the
sketch's file-scope variables, and set it to `NULL` right before `COROUTINE_END`.

If the sketch holds a reference or a pointer to a `Coroutine&` object, it can manipulate its execution from the outside using these functions :

- `suspend()` will prevent any subsequent update to the coroutine
- `resume()` activates a suspended coroutine and allows it to execute in the next update
- `terminate()` makes the coroutine prematurely exit in the next update

The `suspend()` function may also be called from within a coroutine, which blocks its execution until `resume()` is called on it from the sketch.

### *finally* Block

To let a coroutine clean up after an external termination, you can use the `COROUTINE_FINALLY` macro like this :

```
void finallyExample(COROUTINE_CONTEXT(coroutine))
{
    BEGIN_COROUTINE;

    coroutine.wait(1000);
    COROUTINE_YIELD;

    Serial.println("Waited 1000ms");

    COROUTINE_FINALLY
    {
        Serial.println("Exiting...");
    }

    END_COROUTINE;
}
```

In this example, the "Exiting..." string will be printed whether the coroutine is externally terminated or if it finished execution normally, after waiting 1000ms. The "Waited 1000ms" string however will only be printed if the coroutine stays alive for more than 1000ms.

If used, the `COROUTINE_FINALLY` block must be placed before `END_COROUTINE`.

## Limitations

There is currently no way to return something from a coroutine or to pass a parameter to a coroutine. However, they have access to the sketch's file-scope variables,
which can be used for input and/or output.

## Logging

The library comes with debug-logging ability, which can be enabled by defining three macros :

- `trace(...)` is a redirect to `printf_P(...)` (or `printf(...)` if `P` does not go through `PSTR`)
- `assert(condition, ...)` should be defined as `while(cond) { trace(__VA_ARGS__); }`. Do not use the `<assert.h>` implementation from AVR Libc, it will make it very hard to debug issues! (Arduinos stop communicating entirely after an assertion fails)
- `P(string_literal)` is a shortcut to `PSTR` (if you want to hold strings in program memory) with a `\n` appended at the end

## Acknowledgements

This coroutine implementation is based on Simon Tatham's : http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html

...and is very much inspired by Unity's coroutines : http://docs.unity3d.com/ScriptReference/Coroutine.html

Thanks to Bryan McConkey and zerozshadow for sanity checking and suggestions, and to @yuriks on Twitter for pointing me to Simon Tatham's article. (plus a whole bunch of lovely twitter followers for your interest and comments!)
