#!/home/vasil/.nix-profile/bin/python

import yaml
import sys
from dataclasses import dataclass


# TODO:
# 0. option to generate events queue as separate code
#    queue:
#      name:
#      size:
#      header:
#      source:
#
#    or to reuse:
#    queue:
#      name:
#      header: <file>
# *1. Formatting of output code
# 2. Factorization of state num comparisons in event handlers
# 3. User control for state variable name/type and other namings
# 3a. Add optional transition names
# 3b. use constants for state nums
# 4. User control for states enumeration
# 5. User control for state num representation (one-hot, etc)
# 6. Check for deadlocks (hanging states)
# *7. User control for unhandled events
# 8. Check for completeness of conditions
# 9. conditions factorization
# 10. add inline code option, to do not generate separate
#     state enter/exit functions, transition action functions, but
#     inline corresponding code
# !11. enqueue_self() macro for transition 'do' block on delayed events
#     to postpone event handling
#     - from: state1
#       to: state2
#       when: process_it
#       if: resources < 10
#       do: |
#         enqueue_self()
#     - from: state1
#       to: state2
#       when: process_it
#       do: |
#         ....
#     due to priority, first transition actions code will always postpone processing
#     until resources are greater or equal than 10 
#     to do not screw semantic: only self-loops can be postponed, and there is should
#     not be 'do' actions. Then this case is equivalent to rearranging of encoming events
#     and slightly changing 'if' conditions of corresponding transitions.
# ?12. Predicates. 'if' section can contain only boolean formula over predicates
#     predicates:
#       timeout: time > timeout
#     export predicates:
#       - timeout
# ?13. Observers - functions over state, that can return some value
#     observers:
#       get_voltage:
#         uint32_t: ...expression over state variables...
#      observers can be used by external code to get some information about FSM
#     export observers:
#       - get_voltage
#  14. state variables initialization of complex types structs, arrays, etc.
#      type declaration with mandatory initialization?

if len(sys.argv) != 2:
    print("Usage: fsm_gen fsm.yaml")
    exit(0)

with open(sys.argv[1]) as f:
    fsm = yaml.load(f, Loader=yaml.Loader)

fsm['name'] = fsm.get('name', None)

if fsm['name'] is None:
    print('FSM name is not specified')
    exit(1)

header = fsm.get('header', dict())
source = fsm.get('source', dict())
header_text = ""
source_text = ""

header['file'] = header.get('file', fsm['name'] + ".h")
header['prolog'] = header.get('prolog', '')
header['epilog'] = header.get('epilog', '')

source['file'] = source.get('file', fsm['name'] + ".c")
source['prolog'] = source.get('prolog', '')
source['epilog'] = source.get('epilog', '')

fsm['header'] = header
fsm['source'] = source

fsm['type'] = fsm.get('type', 'static')

fsm['delayed'] = fsm.get('delayed', None)

fsm['unhandled events'] = fsm.get('unhandled events', 'halt')
if isinstance(fsm['unhandled events'], dict):
    fsm['unhandled events']['*'] = fsm['unhandled events'].get('*', 'halt')

#naming schema

fsm['name prefix'] = fsm.get('name prefix', f"{fsm['name']}_")
fsm['state parameter name'] = fsm.get('state parameter name', f"{fsm['name prefix']}_state")
fsm['state parameter type'] = fsm.get('state parameter type', f"{fsm['name prefix']}_state_variables" if fsm['type'] == 'static' else f"{fsm['name']}_state_variables")

fsm['state num variable name'] = fsm.get('state num variable name', f"{fsm['name prefix']}state_num")
fsm['state num variable type'] = fsm.get('state num variable type', "unsigned char" if len(fsm['states']) <= 255 else "unsigned int")
fsm['queue'] = fsm.get('queue', False)

if fsm['delayed'] is not None and fsm['queue']:
    fsm['___ev_queue'] = {
        'len': sum([c['max'] for _,c in fsm['delayed'].items()]) + 1, # +1 to account head/tail issue in circular buffer, i.e. we can use only X-1 cells
        'type': f"{fsm['name prefix']}_delayed_event_queue_type",
        'name': f"{fsm['name prefix']}_delayed_event_queue"
    }

# this is needed for work with delayed events and queue on microcontrollers
fsm['interrupts state type'] = fsm.get('interrupts state type', None)
fsm['disable interrupts'] = fsm.get('disable interrupts', '___disable_interrupts')
fsm['restore interrupts'] = fsm.get('restore interrupts', '___restore_interrupts')

def indent(text, level=1):
    for i in range(level):
        text = text.splitlines(True)
        if len(text) == 0:
            return ""
        text = "  ".join(text)
        if text[-1] != '\n':
            text += '\n'
        text = "  " + text
    return text

def param_name(p):
    return list(p)[0]

def param_type(p):
    t = list(p.values())[0]
    if isinstance(t, dict):
        return list(t)[0]
    return t

def param_value(p):
    t = list(p.values())[0]
    if isinstance(t, dict):
        return list(t.values())[0]
    return None

