import typer
import yaml
from pathlib import Path

from pyfsm import parser


app = typer.Typer()


@app.callback()
def callback():
    """
    Finite State Machine CLI
    """


@app.command()
def c_from_yaml(src:Path, dest:Path):
    """
    Generate C header and source files from FSM described in YAML format.
    """

    with open(src) as f:
        fsm = yaml.load(f, Loader=yaml.Loader)

    h, c = parser.generate(fsm)

    header = Path(dest, parser.get_header_file_name(fsm))
    source = Path(dest, parser.get_source_file_name(fsm))

    dest.mkdir(parents=True, exist_ok=True)
    with open(header, "w") as f:
        f.write(h)

    with open(source, "w") as f:
        f.write(c)
