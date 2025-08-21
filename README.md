# Termyte
Handheld computer terminal

## Dependencies

- [psutil](https://pypi.org/project/psutil/) – Used by the `SystemStats` widget to
  gather CPU, memory, and network information.
- [httpx](https://www.python-httpx.org/) – Fetches RSS/Atom feeds for the
  `WebFeeds` widget.
- [feedparser](https://pypi.org/project/feedparser/) – Parses feed data.
- [PyYAML](https://pypi.org/project/PyYAML/) – Reads feed URLs from the
  `feeds.yml` configuration file.

## Web feed configuration

The `WebFeeds` widget loads its feed URLs from a `feeds.yml` file in the project
root.  The file should contain a list of feed URLs:

```yaml
feeds:
  - https://hnrss.org/frontpage
  - https://planetpython.org/rss20.xml
```

Each feed's headlines are rendered in a scrollable list.  With a headline
highlighted, press `o` to open the article using the command specified in the
`$BROWSER` environment variable (falling back to `xdg-open`).
