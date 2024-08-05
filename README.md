# fsm_gen
C-code FSM generator from YAML specifications

# Initial requirements for the project
You should have Python dependency management called [Poetry](https://python-poetry.org/docs/) installed on your system.

# Install
From the root of the project

```bash
poetry install
```

Now you can activate the environment

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
fsm c-from-yaml docs/ex1.yaml generated/ex1
```

Now checkout `generated/ex1` directory for generated files.

# Build and serve documentation
There is a documentation system based on [mkdocs](https://www.mkdocs.org/getting-started/).

You can start docs server locally by running:

```bash
mkdocs serve
```
