import typer


app = typer.Typer()


@app.callback()
def callback():
    """
    Finite State Machine CLI
    """


@app.command()
def ping():
    """
    Simple cli ping
    """
    typer.echo('pong')