def to_c_signature_str(params):
    return ", ".join([f"{param_type(p)} {param_name(p)}" for p in params])

def is_equal_sigs(s1, s2):
    return to_c_signature_str(s1) == to_c_signature_str(s2)

fsm['events'] = {
    e: (params if isinstance(params, list) else [])
    for e, params in fsm['events'].items()
  }

fsm['states'] = {
    s: ({'enter': body.get('enter',None), 'exit': body.get('exit', None)} if isinstance(body, dict) else {'enter': None, 'exit':None})
    for s, body in fsm['states'].items()
  }

if fsm['state variables'] is None:
    fsm['state variables'] = dict()

# TODO: account user specified 'num' field

fsm['state_by_num'] = dict()

for idx, key in enumerate(fsm['states'].keys()):
    fsm['states'][key]['num'] = idx
    fsm['state_by_num'][idx] = key

fsm['delayed_event_params_name'] = \
    lambda ename: f"{fsm['name prefix']}_delayed_{ename}_params"

fsm['delayed_event_params_type'] = \
    lambda ename: f"{fsm['name prefix']}_delayed_{ename}_params_type"

fsm['delayed_event_handler_variable_name'] = \
    f"{fsm['name prefix']}_delayed_event_handler"

def gen_delayed_struct_typedef(fsm, e_name, params):
    text = f"typedef struct {fsm['delayed_event_params_type']( e_name)}" + " {\n"
    params = [f"{param_type(p)} {param_name(p)}" for p in params]
    if fsm['type'] == 'dynamic':
        params = [f"struct {state.ref_decl}"] + params
    params = [f"volatile void(*{fsm['delayed_event_handler_variable_name']})(void *)"] + params
    text += indent(";\n".join(params) + ";")
    text += "} " + f"{fsm['delayed_event_params_type']( e_name)};\n"
    return text

def get_delayed_events(fsm):
    if fsm['delayed'] is None:
        return []
    delayed = []
    for e, config in fsm['delayed'].items():
        params = fsm['events'][e]
        delayed.append((e, config,fsm['events'][e]))
    # events should be sorted for stable numbering scheme
    sorted(delayed, key=lambda x: x[0])
    return delayed

def get_delayed_event_params_accessor(fsm, e_name):
    return state.access(fsm['delayed_event_params_name']( e_name))

@dataclass
class Naming:
    inner_prefix: str
    outer_prefix: str
    def __init__(self, fsm):
        import random
        self.inner_prefix = fsm.get('inner name prefix', f"___{fsm['name']}__{random.randint(0,65536)}__inner_")
        self.outer_prefix = fsm.get('name prefix', f"{fsm['name']}_")
    def inner(self, name):
        return self.inner_prefix + name
    def outer(self, name):
        return self.outer_prefix + name

@dataclass
class Var:
    typ: str
    ref_typ: str
    base_name: str
    name: str
    decl: str
    ref_decl: str
    deref: str
    initval: str
    initcode: str
    struct_initval: str
    def __init__(self, naming_method, type_, name, initval = None, initcode = None):
        self.typ = type_
        self.ref_typ = self.typ + "*"
        self.base_name = name
        self.name = naming_method(name)
        self.initval = initval if initval is not None else f"(({self.typ})0)"
        self.struct_initval = f".{self.name}={self.initval}"
        self.initcode = initcode if initcode is not None else f"{self.name}={self.initval}"
        self.decl = self.typ + " " + self.name
        self.ref_decl = self.ref_typ + " " + self.name
        self.deref = "(*" + self.name + ")"
    def field(self, var):
        name = var.name if isinstance(var, Var) else var
        return self.name + "." + name
    def field_via_ref(self, var):
        name = var.name if isinstance(var, Var) else var
        return self.name + "->" + name

@dataclass
class Record(Var):
    typedef: str
    fields: list[Var]
    def __init__(self, name, variables: list[Var]):
        self.fields = variables
        super().__init__(lambda x: x, f"{name}_type", name)
        self.__gen_typedef()

    def __gen_typedef(self):
        self.initval = "{\n" + ",\n".join([f.struct_initval for f in self.fields]) + "\n}"
        self.initcode = ";\n".join([f"{self.name}.{v.name}={v.initval}"])
        self.struct_initval = f".{self.name}={self.initval}"
        self.typedef = f"""\
struct {self.typ};
typedef struct {self.typ} {{
{';'.join([f.decl for f in self.fields])}
}} {self.typ};
"""
    def append(self, f:Var):
        self.fields.append(f)
        self.__gen_typedef()

