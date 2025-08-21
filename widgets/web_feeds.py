from __future__ import annotations

"""Widget for displaying headlines from RSS/Atom feeds."""

import asyncio
import json
import os
import subprocess
from pathlib import Path
from typing import List, Tuple

import feedparser
import httpx
import yaml
from textual.widgets import Label, ListItem, ListView


class FeedListItem(ListItem):
    """List item storing a link to an article."""

    def __init__(self, title: str, link: str) -> None:
        super().__init__(Label(title))
        self.link = link


class WebFeeds(ListView):
    """Scrollable list view that displays headlines from web feeds."""

    BINDINGS = [
        ("o", "open_link", "Open link in $BROWSER"),
    ]

    _CONFIG_LOCATIONS = [
        Path(__file__).resolve().parent.parent / "feeds.yml",
        Path(__file__).resolve().parent.parent / "feeds.yaml",
        Path(__file__).resolve().parent.parent / "feeds.json",
    ]

    async def on_mount(self) -> None:
        """Load configuration and fetch initial feed items."""
        self.feed_urls = self._load_config()
        await self.refresh_feeds()
        # Refresh every 15 minutes
        self.set_interval(900, self.refresh_feeds)

    def _load_config(self) -> List[str]:
        """Load feed URLs from YAML or JSON configuration file."""
        for path in self._CONFIG_LOCATIONS:
            if path.exists():
                with path.open("r", encoding="utf-8") as config_file:
                    if path.suffix == ".json":
                        data = json.load(config_file)
                    else:
                        data = yaml.safe_load(config_file)
                feeds = data.get("feeds", [])
                return feeds if isinstance(feeds, list) else []
        return []

    async def refresh_feeds(self) -> None:
        """Fetch and display feed headlines."""
        self.clear()
        if not self.feed_urls:
            self.append(FeedListItem("No feeds configured", ""))
            return

        async with httpx.AsyncClient() as client:
            tasks = [client.get(url, timeout=10.0) for url in self.feed_urls]
            responses = await asyncio.gather(*tasks, return_exceptions=True)

        entries: List[Tuple[str, str]] = []
        for response in responses:
            if isinstance(response, Exception):
                continue
            parsed = feedparser.parse(response.text)
            for entry in parsed.entries:
                title = getattr(entry, "title", "Untitled")
                link = getattr(entry, "link", "")
                entries.append((title, link))

        for title, link in entries:
            self.append(FeedListItem(title, link))

    def action_open_link(self) -> None:
        """Open the highlighted feed link in the user's browser."""
        item = self.highlighted_child
        if item and getattr(item, "link", None):
            browser = os.environ.get("BROWSER", "xdg-open")
            subprocess.Popen([browser, item.link])
