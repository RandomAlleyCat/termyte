"""Chatpad input device integration for Termyte.

This module detects an attached Xbox Chatpad under ``/dev/input`` and forwards
its key presses to the Textual application.  Scancodes are translated to Textual
key strings using a configurable mapping loaded from ``chatpad_keymap.yml``.
"""

from __future__ import annotations

import asyncio
import contextlib
from pathlib import Path
from typing import Dict, Optional

try:  # pragma: no cover - optional dependency
    from evdev import InputDevice, ecodes, list_devices
except Exception:  # pragma: no cover - evdev not installed
    InputDevice = None  # type: ignore[assignment]

import yaml
from textual import events

# Default location of keymap configuration
KEYMAP_PATH = Path(__file__).with_name("chatpad_keymap.yml")


def load_keymap(path: Optional[Path] = None) -> Dict[int, str]:
    """Load scancode-to-key mapping from a YAML file.

    Args:
        path: Optional path to a YAML configuration file.  If omitted, the
            default ``chatpad_keymap.yml`` next to this module is used.

    Returns:
        Mapping of numeric scancode to Textual key string.
    """

    path = path or KEYMAP_PATH
    if not path.exists():
        return {}
    data = yaml.safe_load(path.read_text()) or {}
    mapping: Dict[int, str] = {}
    for code_str, key in data.items():
        mapping[int(str(code_str), 0)] = str(key)
    return mapping


def find_chatpad() -> Optional[InputDevice]:
    """Locate an attached Chatpad device."""

    if InputDevice is None:
        return None
    for dev_path in list_devices():
        device = InputDevice(dev_path)
        if "Chatpad" in device.name:
            return device
    return None


class ChatpadListener:
    """Background task that forwards Chatpad key presses to the app."""

    def __init__(self, app, keymap: Optional[Dict[int, str]] = None) -> None:
        self.app = app
        self.keymap = keymap or load_keymap()
        self.device = find_chatpad()
        self._task: Optional[asyncio.Task] = None

    async def start(self) -> None:
        """Start listening for Chatpad events."""

        if self.device is None:
            return
        self._task = asyncio.create_task(self._read_events())

    async def stop(self) -> None:
        """Stop listening for Chatpad events."""

        if self._task is not None:
            self._task.cancel()
            with contextlib.suppress(Exception):
                await self._task

    async def _read_events(self) -> None:
        assert self.device is not None
        async for event in self.device.async_read_loop():
            if event.type == ecodes.EV_KEY and event.value == 1:
                key = self.keymap.get(event.code)
                if key:
                    char = key if len(key) == 1 else None
                    self.app.post_message(events.Key(key, char))
