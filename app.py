from textual.app import App, ComposeResult
from textual.widgets import Header, Footer, TabPane, TabbedContent

from widgets.system_stats import SystemStats


class TermyteApp(App):
    """Simple Textual application with a system stats tab."""

    BINDINGS = [("v", "toggle_stats", "Toggle stats view")]

    def compose(self) -> ComposeResult:
        yield Header()
        with TabbedContent():
            with TabPane("System Stats"):
                yield SystemStats(id="stats")
        yield Footer()

    def action_toggle_stats(self) -> None:
        self.query_one(SystemStats).action_toggle()


if __name__ == "__main__":
    app = TermyteApp()
    app.run()
