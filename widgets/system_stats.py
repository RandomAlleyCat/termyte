from __future__ import annotations

import psutil
from rich.table import Table
from rich.text import Text
from textual.widget import Widget
from textual.reactive import reactive


class SystemStats(Widget):
    """Display basic system statistics.

    This widget shows CPU, memory, disk, and network usage. Metrics are
    refreshed every second using an asynchronous timer. Press ``v`` to
    toggle between a compact and a verbose table view.
    """

    cpu = reactive(0.0)
    memory = reactive(0.0)
    disk = reactive(0.0)
    net_up = reactive(0.0)
    net_down = reactive(0.0)
    compact = reactive(True)

    def on_mount(self) -> None:
        """Start the update timer when the widget is mounted."""
        self._net_prev = psutil.net_io_counters()
        self.set_interval(1.0, self._update_stats)
        self.focus()

    def _update_stats(self) -> None:
        """Refresh system metrics and trigger a re-render."""
        self.cpu = psutil.cpu_percent()
        self.memory = psutil.virtual_memory().percent
        self.disk = psutil.disk_usage("/").percent
        net = psutil.net_io_counters()
        self.net_up = (net.bytes_sent - self._net_prev.bytes_sent) / 1024
        self.net_down = (net.bytes_recv - self._net_prev.bytes_recv) / 1024
        self._net_prev = net
        self.refresh()

    def render(self) -> Text | Table:
        if self.compact:
            return Text(
                f"CPU {self.cpu:5.1f}% | RAM {self.memory:5.1f}% "
                f"| Disk {self.disk:5.1f}% | Net {self.net_up:5.1f}kB/s↑ {self.net_down:5.1f}kB/s↓"
            )
        table = Table.grid(padding=(0, 1))
        table.add_row("CPU", f"{self.cpu:5.1f}%")
        table.add_row("RAM", f"{self.memory:5.1f}%")
        table.add_row("Disk", f"{self.disk:5.1f}%")
        table.add_row("Net Up", f"{self.net_up:5.1f} kB/s")
        table.add_row("Net Down", f"{self.net_down:5.1f} kB/s")
        return table

    def action_toggle(self) -> None:
        """Toggle between compact and verbose render modes."""
        self.compact = not self.compact
        self.refresh()
