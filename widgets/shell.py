"""Shell widget for Termyte.

Attempts to use :class:`textual.widgets.Terminal` when available.  If the
Terminal widget is missing, a minimal PTY based implementation is provided as a
fallback.  The shell command launched defaults to ``/bin/bash`` but respects the
``$SHELL`` environment variable when set.
"""

from __future__ import annotations

import asyncio
import os
import pty
from typing import Optional

from rich.text import Text
from textual import events
from textual.widget import Widget

try:  # pragma: no cover - optional dependency
    from textual.widgets import Terminal as _Terminal
except Exception:  # Terminal widget not available
    _Terminal = None


if _Terminal is not None:
    class ShellWidget(_Terminal):
        """Shell widget built upon Textual's Terminal widget."""

        def __init__(self, shell: Optional[str] = None, **kwargs) -> None:
            super().__init__(shell or os.environ.get("SHELL", "/bin/bash"), **kwargs)
else:
    class ShellWidget(Widget):
        """Simple PTY-based shell widget.

        Spawns an interactive shell inside a pseudo-terminal and relays user
        input / output.  ANSI escape sequences are preserved so that shell
        colors fit the application's retro theme.
        """

        DEFAULT_CSS = """
        ShellWidget {
            background: black;
            color: #00ff00;
        }
        """

        def __init__(self, shell: Optional[str] = None, **kwargs) -> None:
            super().__init__(**kwargs)
            self.shell = shell or os.environ.get("SHELL", "/bin/bash")
            self._fd: Optional[int] = None
            self._buffer = Text()
            self._reader: Optional[asyncio.Task] = None

        async def on_mount(self) -> None:
            """Spawn the shell process when the widget is mounted."""
            pid, fd = pty.fork()
            if pid == 0:  # child process
                os.execvp(self.shell, [self.shell])
            self._fd = fd
            os.set_blocking(fd, False)
            self._reader = asyncio.create_task(self._read_pty())

        async def _read_pty(self) -> None:
            assert self._fd is not None
            while True:
                try:
                    data = os.read(self._fd, 1024)
                except BlockingIOError:
                    await asyncio.sleep(0.05)
                    continue
                if not data:
                    break
                self._buffer += Text.from_ansi(data.decode("utf-8", "ignore"))
                self.refresh()

        def render(self) -> Text:
            return self._buffer

        async def on_key(self, event: events.Key) -> None:  # pragma: no cover - runtime
            if self._fd is None:
                return
            key = event.key
            if key == "enter":
                os.write(self._fd, b"\n")
            elif key == "tab":
                os.write(self._fd, b"\t")
            elif key == "backspace":
                os.write(self._fd, b"\x7f")
            elif key == "ctrl+c":
                os.write(self._fd, b"\x03")
            elif len(key) == 1:
                os.write(self._fd, key.encode())
