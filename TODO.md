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

## FSM parametrization

To reuse FSM specs between projects a good way of parametrization is needed.
Say we have a driver of a display, but in one project it is connected to one set of pins,
and in other project to a different set of pins. And, essentially, we want to reuse driver
without any modifications in specs, only by parametrization in upper code.
Parametrization may be done by passing an FSM, in which we will abstract pins, etc.
Here we have a question about FSM signatures (types) and subtyping relation.

## Common event queue

Make FSM specs parameterized by event queue, so all FSM can share the same event
queue.

It is very desirable in small embedded systems.

## Event processing optimization

FSM compiler can do:
1. Automatically decide when to make call from one event handler to
   another syncronous/asyncronous using call graph information.
2. Automatically introduce intermediate internal events to split
   big 'do' sections into smaller ones via extra async events.
   In this case we should be able to manage evets queue to put
   this events on top of queue.

## Event priorities

We may introduce event priorities and their inheritance during
chain of async event calls.

## Passing parameters to event closures

    closure = delay_display_time(hh, mm) -> closure()
    closure = delay_display_time(hh) -> closure(mm)

For c-code generation it requires generation a set of functions for event handler closure
generation  and possible some code preprocessing.

## To simplify things remove support of 'static' type of FSM

There are many complications in code generation for static type of FSM.
User can always statically allocate state variables and pass them around during
calls to FSM functions.

## Event handling issues

1. Direct calls from one event handler to others may lead to deadlocks or missing,
   improperly handled events

   Standard sequence of event handler execution:
   1. check fsm1.state_num == fsm1.state1 (assume true)
   2. execute fsm1 state1 exit_code
   3. execute fsm1 event hanlder actions ---+
   [ 3a. fsm1 state_num = fsm1.state2 ]     |
   [ 3b. execute fsm1 state2 enter_code ]   |
   4. fsm2_event call    <------------------+
   5. fsm2.state_num == fsm2.stateN (assume true)
   6. execute fsm2 stateN exit_code
   7. execute fsm2 event actions handler ----+
   [ 7a. fsm1 state_num = fsm1.state2 ]      |
   [ 7b. execute fsm1 state2 enter_code ]    |
   8. fsm1_event call     <------------------+
   9. !!! problem here: fsm1 did not update state_num yet and not yet executed state2 enter code
      i.e. fsm1 is inconsistent state!

Note 3a, 3b, 7a, 7b missed steps in event processing.
Syncronous calls between event handlers are very dangerous
(loops may be far far not obvious), so every
call between event handlers should go via event queue.

2. storing several events in queue may give unexpected results, when
   some events may be unhandled, because called fsm expects some other
   events between them.
   So, in transition actions, enter/exit state code it is strongly recommended
   to store only one event for particular called fsm.

3. Sequencing events handling and syncronization of fsms via callback events (see tm1637 driver code)
   may lead to unexpected exausting of delayed event storage:
   For instance 'done' event should have minimun size of storage for two 'done' delayed events.
   Because storage is freed after event handler execution and if it try to allocate storage
   for the event being processed it may observe storage shortage.
   For instance: delayed 'done' event is fetched from queue and being executed, and
   in the process of execution, it call to 'delay_done()' to get delayed self for passing
   as callback to other fsm event handler. So, in this case there should be 2 blocks of mem
   in delayed 'done' event storage.
   Early freeing of storage before event is processed is dangerous when we work with by ref, not
   by value.

## Memory footprint optimization

For small systems we may globally optimize a set of FSMs:
1. For event closures we may use small numbers, instead of pointers.
   In small embeded system it is quite common to have less than 256 cells for
   event closures, so we may use just indices in arrays.
2. Optimize event parameters storage: use unions, to share the same cell among
   different event closures, etc
3. Fine tune event parameters storage, event queue, etc for sizes, sharing parameters,
   give these knobs to user.

## Forbid delayed events that can be processed outside an event queue

Delayed events during execution, should only put corresponding event in the event queue, not
process event hanlder directly.

Callbacks from one FSM to other FSM only via event queue.

## Deactivate delayed events without execution

For instance, we have a 'menu' FSM and call into it with two delayed events 'accept' and 'cancel'.
When 'menu' FSM calls one callback event it should inactivate other event.