@dataclass
class StateVariables(Var):
    typedef: str
    def __init__(self, fsm, naming):
        self.naming = naming
        self.static = fsm['type'] == 'static'
        typ = (self.naming.inner if self.static else self.naming.outer)("state_variables_type")
        super().__init__(self.naming.inner, typ, "state_variables")
        self.parameter = None if self.static else self.ref_decl
        self.num = Var(self.naming.inner, "int", "state_num", f"{fsm['states'][fsm['initial state']]['num']}")
        self.variables = [
            Var(self.naming.inner, param_type({name: vt}), name, param_value({name: vt}))
            for name, vt in fsm['state variables'].items()
        ]
        self.variables.append(self.num)
        for v in self.variables:
            v.initcode = f"{self.access(v)} = {v.initval};"

    def access(self, var: Var):
        return self.field(var) if self.static else self.field_via_ref(var)

    def append(self, var: Var):
        self.variables.append(var)

    def __gen_typedef(self):
        pass
        # fsm_name = fsm['name']
        # initializers = [v.struct_initval for v in state.variables]
        # variables = [v.decl for v in state.variables]
        # account for delayed events
        #for e, config, params in get_delayed_events(fsm):
            #variables.append(f"{fsm['delayed_event_params_type'](e)} {fsm['delayed_event_params_name']( e)}[{config['max']}]")
        #delayed_events = get_delayed_events(fsm)
        #variables = ";\n".join(variables) + ";"
        #result = f"struct {state.typ};\n" if fsm['type'] == 'dynamic' else ""
        #for e, _, params in delayed_events:
            #result += f"{gen_delayed_struct_typedef(fsm, e, params)}"
        #result += state.queue.typedef
        #result += f"typedef struct {state.typ}" + " {\n" + indent(variables) + "} " + f"{state.typ};\n"
        #return result

@dataclass
class Function:
    result_type: str
    name: str
    params: list[Var]
    declaration: str
    definition: str | None
    def __init__(self, name, result_type, params, body = None):
        self.name = name
        self.result_type = result_type
        self.params = params
        self.__gen_declaration()
        self.definition = None
        if body is not None:
            self.body = body
            self.__gen_definition()
    def __gen_declaration(self):
        params = [p.decl for p in self.params]
        if isinstance(self.state, StateVariables):
            if not self.state.static:
                params = [self.state.ref_decl] + params
        params = ", ".join(params)
        self.declaration = f"{self.result_type} {self.name} ({params})"
    def __gen_definition(self):
        if isinstance(self.body, str):
            self.definition = self.declaration + "\n{\n" + indent(self.body) + "}\n"
    def state(self, state):
        self.state = state
        self.__gen_declaration()
    def body(self, body):
        self.body = body
        self.__gen_definition()
    def call(self, params):
        params = [v.name if isinstance(v, Var) else v for v in params]
        params = ", ".join(params)
        if isinstance(self.state, StateVariables):
            if not self.state.static:
                params = self.state.name + ", " + params
        return f"{self.name}({params})"

@dataclass
class Sync:
    istate: Var
    disable: str
    restore: str
    def __init__(self, naming_method = None, typ = None, disable = None, restore = None):
        self.disable = disable
        self.restore = restore
        if self.disable is not None:
            self.istate = Var(naming_method, typ, "interrupts_state")
    def under_disabled_interrupts(self, code):
        if self.disable is None:
            return code
        return f"""\
{self.istate.decl} = {self.disable}();
{code}
{self.restore}({self.istate.name});
"""

@dataclass
class EventQueue(Var):
    size: int
    typedef: str
    head: Var
    tail: Var
    handlers: Var
    access_head: str
    access_tail: str
    access_handlers: str
    enqueue_fun: Function
    def __init__(self, naming, size = None, sync = None):
        self.naming = naming
        self.sync = Sync()
        self.head = Var(naming.inner, 'volatile int', 'queue_head')
        self.tail = Var(naming.inner, 'volatile int', 'queue_tail')
        self.handlers = Var(naming.inner, 'void*', 'enent_handlers')
        self.size = size if size is not None else 16
        super().__init__(
            naming.outer,
            'event_queue_type',
            'event_queue'
        )
        self.access_head = self.field(self.head)
        self.access_tail = self.field(self.tail)
        self.access_handlers =  self.field(self.handlers)
        self.struct_initval = f".{self.name}={{.{self.head.name}=0, .{self.tail.name}=0}}"
        self.initcode = f"{self.access_head} = 0;\n{self.access_tail} = 0;"
        self.__gen_typedef()
        self.__gen_initializers()
        self.__gen_enqueue_fun()
        if self.size == 0:
            self.typedef = ""
            self.initcodes = ""
            self.initval = ""
            self.struct_initval = ""

    def __gen_typedef(self):
        self.typedef = f"""\
typedef struct {self.typ} {{
  {self.handlers.decl}[{self.size}];
  {self.head.decl};
  {self.tail.decl};
}} {self.typ};
"""
    def __gen_initializers(self):
        self.initcode = f"{self.access_head} = 0;\n{self.access_tail} = 0;"
        self.struct_initval = f".{self.name}={{.{self.head.name}=0, .{self.tail.name}=0}}"

    def state(self, state):
        self.state = state
        state.append(self)
        state.queue = self
        super().__init__(
             self.naming.inner,
             self.naming.inner('event_queue_type'),
            'event_queue'
        )
        self.access_head = self.state.access(self.field(self.head))
        self.access_tail = self.state.access(self.field(self.tail))
        self.access_handlers =  self.state.access(self.field(self.handlers))
        self.__gen_typedef()
        self.__gen_initializers()
        self.__gen_enqueue_fun()

    def set_sync(self, sync):
        self.sync = sync
        self.__gen_enqueue_fun()

    def __gen_enqueue_fun(self):
        if isinstance(self.state, StateVariables):
            self.enqueue_fun = Function(naming.inner("event_enqueue___"), "int", [Var(naming.inner, "void*", "delayed_event")])
            self.enqueue_fun.state(self.state)
        else:
            self.enqueue_fun = Function(naming.outer("event_enqueue"), "int", [Var(naming.inner, "void*", "delayed_event")])
        param = self.enqueue_fun.params[0]
        self.enqueue_fun.body(f"if ({param.name} == {param.initval}) return 0;\n" +
          self.sync.under_disabled_interrupts(f"""\
int tail = {self.access_tail};
int old_tail = tail;
++tail;
if(tail >= {self.size}) tail = 0;
int not_full = tail != {self.access_head};
if (not_full) {{
  {self.access_handlers}[old_tail] = {param.name};
  {self.access_tail} = tail;
}}""") + "return not_full;"
        )

