# Common codeblocks

This is a test fixture for common code blocks.

## Including codeblocks
It contains first block:

```yaml
one:
    type: 1
```

And the second part of that `block`:

```yaml
    part: 2
    label: 3
```

It also contains the second block:

```yaml
two:
    type: 2
    part: 3
    label: 4
```

## Excluding codeblocks
The same document contains another block that should be excluded. We will mark it with a `title="Excluded"`:

```yaml title="Excluded"
excluded:
    type: 3
    part: 4
    label: 5
```

When we extract those blocks we should get:

```yaml title="Result"
one:
    type: 1
    part: 2
    label: 3
two:
    type: 2
    part: 3
    label: 4
```
