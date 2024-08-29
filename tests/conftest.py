import pytest
from pathlib import Path


@pytest.fixture
def project_root():
    root_path = Path(__file__).parent.parent.absolute()
    return root_path