@dataclass
class Event:
    base_name: str
    name: str
    params: list[Var]
    handler: Function
    unhandled_handler: Function
    def __init__(self, naming, state, event):
        self.base_name = event['name']
        self.state = state
        self.name = naming.outer(self.base_name)
        val = None
        iden = lambda x: x
        def param_to_var(param):
            name = list(param.keys())[0]
            typ = list(param.values())[0]
            val = None
            if isinstance(typ, dict):
                val = list(typ.values())[0]
                typ = list(typ.keys())[0]
            return Var(iden, typ, name, val)
        self.params = [param_to_var(p) for p in event['params']]
        self.handler = Function(self.name, "void", self.params)
        self.handler.state(state)
        self.unhandled_handler = Function(
            naming.inner(self.base_name)+"_unhandled_event_handler___",
            "static inline void",
            self.params + [Var(iden, "const char*", "__state_name"), Var(iden, "const char*", "__event_name")])
        self.unhandled_handler.state(state)
        body = event['unhandled']
        if event['unhandled'] == 'ignore':
            body = "// warning: ignoring unhandled event!"
        elif event['unhandled'] == 'halt':
            body = "// halt\nwhile(1){}"
        self.unhandled_handler.body(body)
    def gen_handler_definition(self, transitions):
        pass

@dataclass
class DelayedEvent:
    event: Event
    handler: Function
    def __init__(self, event: Event):
        self.params = event.params
        

@dataclass
class EventParamsStorage(Var):
    definition: str
    def __init__(self, naming, events):
        self.naming = naming
        self.events = events
        sorted(self.events, lambda e: e.base_name)
        self.size = max([int(e.amount) for e in events])
        name = "_".join(events)
        super().__init__(
            self.naming.outer,
            f"{self.name}_params_storage_type",
            f"{self.name}_params_storage"
        )
        self.__gen_definition()
    def __gen_definition(self):
        pass

class State:
    def __init__(self, fsm, s):
        pass

naming = Naming(fsm)
state = StateVariables(fsm, naming)
queue = EventQueue(naming,  sum([c['max'] for _,c in fsm['delayed'].items()]) + 1)
queue.state(state)
queue.set_sync(Sync(naming.inner, "uint32_t", "___disable_interrupts", "___restore_interrupts"))

def get_unhandled(fsm, e):
    ue = fsm['unhandled events']
    if isinstance(ue, str):
        return ue
    return ue.get(e, ue.get("*", "halt"))

events = [{'name': n, 'params': p if p is not None else [], 'unhandled': get_unhandled(fsm,n) }
          for n, p in fsm['events'].items()]

events = { e['name']: Event(naming, state, e) for e in events}

def gen_delayed_events_queue_initializer(fsm):
    delayed = get_delayed_events(fsm)
    if delayed == [] or not fsm['queue']:
        return ""
    if fsm['type'] == 'static':
        queue.struct_initval
    return queue.initcode

def gen_state_struct(fsm):
    fsm_name = fsm['name']
    initializers = [v.struct_initval for v in state.variables]
    variables = [v.decl for v in state.variables]
    # account for delayed events
    for e, config, params in get_delayed_events(fsm):
        variables.append(f"{fsm['delayed_event_params_type'](e)} {fsm['delayed_event_params_name']( e)}[{config['max']}]")
    delayed_events = get_delayed_events(fsm)
    variables = ";\n".join(variables) + ";"
    result = f"struct {state.typ};\n" if fsm['type'] == 'dynamic' else ""
    for e, _, params in delayed_events:
        result += f"{gen_delayed_struct_typedef(fsm, e, params)}"
    result += state.queue.typedef
    result += f"typedef struct {state.typ}" + " {\n" + indent(variables) + "} " + f"{state.typ};\n"
    return result

def get_state_variables_init_vals(fsm):
    initializers = [v.struct_initval for v in state.variables]
    return initializers

