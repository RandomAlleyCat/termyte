from __future__ import annotations

"""System statistics widget using psutil."""

import psutil
from textual.widgets import TextLog


class SystemStats(TextLog):
    """A widget that displays real time CPU, memory and network stats."""

    def on_mount(self) -> None:
        """Set up a timer to refresh statistics."""
        self.update_stats()
        # Refresh every second
        self.set_interval(1.0, self.update_stats)

    def update_stats(self) -> None:
        """Gather system statistics and write them to the log."""
        cpu = psutil.cpu_percent()
        mem = psutil.virtual_memory().percent
        net = psutil.net_io_counters()

        # Clear previous stats and write new ones
        self.clear()
        self.write(f"CPU Usage: {cpu}%")
        self.write(f"Memory Usage: {mem}%")
        self.write(f"Bytes Sent: {net.bytes_sent}")
        self.write(f"Bytes Received: {net.bytes_recv}")
