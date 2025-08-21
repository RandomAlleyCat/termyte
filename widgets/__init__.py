"""Widget implementations for Termyte."""

from .shell import ShellWidget
from .system_stats import SystemStats
from .web_feeds import WebFeeds

__all__ = ["SystemStats", "WebFeeds", "ShellWidget"]
