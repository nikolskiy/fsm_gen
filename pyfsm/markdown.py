import io


class Label:
    value: str | None

    def __init__(self, raw: str) -> None:
        self.value = raw.strip()

    def __repr__(self) -> str:
        return f'Label(value={self.value})'

    def __eq__(self, other):
        if isinstance(other, Label):
            return self.value == other.value

        return NotImplemented


class CodeBlock:
    labels: list[Label]
    code_lines: list[str]

    def __init__(self, labels) -> None:
        self.labels = self.parse_labels(labels)
        self.code_lines = []

    def parse_labels(self, line: str) -> list[Label]:
        # We don't need the new line chars at this point.
        line = line.strip().rstrip('/n/s')
        # Check that we actually got a line that marks a codeblock.
        assert line.startswith("```")
        # At this point we don't need codeblock markers anymore.
        line = line.lstrip("`").strip().strip('{}')

        # We need to convert text to the labels.
        return [Label(token) for token in line.split()]

    @property
    def code(self) -> str:
        return ''.join(self.code_lines)

    def find_any_label_from(self, given: list[str]) -> list[bool]:
        """
        Given a list of labels find if any of them can be applied
        to the code block.
        """
        matched = [any([Label(l) in self.labels]) for l in given]
        return matched

    def matches(
            self,
            include_all: list[str] = [],
            include_any: list[str] = [],
            exclude_any: list[str] = []
        ) -> bool:
        """
        Check if the labels in the codeblock match provided filters.
        """

        # If we found any labels that need to be excluded,
        # we report no match.
        if any(self.find_any_label_from(exclude_any)):
            return False

        # Report a match if any label in `include_any` were found.
        if any(self.find_any_label_from(include_any)):
            return True

        # Report a match if all labels in `include_all` were found.
        if include_all and all(self.find_any_label_from(include_all)):
            return True

        return False


class Blocks:
    blocks: list[CodeBlock]

    def __init__(self) -> None:
        self.blocks = []

    def add_code_from(self, text: io.TextIOWrapper) -> None:
        current_line = 0
        while line := text.readline():
            current_line += 1

            if not line.startswith("```"):
                continue

            code_block = CodeBlock(line)
            self.blocks.append(code_block)
            while True:
                code_line = text.readline()
                if not code_line:
                    raise RuntimeError("Reached the end of the file without closing the code block.")
                current_line += 1
                if code_line.startswith("```"):
                    break

                code_block.code_lines.append(code_line)

    def __getitem__(self, index: int):
        return self.blocks[index]

    def code(self, *args, **kwargs) -> str:
        result = ''.join(
            [block.code for block in self if block.matches(*args, **kwargs)]
        )
        return result
