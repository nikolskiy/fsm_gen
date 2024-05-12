## Absctraction from an output language

Possible solution here is using processing in two stages:
1. Yaml -> AST
2. AST -> output code

## Optimizations of output code

To lower dependency on compiler optimizations, some optimizations
can be done in AST:
1. Storing intermediate results of predicates:

    if ((state_num == 2 | state_num == 3) && Predicate1) {
      ...
    }
    if ((state_num == 2 | state_num == 3 | state_num == 4) && Predicate2) {
      ...
    }
    
=>
    
    int state_num_check_2_3 = (state_num == 2 | state_num == 3);
    if ( state_num_check_2_3 && Predicate1) {
      ...
    }
    int state_num_check_2_3_4 = state_num_check_2_3 | (state_num == 4);
    if (state_num_check_2_3_4 && Predicate2) {
      ...
    }
    

## Give the user an ability to fully control naming scheme on code generation stage

We should give an user full control of the naming in code generation, because it may be
very important for integration of the code into users project

## Give control of some types

State num variable type, for instance. Currently it defined as 8 bit int for fsm with
small number of states, but for perf issues it may be desirable to use 32 bit int.

So user should have a control over this moments.

## Better debug and diagnostic functionality

1. Optional names for transitions
2. Extra parameters to user handlers (for unhandled events for instance):
  - source code location
  - transition name
  - event name (handler can be common for all events)
  - etc

## Symbolic names for state constants

Currently only numbers are used, but for better source code readabily
we may give theese numbers names.

## User defined states enumeration

May be user want to have specific numbers for specific states?

## State number coding schema

What if for some reason user wants one-hot encoding (bit in word) instead
of numbers?

## Various light-weight checks

1. syntactic checks: state names, etc
2. semantic checks: dead-locks (no output transitions)
   conditions completeness, etc


## Introduce observers

Observer is functions over state variables. Only pure expressions are allowed.

It will help to make specification more readable, etc.

     observers:
       get_voltage:
         uint32_t: ...expression over state variables...

Observers can be used by external code to get some information about FSM

     export observers:
       - get_voltage

## Introduce predicates in specification

This will help (and force to some extent) to give meaningful names for transition
conditions, etc

     predicates:
       timeout: time > timeout_val
     export predicates:
       - timeout
     ...
     transitions:
       - from: state1
         to: state2
         when: button_pressed
         if: timeout
         do: |
           ...

## Control on inlining internal functions

    static __INLINE__ void fsm_state_state1_enter()

## Data types specifications

To abstract away language details we should develop our own
data types specification.

For instance:
1. integer datatypes can be used as in stdint: uint8_t, int8_t, uint32_t, etc
   (maybe whithout '_t' suffix)
2. structures/records

    records:
      - record1:
        - field1:
            uint8_t: 0
        - filed2: {uint32_t: 0xFF}

3. Initialization is mandatory for fields of primitive type like integers (
   or default to 0?), so transitively closing this initialization we will
   have fully initialized records

4. Arrays are predefined objects with buit-in iterators, etc:
   Only fixed len?

   arrays:
     - int8arr:
         type:
           int8_t: 0
         size: 16

   GCC-style array initialization?

5. Destruct binding/access ?
   {a = field1, b = field2[5], field3{ e=field5 ... } ... } = record2;
   c = a + b + e;

Questions:
1. By ref/by val passing complex types, like records/arrays?
   By value may be emulated via passing refs with read-only access mode.
2. Recursive complex types? In many cases it is not needed in FSMs, but
   when it is needed its absence is very hard/ugly to workaround.
3. Syntax?

## Event closures, queues, etc

How event closures, passing them around and enqueueing them, relate with
theory?

In some cases we may obvously show that it is an equvalent of rearranging of incoming
events. But what about other cases?

Postponing events (enqueue_self):

     - from: state1
       to: state2
       when: process_it
       if: resources < 10
       postpone:

     - from: state1:
       to: state2
       when: process_it
       do: |
         ...


This obviousle equv to changing events order and to extend other transitions
(state1->stae2:process_it) conditions by adding 'resources>=10'.
But it is not fully behav equv: what if resources >= 10 is not reached?
And rearranging of incoming events means that we alter reason->consequence
relation between an FSM and an external environment.
Esp. taking into account handling of unhandled events.
Without postoponing we may reach a halt of the system, but with postponing
we can avoid the halt.

Or even worse:

     - from: state1
       to: state2
       when: process_it
       if: resources < 10
       do: |
         ... alter state variables ...
         enqueue_self()

     - from: state1:
       to: state2
       when: process_it
       do: |
         ...

Here we not only postpone event processing, but also alter the state before it.

In general an ability to make and call closures of event processing raises many
difficult questions of relation between practice and theary.

In practice, closures, etc are very useful for reactive control systems.

## Fallthrough event handling

In practice it may be useful to share some common code between all branches:

    - when: tick
      fallthrough:
      do: |
        ++tick;

    - from: state1
      to: state2
      when: tick
      if: ticks > 5
      do: |
        ...

Pros: siplify sharing common code between transition actions in event handler.
Cons: breaks theory. What if no other conditions are met and we get into unhandled
event handler? Behaviours are not similar.

An almost equiv to the example above:

    - from: state1
      to: state2
      when: tick
      if: ticks > 5
      do: |
        ...

    - when: tick
      do: |
        ++tick;
        enqueue_self();

Which also have issues with theoretical semantics.

## Hierarchical states

    states:
      state1:
        enter: |
          ...
        states:
          substate1:
          substate2:
            enter: |
              ...
            exit: |
              ...

At first sight it is an equv to cartesian product of transitions with corresponding merging of actions and state enter/exit code.

## FSM composition

Is it possible to compose FSMs in predictable and consistent way?

One possible approach is to include one FSM into another like record fields of record type.
It is clear how to compose state variable records, but what to do with event handling?
Possible solution it to totally hide inner FSM, and all events to it can only be generated by
outer FSM.
I believe that in this case FSMs composition is equiv to somwhat altered outer FSM: we can consider
the composition as outer FSM with some extra transitions, updates of conditions and extensions to
actions in transitions/enter/exit and init/deinit.

Using this method of composition we can effectively share inner FSMs, just generate them as
'dynamic' type with explicitely exposed state variables.

## To simplify things remove support of 'static' type of FSM

There are many complications in code generation for static type of FSM.
User can always statically allocate state variables and pass them around during
calls to FSM functions.