import importlib.metadata
import typer
import yaml
from pathlib import Path
from pyfsm import parser
from pyfsm.markdown import Blocks


app = typer.Typer()


@app.callback()
def callback():
    """
    Finite State Machine CLI
    """


@app.command()
def version():
    """
    Display current version
    """
    version = importlib.metadata.version("fsm-gen")
    print(f'fsm version {version}')


def read_file(src:Path) -> str:
    """
    Detect file format based on the file name suffix and apply appropriate loader.
    """
    extension = src.suffix

    if extension == '.yaml':
        # Read the entire file as is.
        with open(src) as f:
            text = f.read()
    elif extension == '.md':
        # Pick yaml appropriate yaml blocks of code.
        blocks = Blocks()
        with open(src, mode='r') as f:
            blocks.add_code_from(f)

        text = blocks.code('yaml')
    else:
        raise ValueError(f"Not supported file format: {extension}")

    return text


@app.command()
def c_from(src:Path, dest:Path, debug: bool = False):
    """
    Generate C header and source files from a file containing FSM description.
    """

    text = read_file(src)

    if debug:
        with open(Path(dest, 'debug.yaml'), "w") as f:
            f.write(text)

    fsm = yaml.safe_load(text)

    h, c = parser.generate(fsm)

    header = Path(dest, parser.get_header_file_name(fsm))
    source = Path(dest, parser.get_source_file_name(fsm))

    dest.mkdir(parents=True, exist_ok=True)
    with open(header, "w") as f:
        f.write(h)

    with open(source, "w") as f:
        f.write(c)
