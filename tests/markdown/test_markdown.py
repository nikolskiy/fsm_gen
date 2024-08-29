import pytest
from pathlib import Path
from pyfsm.markdown import Blocks


@pytest.fixture
def file_with_code_blocks(project_root):
    fixture_path = Path(
        project_root,
        'docs/dev/tests/common_code_blocks.md'
    )
    with open(fixture_path, 'r') as file_object:
        yield file_object


def test_extract_code_blocks(file_with_code_blocks):
    blocks = Blocks()
    blocks.add_code_from(file_with_code_blocks)
    code = blocks.code(
        include_all=['yaml'],
        exclude=['title="Result"', 'title="Excluded"'],
    )

    answer = blocks.code(
        include_all=['yaml', 'title="Answer"']
    )

    assert code == answer