def get_delayed_struct_init_vals(fsm):
    initializers = dict()
    for e, config, params in get_delayed_events(fsm):
        initializers[f"{fsm['delayed_event_params_name'](e)}"]="{" + ", ".join([f"{{.{fsm['delayed_event_handler_variable_name']}=((void(*)(void*))0)}}" for i in range(int(config['max']))]) + "}" 
    return initializers

def gen_state_struct_initializers(fsm):
    inits = get_state_variables_init_vals(fsm)
    delayed_inits = get_delayed_struct_init_vals(fsm)
    if len(delayed_inits) > 0:
        inits += [ f".{name}={val}" for name, val in delayed_inits.items()]
    if fsm['queue'] and fsm['delayed'] is not None:
        inits += [queue.struct_initval]
    return "\n" + indent(",\n".join(inits))

def gen_header_prolog(fsm):
    fsm_name = fsm['name']
    header = fsm['header']
    text = f"#ifndef {fsm['name prefix']}_header__\n#define {fsm['name prefix']}_header__\n{header['prolog']}\n"
    if fsm['type'] == 'static':
        text += f"void {fsm_name}_init();\n"
        text += f"void {fsm_name}_deinit();\n"
    else:
        text += gen_state_struct(fsm);
        text += f"void {fsm_name}_init ({state.ref_typ});\n"
        text += f"void {fsm_name}_deinit ({state.ref_typ});\n"
    return text

def gen_state_names_function(fsm):
    text = f"static const char* {fsm['name prefix']}_get_state_name("
    text += "" if fsm['type'] == 'static' else f"const {state.ref_decl}"
    text += ")\n{\n"
    text += f"  switch({state.access(state.num)})" + " {\n"
    for name, s in fsm['states'].items():
        text += f"    case {s['num']}: return \"{name}\";\n"
    text += "    default: return \"__Unknown__\";\n  }\n"
    text += "}\n"
    return text

def gen_source_prolog(fsm):
    source = fsm['source']
    text = f"#include \"{header['file']}\"\n{source['prolog']}\n"
    if fsm['type'] == 'static':
        text += gen_state_struct(fsm);
        text += f"static {state.decl}" + " = { " + gen_state_struct_initializers(fsm) + "};\n"
        for var in state.variables:
            if var.name != state.num.name:
                text += f"#define {var.base_name} {state.access(var)}\n"
        delayed = get_delayed_events(fsm)
        for e, _, _ in delayed:
            text += f"#define delay_{e}(...) ({fsm['name']}_{e}_delayed (__VA_ARGS__))\n"
        if fsm['queue']:
            for e, _, _ in delayed:
                text += f"#define enqueue_{e}(...) {fsm['name']}_{e}_enqueue (__VA_ARGS__)\n"
    else:
        for var in state.variables:
            if var.name != state.num.name:
                text += f"#define {var.base_name} {state.access(var)}\n"
        delayed = get_delayed_events(fsm)
        for e, _, p in delayed:
            if p == []: 
                text += f"#define delay_{e}() ({fsm['name']}_{e}_delayed({state.name}))\n"
            else:
                text += f"#define delay_{e}(...) ({fsm['name']}_{e}_delayed({state.name}, __VA_ARGS__))\n"
        if fsm['queue']:
            for e, _, p in delayed:
                if p == []: 
                    text += f"#define enqueue_{e}() {fsm['name']}_{e}_enqueue({state.name})\n"
                else:
                    text += f"#define enqueue_{e}(...) {fsm['name']}_{e}_enqueue({state.name}, __VA_ARGS__)\n"
    return text

def gen_call_to_state_entry_exit(fsm, idx, kind):
    call = None
    name = fsm['state_by_num'][idx]
    body = fsm['states'][name]
    if body[kind] is not None:
        if fsm['type'] == 'static':
            call = f"{fsm['name prefix']}_state_{name}_{kind}()"
        else:
            call = f"{fsm['name prefix']}_state_{name}_{kind} ({state.name})"
    return call

def gen_exit_state_switch_case(fsm, state_nums, state_num = state.access(state.num)):
    exits = [(num, gen_call_to_state_entry_exit(fsm, num, 'exit')) for num in state_nums]
    exits = [exit for exit in exits if exit[1] is not None]
    exits_code = ""
    if len(exits) > 0:
        exits_code += f"switch({state_num})" + " {\n"
        for num, call in exits:
            exits_code += f"  case {num}: {call}; break;\n"
        exits_code += "  default: break;\n}\n"
    return exits_code

