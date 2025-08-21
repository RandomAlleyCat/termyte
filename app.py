"""Termyte Textual application.

This module defines the :class:`TermyteApp` which provides a retro styled
terminal interface designed for a small 5" 800×480 display.  The layout and
styling aim to emulate 1980s dial-up BBS terminals with green/amber text on a
black background and box drawing borders.
"""

from dataclasses import dataclass
from typing import Dict
import time

from textual import events
from textual.app import App, ComposeResult
from textual.widgets import Placeholder
from widgets.system_stats import SystemStats
from widgets.web_feeds import WebFeeds
from widgets.shell import ShellWidget
from chatpad import ChatpadListener


@dataclass
class WidgetConfig:
    """Configuration for future widgets.

    Each widget can be toggled on/off through the ``enabled`` flag.  Widgets are
    defined up-front so that application configuration can be extended in the
    future without altering the layout code.
    """

    enabled: bool = False


class TermyteApp(App):
    """Main Textual application for the Termyte handheld."""

    # Target display size (columns, rows) for a 5" 800×480 screen using an
    # 8×16 pixel terminal font, which yields an 80×24 character grid.
    TARGET_SIZE: tuple[int, int] = (80, 24)

    # Load styling from an external CSS file for easier tweaking.
    CSS_PATH = "app.css"

    # Key bindings for runtime actions.
    BINDINGS = [
        ("ctrl+t", "toggle_theme", "Toggle CRT theme"),
    ]

    theme: str = "green"

    # Placeholder configuration for upcoming widgets.  All are disabled by
    # default and will only be created when enabled.
    widget_config: Dict[str, WidgetConfig] = {
        "system_stats": WidgetConfig(),
        "web_feeds": WidgetConfig(),
        "shell": WidgetConfig(),
    }

    def compose(self) -> ComposeResult:
        """Compose layout, adding widgets only when enabled."""
        for name, cfg in self.widget_config.items():
            if cfg.enabled:
                if name == "system_stats":
                    yield SystemStats()
                elif name == "web_feeds":
                    yield WebFeeds()
                elif name == "shell":
                    yield ShellWidget()
                else:
                    yield Placeholder(id=name)

    async def on_mount(self) -> None:
        """Start background listeners once the application is mounted."""
        self._chatpad = ChatpadListener(self)
        await self._chatpad.start()
        self._apply_theme()

    async def on_shutdown(self) -> None:
        """Stop background listeners when the application exits."""

        if hasattr(self, "_chatpad"):
            await self._chatpad.stop()

    async def on_key(self, event: events.Key) -> None:
        """Handle global key bindings."""

        if event.key == "ctrl+c":
            await self.action_quit()

    def _apply_theme(self) -> None:
        """Apply the current CRT color theme."""
        self.screen.set_class(self.theme == "green", "theme-green")
        self.screen.set_class(self.theme == "amber", "theme-amber")

    def action_toggle_theme(self) -> None:
        """Switch between amber and green CRT themes."""
        self.theme = "amber" if self.theme == "green" else "green"
        self._apply_theme()


def run() -> None:
    """Run the Termyte Textual application with a retro banner."""
    banner = r"""
 _______                  __          __        
|_   __ \                [  |        [  |       
  | |__) |   .--.   .--./\_| |  .--.   | |--.   
  |  ___/  / .'`\ \/ /'`\ ]| |/ .'`\ \ | .-. |  
 _| |_    | \__. |\ \._// | || \__. | | | | |  
|_____|    '.__.'  '.__.'[___]'.__.' [___]|__] 
    """
    for line in banner.splitlines():
        print(line)
        time.sleep(0.05)
    time.sleep(0.5)
    TermyteApp().run()


if __name__ == "__main__":
    run()
