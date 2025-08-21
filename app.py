"""Termyte Textual application.

This module defines the :class:`TermyteApp` which provides a retro styled
terminal interface designed for a small 5" 800×480 display.  The layout and
styling aim to emulate 1980s dial-up BBS terminals with green/amber text on a
black background and box drawing borders.
"""

from dataclasses import dataclass
from typing import Dict

from textual.app import App, ComposeResult
from textual.widgets import Placeholder
from widgets.system_stats import SystemStats
from widgets.web_feeds import WebFeeds


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

    # Retro BBS inspired global styling.
    CSS = """
    Screen {
        background: black;
        color: #00ff00;               /* bright green foreground */
        font-family: monospace;       /* terminal friendly font */
        width: 80;                    /* fit 5" 800×480 display */
        height: 24;
    }

    * {
        border: heavy #ffbf00;        /* amber ANSI box-drawing borders */
    }
    """

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
                else:
                    yield Placeholder(id=name)


def run() -> None:
    """Run the Termyte Textual application."""
    TermyteApp().run()


if __name__ == "__main__":
    run()