def gen_source_init_deinit(fsm):
    source = fsm['source']
    text = ""
    initial_state_enter_call = gen_call_to_state_entry_exit(fsm, int(state.num.initval), 'enter') or ""
    #TODO generate state exit calls on deinit
    if fsm['type'] == 'static':
        init = fsm.get('init','')
        if initial_state_enter_call != "":
            init += initial_state_enter_call + ";"
        text += f"void {fsm['name']}_init()\n" + "{\n"  + indent(init)  + "}\n";
        text += f"void {fsm['name']}_deinit()\n" + "{\n" + indent(gen_exit_state_switch_case(fsm, [s['num'] for s in fsm['states'].values()]) + fsm.get('deinit','')) + "}\n";
    else:
        inits = [v.initcode for v in state.variables]
        for ename, config, _ in get_delayed_events(fsm):
            inits.append(f"for(int i=0; i<{config['max']}; ++i) {state.access(fsm['delayed_event_params_name'](ename))}[i].{fsm['delayed_event_handler_variable_name']} = ((void(*)(void*))0);")
        if fsm['queue'] and fsm['delayed'] is not None:
            inits.append(gen_delayed_events_queue_initializer(fsm))
        inits = "\n".join(inits)
        text += f"void {fsm['name']}_init ({state.ref_decl})\n" + "{\n" + indent(inits + "\n" + initial_state_enter_call + ";\n"+ fsm.get('init','')) + "}\n";
        text += f"void {fsm['name']}_deinit ({state.ref_decl})\n" + "{\n" + indent(gen_exit_state_switch_case(fsm, [s['num'] for s in fsm['states'].values()]) + fsm.get('deinit','')) + "}\n";
    return text

def gen_source_unhandled_event_handlers(fsm):
    return "\n".join(
        [gen_state_names_function(fsm)] +
        [e.unhandled_handler.definition for e in events.values()])

def gen_event_handler_signature(fsm, e):
    s = fsm['events'][e]
    sig = []
    if fsm['type'] != 'static':
        sig = [f"{state.ref_decl}"]
    s = to_c_signature_str(s)
    if s != "":
        sig.append(s)
    sig = ", ".join(sig)
    return sig

def gen_event_handlers_definitions(fsm):
    text = ""
    for e, s in fsm['events'].items():
        text += f"void {fsm['name']}_{e} ({gen_event_handler_signature(fsm, e)});\n"
    return text

def gen_states_entries_and_exits(fsm):
    text = ""
    for name, body in fsm['states'].items():
        def gen_code(kind):
            code = ""
            if body[kind] is not None:
                if fsm['type'] == 'static':
                    code += f"static void {fsm['name prefix']}_state_{name}_{kind}()\n"
                else:
                    code += f"static void {fsm['name prefix']}_state_{name}_{kind} ({state.ref_decl})\n"
                code += "{\n" + indent(body[kind]) + "}\n"
            return code
        text += gen_code('enter')
        text += gen_code('exit')
    return text

def gen_call_to_transition_action(fsm, idx):
    tr = fsm['transitions'][idx]
    evt = tr['when']
    if isinstance(evt, list):
        evt = evt[0]
    params = []
    if fsm['type'] != 'static':
        params = [state.name]
    params += [param_name(p) for p in fsm['events'][evt]]
    params = ", ".join(params)
    return f"{fsm['name prefix']}_transition_actions_{idx} ({params})"

def gen_transition_actions(fsm):
    events = fsm['events']
    text = ""
    for idx, tr in enumerate(fsm['transitions']):
        evt = tr['when']
        if isinstance(evt, list):
            if not all([is_equal_sigs(events[evt[0]], events[e]) for e in evt]):
                print("Error: event signatures are not the same")
                for e in evt:
                    print({e: events[e]})
                exit(1)
            evt = evt[0]
        sig = []
        if fsm['type'] != 'static':
            sig = [state.ref_decl]
        s = to_c_signature_str(events[evt])
        if s != "":
            sig.append(s)
        sig = ", ".join(sig)
        if 'do' in tr:
            text += f"static void {fsm['name prefix']}_transition_actions_{idx} ({sig})\n"
            text += "{\n" + indent(tr['do']) + "}\n"
    return text

def gen_unhandled_event_handler(fsm, event_name):
    e = events[event_name]
    if fsm['type'] == 'static':
        return e.unhandled_handler.call(e.handler.params + [f"{fsm['name prefix']}_get_state_name()" , f"\"{event_name}\""]) + ";\n"
    else:
        return e.unhandled_handler.call(e.handler.params + [f"{fsm['name prefix']}_get_state_name({state.name})" , f"\"{event_name}\""]) + ";\n"

def gen_call_to_event_enqueue(fsm, e, params):
    params = [param_name(p) for p in params]
    if fsm['type'] == 'dynamic':
        params = [state.name] + params 
    return f"{fsm['name']}_{e}_enqueue (" + ", ".join([p for p in params]) + ")"

def gen_macro_enqueue_self(fsm, ename, params):
    if ename in fsm['delayed']:
        return f"#define enqueue_self() {gen_call_to_event_enqueue(fsm, ename, params)}"
    return ""

def gen_end_of_macro_enqueue_self(fsn, ename):
    if ename in fsm['delayed']:
        return f"#undef enqueue_self // for {ename}"
    return ""

