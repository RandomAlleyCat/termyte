# Termyte
Handheld computer terminal

## Prerequisites

- Python 3
- git
- curl (or PowerShell on Windows)

## Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/RandomAlleyCat/termyte.git
   cd termyte
   ```

2. Run the installer script to create a virtual environment and install the project's Python dependencies:

   ```bash
   curl -fsSL https://raw.githubusercontent.com/RandomAlleyCat/termyte/main/scripts/install.sh | bash
   # or
   ./scripts/install.sh
   ```

   On Windows PowerShell:

   ```powershell
   ./scripts/install.ps1
   ```

3. Activate the virtual environment:

   ```bash
   source .venv/bin/activate  # Linux/macOS
   # or
   .\.venv\Scripts\Activate.ps1  # Windows PowerShell
   ```

4. Launch the app:

   ```bash
   python app.py
   ```

## Dependencies

- [psutil](https://pypi.org/project/psutil/) – Used by the `SystemStats` widget to gather CPU, memory, and network information.
- [httpx](https://www.python-httpx.org/) – Fetches RSS/Atom feeds for the `WebFeeds` widget.
- [feedparser](https://pypi.org/project/feedparser/) – Parses feed data.
- [PyYAML](https://pypi.org/project/PyYAML/) – Reads feed URLs from the `feeds.yml` configuration file.
- [evdev](https://pypi.org/project/evdev/) – Captures scancodes from the Chatpad keyboard. Linux only; may be skipped on Windows.

## Web feed configuration

The `WebFeeds` widget loads its feed URLs from a `feeds.yml` file in the project root. The file should contain a list of feed URLs:

```yaml
feeds:
  - https://hnrss.org/frontpage
  - https://planetpython.org/rss20.xml
```

Each feed's headlines are rendered in a scrollable list. With a headline highlighted, press `o` to open the article using the command specified in the `$BROWSER` environment variable (falling back to `xdg-open`).

## Chatpad input

Termyte can use an attached Xbox Chatpad as a tiny hardware keyboard. When the device is plugged in it is detected automatically under `/dev/input` and its scancodes are translated to Textual key names. The default mappings live in `chatpad_keymap.yml` in the project root and may be edited to remap individual keys. Any changes take effect the next time the application starts.

## Roadmap

- **Weather widget** – display current conditions and forecasts.
- **Message board** – lightweight BBS for local chatter.
- **Media controls** – play/pause and volume control for system audio.
- **Plugin API** – allow third parties to develop and share their own widgets.

Contributions are welcome! Please open an issue or submit a pull request with ideas and improvements.

## Security notes

The `ShellWidget` spawns an interactive shell inside the application. Anything typed into this widget runs with the same permissions as the user launching Termyte, so the program should only be used in trusted environments.
