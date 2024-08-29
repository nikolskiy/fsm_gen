# FSM example

Define a simple example.

## Meta information

Name is required.

```yaml
name: example1
```

## Type

Static type assumes only one copy of fsm per executable
intended use is creation on program startup and deletion
on program termination
state variables are hidden from external code
all functions work on this hidden state
i.e. passing of state is not assumed
only parameters for even handlers

```yaml title="Excluded"
type: static
```

dynamic type allow to have many fsm states
all functions take explicit state parameter
and user is responsible for managing data
for these states

```yaml
type: dynamic
```

## Code generation options

These options are not mandatory
by default names for files are: {name}.h and {name}.c
without any prolog/epilog sections

one may specify file names explicitly

header: fsm.h
source: fsm.c

ore even specify prolog/epilog sections
for extra types and functions

```yaml
header:
  file: fsm.h
  prolog: |
    #define something "123"
  epilog: |
    #define other_thing something
    // comment

source:
  file: fsm.c
  prolog: |
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
  epilog: |
    // to simplify testing
    int main() {
        example1_state_variables_type fsm;
        example1_init(&fsm);
        example1_tick_enqueue(&fsm);
        example1_tick_enqueue(&fsm);
        example1_process_queue(&fsm);
        example1_deinit(&fsm);
    }
```

## Data types

Move data types to separate spec and put here only include stmt
do not generate data type declarations in fsm code

data types: 'file'

```yaml
data types:
  - structure1:
     - field1: u8
     - field2: u16
  - structure2:
    - field1: structure1
```

Data types allocation scheme:
  - dynamic - uses ordinary `malloc`.
  - fixed - uses preallocated structures
  - management: using pointers, using indices `structure1_ptr`, `strucutre1_alloc()`, `void strucutre1_free(structure1_ptr)`
  - access:
    - set(structure2.field1.field2, value)
    - get(structure2.field1.field2)
  - garbage collection: rc and explicit gc

The main idea is to give simple and efficient memory
management for small embedded systems.

## External functions

Can be used in conditions it is responsibility of user
to guarantee that these functions are pure.

```yaml title="Excluded"
pure_functions:
  function1:
    result: type
    inputs:
      - param1: type
      - param2: type
```

<!-- Todo: clarify what this means. -->
Can be used only in code that has some effects:
  'do' actions
  'enter'/'exit'

```yaml title="Excluded"
functions:
  # if result is not specified, then function is treated as procedure
  function2:
    inputs:
      ....
```

## Events definitions

Events are central idea of an FSM.
All transitions (state variables modifications and action code execution)
are made only when some event happens.

Events may have parameters.

TODO: use only defined data types.

```yaml
events:
  event1:
    - p1:
        # defaults are useless now for c-language
        # but may be useful later for c++ code generation
        int: 0
    - p2: char
    - p3: int
  event2: # no parameters
  event3:
    # event3 should have the same signature to test merging transition handlers
    - p1: int
    - p2: char
    - p3: int
  tick:
```

## Delayed events

List of events that can be delayed.
What is a delayed event?
It is just a closure of the corresponding event handler {fn, data}.
A closure is created as an ordinary call to event handler,
but instead of handling event, it just returns data to process event
later elsewhere: `void* {fsm_name}_event_delayed(...params...)`.

This approach may be useful in fully reactive systems, when you want to
process external event in ISR as fast as possible in order to not miss other
interrupts. In this case you just create a closure of event handler in an ISR
and execute this handler later in main event loop.

If a closure cannot be created (maximum is reached) then NULL ptr is returned
and it is upper code responsibility to handle this situation.

```yaml
delayed:
  event1:
    max: 8 # max possible delayed events of this type
  event3:
    max: 4
  tick:
    max: 16
```

Give user an ability to explicitly specify merging of parameters.

```yaml title="Excluded"
delayed:
  - max: 8
    events: [event1, event2 ...]
  - max: 16
    events: [...]
  - max: 32
    events: *
```

<!-- TODO: Fix the explanation. It is not clear -->

Events are not handled immediately, but closures are placed in queue
then later user can call fsm_process_queue() function
to handle all queued events
call: void fsm_queue_event1(...params...)
queue mostly reuses delayed event structures, so
if you want to queue some event type, you should define it in delayed.

```yaml
queue: true
```

or create separate management for queue:

```yaml title="Excluded"
queue:
  max: 8
```


## How to handle unhandled events