def gen_event_handlers(fsm):
    fsm_name = fsm['name']
    events = fsm['events']
    text = ""

    state_num_var = state.access(state.num)
    state_num_var_local = state.num.name
    all_states = list(fsm['states'].keys())

    for evt_name, params in events.items():
        text += f"""\
void {fsm_name}_{evt_name} ({gen_event_handler_signature(fsm, evt_name)})
{{
  {gen_macro_enqueue_self(fsm, evt_name, params)}
  register {state.num.typ} {state_num_var_local} = {state_num_var};
"""

        unconditional_return = False

        for idx, tr in enumerate(fsm['transitions']):

            if unconditional_return:
                text += "  // NB: Unconditial return here, so next transitions are not procesed.\n"
                break

            evt = tr['when']
            if not isinstance(evt, list):
                evt = [evt]
            if not evt_name in evt:
                continue
            text += f"  // transition {idx}\n"
            from_ = tr.get('from', all_states)
            if not isinstance(from_, list):
                from_ = [from_]
            to = tr.get('to', None)
            self_loop_for_to = isinstance(from_, list) and to in from_

            if self_loop_for_to:
                from_.remove(to)

            states_nums = list({ (fsm['states'][state]['num']) for state in from_ })

            if len(states_nums) == len(all_states):
                # 'from' contains all states, so no check is needed
                check = tr.get('if','')
                text += "  // field 'from' contains all states, so no check for state is needed\n"
            else:
                check = [f"{state_num_var_local} == {num}" for num in states_nums]
                check = "  ||\n".join(check)
                check = "(\n" + indent(check) + ")"
                if 'if' in tr:
                    check += " &&\n(" + tr['if'] + ")"

            def make_if(cond, code):
                src = ""
                if cond != "":
                    src += f"if (\n{indent(cond)}) " + "{\n"
                src += indent(code)
                if cond != "":
                    src += "}\n"
                    src = indent(src)
                return src

            unconditional_return = check == ""

            if to is None:
                # self loop, no state enter, no state exit execution
                if 'do' in tr:
                    text += make_if(check, gen_call_to_transition_action(fsm, idx) + ";\nreturn;\n")
                else:
                    text += f"  // Warning: meaningless self loop transition withot 'do'\n"
            else:
                # self loop first as hottest path
                if self_loop_for_to:
                    text += "  // check self-loop first, considering it as hottest control-flow path\n"
                    check_self = f"( {state_num_var_local} == {fsm['states'][to]['num']} )"
                    if 'if' in tr:
                        check_self += " && (" + tr['if'] + ")"
                    if 'do' in tr:
                        text += f"  if ( {check_self} ) " + "{ " + gen_call_to_transition_action(fsm, idx) + "; return; }\n"

                # generate state entry calls
                entry = gen_call_to_state_entry_exit(fsm, fsm['states'][to]['num'], 'enter')

                exits_code = gen_exit_state_switch_case(fsm, states_nums, state_num_var_local)

                if 'do' not in tr and entry is None and exits_code == "" and not self_loop_for_to:
                    text += f"  // Warning: meaningless transition {from_} -> {to}\n"
                    text += f"  //          no transition actions, no enter state actions,\n"
                    text += f"  //          no exit state actions.\n"
                    text += f"  //          Only state num varible will be changed.\n"
                    text += f"  //          It might be an error in spec.\n"
                    src = f"{state_num_var} = {fsm['states'][to]['num']};\n"
                    src += "return;\n"
                    text += make_if(check, src);
                else:
                    src = exits_code
                    if 'do' in tr:
                        src += gen_call_to_transition_action(fsm, idx) + ";\n"
                    src += f"{state_num_var} = {fsm['states'][to]['num']};\n"
                    if entry is not None:
                        src += f"{entry};\n"
                    src += "return;\n"
                    text += make_if(check, src)

        if not unconditional_return:
            text += gen_unhandled_event_handler(fsm, evt_name)

        text += indent(gen_end_of_macro_enqueue_self(fsm, evt_name))

        text += "}\n"

    return text

def gen_delayed_event_handlers_definitions(fsm):
    delayed = get_delayed_events(fsm)
    if delayed == []:
        return ""
    text = ""
    for e, _, _ in delayed:
        text += f"void* {fsm['name']}_{e}_delayed ({gen_event_handler_signature(fsm, e)});\n"
    text += f"#define {fsm['name']}_process_delayed_event(E) (*((void(**)(void*))E))(E)\n"
    return text

