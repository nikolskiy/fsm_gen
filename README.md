# Description

C-code FSM generator from YAML specifications

# Requirements

You should have Python dependency manager called [Poetry](https://python-poetry.org/docs/) installed on your system.

# Install

Clone the project and from the root of the project run:

```bash
poetry install
```

Now you can activate the environment:

```bash
poetry shell
```

# Run fsm CLI

When the environment is activated you will have access to `fsm` CLI. Run

```bash
fsm --help
```

to see available options.

Here is an example how to generate C code from a YAML file:

```bash
fsm c-from docs/ex1.md generated/ex1
```

Now checkout `generated/ex1` directory for generated files.

In order to see generated `yaml` file from provided Markdown, use `--debug` option.

```bash
fsm c-from docs/ex1.md generated/ex1 --debug
```

This will generate parsed yaml file called `debug.yaml` in the destination directory
even if the parser failed to generate C code.


# Build and serve documentation

There is a documentation system based on [mkdocs](https://www.mkdocs.org/getting-started/).

You can start docs server locally by running:

```bash
mkdocs serve
```