Sometimes there is no transition from state for incoming event
or transition conditions is not met
So there is several ways to handle this case:
1. Consider this situation as exceptional and halt the program
2. Treat this situation as ordinary and just ignore event
3. Allow user to specify his own code for handling

Handler can be common for all events.
ignore and halt are self explanatory:

```yaml title="Excluded"
unhandled events: ignore
```

```yaml title="Excluded"
unhandled events: halt
```

```yaml title="Excluded"
unhandled events: |
  // user can place here specific code
  // for handling unhandled events
  // all state variables are accessible
  // and extra __state_name and __event_name strings are
  // passed to this code (for logging or debug purposes))
```

Also reaction for unhandled events may be
specialized depending of event
just specify a dict with events names with
corresponding reactions
'*' name is reserved as default reaction for other
events, which are not mentioned in dict

```yaml
unhandled events:
  event1: ignore
  event2: |
    // user code as for common handlers
  "*": |
    // 1234
```

## State variables definition

There is one extra hidden variable is added
to track current state

```yaml
state variables:
  count: int
  ticks: # with initial values
    char: 0
  edge:
    char: 0
```

## Optional Initialization & de-initialization

User may define specific actions on FSM creation/destroy
to handle some complex initialization of state variables
or making some side effects

```yaml
init: |
  // user specific code is called after setup
  // of initial values for state variables
  // i.e. state variables are already initialized

deinit: |
  // specific actions on fsm de-allocation/de-initialization
  // state variables are still accessible here
  // FSM data should not be de-allocated yet
```

## States definitions

Each state can have optional entry/exit actions
Enter actions are executed after transition actions and
exit actions are executed before transition actions

So if transition is switching FSM from state1 to state2 then
sequence is:
1. Execute state1 exit actions if any
2. Execute transition actions if any
3. Execute state2 enter actions if any
4. return control to event handler caller

```yaml
states:
  state1:
  state2:
  state3:
    enter: |
      // do some actions on enter state
      // state variables are accessible here
      // they are modified by transition actions
      // hidden state num variable are set to this state
      // event parameters are not accessible (obviously)
    exit: |
      // some actions on state exit
      // hidden state num variable points to this state
      // all state variables are accessible and untouched by
      // transition actions (transition actions were not executed yet)
```

Mandatory definition of initial state

```yaml
initial state: state3
```

## Transitions

The most interesting part of FSM specification

Transitions are organized as list, to keep linear order
Order implicitly defines priority: if several transitions are enabled,
then upper one (in this list) will be executed

transition conditions in 'if' sections are assumed pure functional, ie
without any side-effects (no ++X, Y--, calls to impure functions, etc)

I hope this definition id self explanatory.

```yaml
transitions:
  - from: state1
    to: state2
    when: event1
    if: count < 10
    do: |
      count++;
      ticks--;
      edge=23;
      delay_event3(1,2,3);
```

we may define several transitions in one definition
actually, next definition is shortcut for two:

```yaml title="Excluded"
    - from: state1
      ...
    and
    - from: state2
      ...
```

```yaml
  - from: [state1, state2]
    to: state2
    when: event1
    if: count < 10
    do: |
      count++;
      ticks--;
      edge=3;
      delay_tick();
```

We may omit 'to' in definition
in this case transition is treated as loop to the state itself
so no entry/exit state code are executed.

```yaml
  - from: state1
    when: event2
```

we may add loops to several states at once
here are two rules in action:
1. expanding 'from' and generating several definition, as was explained earlier
2. applying rule of absent 'to' field, as in previous transition

```yaml
  - from: [state1, state2]
    when: tick
    do: |
      ticks++;
```

we may add transition to all stated, just fully omitting 'from'
so next definition is adding self-loops on event 'tick'
(note absence of 'to' field) for all states

```yaml
  - when: tick
    do: |
      ticks+=5;
      printf("%d\n", ticks);
```

Here is the trickiest definition
it actually defines four transitions:

   1. state1->state3 on event1
   2. state1->state3 on event3
   3. state2->state3 on event1
   4. state2->state3 on event3

BUT NB!: `event1` and `event3` parameters must be exactly the same
because all these transitions effectively share the same 'do' section
we may loose requirements for definitions without 'do' section, but
it may lead to some mess, because of different semantic handling of
definitions depending on presence of 'do' section, which might be undesirable

```yaml
  - from: [state1, state2]
    to: state3
    when: [event1, event3]
```