def gen_delayed_event_handlers(fsm):
    delayed = get_delayed_events(fsm)
    if delayed == []:
        return ""
    # generate delayed handlers
    params_type = fsm['delayed_event_params_type']( delayed[0][0])
    params_name = f"{fsm['name prefix']}_delayed_event_params"
    text = ""
    if fsm['interrupts state type'] is None:
        fsm['interrupts state type'] = "unsigned int"
        text += """\
#define ___disable_interrupts() 0
#define ___restore_interrupts(...)
"""
    def gen_call_to_event_handler(fsm, e, params):
        params = [param_name(p) for p in params]
        if fsm['type'] == 'dynamic':
            params = [state.name] + params 
        call = f"{fsm['name']}_{e} (\n" + \
            indent(",\n".join([f"(({fsm['delayed_event_params_type'](e)}*){params_name})->{p}" for p in params])) + \
            ");\n" + f"(({fsm['delayed_event_params_type'](e)}*){params_name})->{fsm['delayed_event_handler_variable_name']} = (void(*)(void*))0;"
        return call

    def gen_delayed_struct_alloc_and_assignment(fsm, ename, config, params):
        params = [param_name(p) for p in params]
        array_len = config['max']
        code = f"""\
  {fsm['interrupts state type']} {fsm['name prefix']}_interrupts_state = {fsm['disable interrupts']}();
  for(int idx=0; idx < {array_len}; ++idx) {{
    if (((void*){get_delayed_event_params_accessor(fsm, ename)}[idx].{fsm['delayed_event_handler_variable_name']}) == (void*)0) {{
"""
        if fsm['type'] == 'dynamic':
            params = [f"{state.name}"] + params 
        code += indent(";\n".join([f"{get_delayed_event_params_accessor(fsm, ename)}[idx].{p} = {p}" for p in params]) + ";\n", 3)
        code +=f"""\
      {get_delayed_event_params_accessor(fsm, ename)}[idx].{fsm['delayed_event_handler_variable_name']}={fsm['name']}_{ename}_process_delayed;
      {fsm['restore interrupts']}({fsm['name prefix']}_interrupts_state);
      return (void*)(&(({get_delayed_event_params_accessor(fsm, ename)})[idx]));
    }}
  }}
  {fsm['restore interrupts']}({fsm['name prefix']}_interrupts_state);\
"""
        return code

    for idx, (ename, config, params) in enumerate(delayed):
        text += f"""\
static void {fsm['name']}_{ename}_process_delayed (void* {params_name})
{{
{indent(gen_call_to_event_handler(fsm, ename, params))}\
}}
void* {fsm['name']}_{ename}_delayed ({gen_event_handler_signature(fsm, ename)})
{{
{gen_delayed_struct_alloc_and_assignment(fsm, ename, config, params)}
  return (void *)0;
}}
"""
    return text

def gen_queued_event_handlers_definitions(fsm):
    delayed = get_delayed_events(fsm)
    if delayed == [] or not fsm['queue']:
        return ""
    text = ""
    for e, _, _ in delayed:
        text += f"void {fsm['name']}_{e}_enqueue ({gen_event_handler_signature(fsm, e)});\n"
    if fsm['type'] == 'static':
        text += f"void {fsm['name']}_process_queue();\n"
    else:
        text += f"void {fsm['name']}_process_queue({state.ref_decl});\n"
    return text

def gen_queued_event_handlers(fsm):
    delayed = get_delayed_events(fsm)
    if delayed == [] or not fsm['queue']:
        return ""
    text = "static " + queue.enqueue_fun.definition + "\n"

    def gen_call_to_event_delayed(fsm, e, params):
        params = [param_name(p) for p in params]
        if fsm['type'] == 'dynamic':
            params = [state.name] + params 
        return f"{fsm['name']}_{e}_delayed (" + ", ".join([p for p in params]) + ")"

    for ename, cfg, params in delayed:
        text += f"""\
void {fsm['name']}_{ename}_enqueue ({gen_event_handler_signature(fsm, ename)})
{{
  if ({queue.enqueue_fun.call([gen_call_to_event_delayed(fsm, ename, params)])}) return;
{gen_unhandled_event_handler(fsm, ename)}\
}}
"""
    if fsm['type'] == 'static':
        text += f"void {fsm['name']}_process_queue()"
    else:
        text += f"void {fsm['name']}_process_queue({state.ref_decl})"
    text +=f"""
{{
  if ({queue.access_head} != {queue.access_tail}) {{
    int head = {queue.access_head};
    int tail = {queue.access_tail};
    while(head != tail) {{
      {fsm['name']}_process_delayed_event({queue.access_handlers}[head]);
      ++head;
      if(head >= {queue.size}) head = 0;
    }}
    {queue.access_head} = head;
  }}
}}
"""
    return text

def gen_header_epilog(fsm):
    return f"\n{fsm['header']['epilog']}\n#endif // {fsm['name prefix']}_header__\n"

def gen_source_epilog(fsm):
    text = ""
    for var_name in fsm['state variables'].keys():
        text += f"#undef {var_name}\n"
    text += f"\n{fsm['source']['epilog']}\n"
    return text

header_text += gen_header_prolog(fsm)
header_text += gen_event_handlers_definitions(fsm)
header_text += gen_delayed_event_handlers_definitions(fsm)
header_text += gen_queued_event_handlers_definitions(fsm)
header_text += gen_header_epilog(fsm)

source_text += gen_source_prolog(fsm)
source_text += gen_states_entries_and_exits(fsm)
source_text += gen_source_init_deinit(fsm)
source_text += gen_source_unhandled_event_handlers(fsm)
source_text += gen_transition_actions(fsm)
source_text += gen_event_handlers(fsm)
source_text += gen_delayed_event_handlers(fsm)
source_text += gen_queued_event_handlers(fsm)
source_text += gen_source_epilog(fsm)

with open(header['file'], "w") as f:
    f.write(header_text) 

with open(source['file'], "w") as f:
    f.write(source_text) 
