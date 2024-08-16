# Tasks
List of all completed or in progress tasks.
This list somewhat duplicates GitHub's "Issues" functionality but this way we
can preserve the history of built features across multiple independent repositories.

# Add versioning support
`fsm --version` should output the current version.
We need the feature for [change log](./change-log.md) support.

# Add MarkDown support
As of `v0.1.0` we can parse only yaml files.
The goal of supporting MD format is to encourage [literate programming](https://en.wikipedia.org/wiki/Literate_programming) style.
Instead of one large yaml file with lots of comments developers will be able to create
MarkDown file with chunks of yaml code:

```yaml
here-is-a-section:
    - with
    - some
    - features
```

This will make the entire system more readable.

```yaml
another-section:
    - with
    - other
    - features
```

The idea is borrowed from [K Framework](https://github.com/runtimeverification/k/tree/master/k-distribution/k-tutorial/1_basic/08_literate_programming).
