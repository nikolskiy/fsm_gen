import io


class CodeBlock:
    labels: list[str]
    code_lines: list[str]

    def __init__(self, labels) -> None:
        self.labels = self.parse_labels(labels)
        self.code_lines = []

    def parse_labels(self, line: str) -> list[str]:
        line = line.strip().rstrip('/n/s')
        assert line.startswith("```")
        line = line.lstrip("`").strip().strip('{}')
        labels = [l.strip() for l in line.split()]
        return labels

    @property
    def code(self) -> str:
        return ''.join(self.code_lines)

    def matches(self, labels: str) -> bool:
        # Hardcode `.exclude` label for now
        exclude = '.exclude' in self.labels
        return labels in self.labels and not exclude


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

    def code(self, labels: str) -> str:
        result = ''.join([b.code for b in self.blocks if b.matches(labels)])
        return result
