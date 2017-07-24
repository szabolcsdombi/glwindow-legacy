# Change Log

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]

### Changed

- `mouse` property returns the `mouse_delta` when `grab_mouse` is true.
- `mouse_delta` is deprecated.

### Fixed

- division by zero when height is zero.
- memory leaks.

## [2.3.0] - 2017-07-10

### Fixed

- duplicate keys in dictionary.

### Added

- `title` getter.
- `time_delta` attribute.

### Changed

- `keys` method to `keys` property.

## [2.2.1] - 2017-07-09

### Fixed

- Restore window default size.

## [2.2.0] - 2017-07-05

### Removed

- Smart sleep when vsync is off.

### Changed

- `width`, `height` arguments were replaced with a `size` argument.
- Window `debug_hotkeys` attribute.

## [2.1.2] - 2017-06-17

### Added

- `clear()` function.
- `ratio` attribute.
- `keys` attribute.

### Fixed

- key states are unchanged when the window lost focus.

## [2.1.1] - 2017-06-06

### Fixed

- Cursor visibility.

## [2.1.0] - 2017-05-30

### Added

- Threaded window handling.
- Getter for vsync attribute.
- `get_window()` function.

### Removed

- `swap_buffers()` method.

### Fixed

- Render hang when window was moving.

## [2.0.4] - 2017-05-22

### Added

- Sample program in **\_\_main\_\_.py**.
- Automatic viewport change when the Window is resized.
- Error checking for `create_window()`.

### Fixed

- Render hang when alt key pressed.
- Bypassing maximize using the menu.

## [2.0.3] - 2017-05-21

## [1.0.0] - 2016-08-29

[Unreleased]: https://github.com/cprogrammer1994/GLWindow/compare/2.3.0...master
[2.3.0]: https://github.com/cprogrammer1994/GLWindow/compare/2.2.1...2.3.0
[2.2.1]: https://github.com/cprogrammer1994/GLWindow/compare/2.2.0...2.2.1
[2.2.0]: https://github.com/cprogrammer1994/GLWindow/compare/2.1.2...2.2.0
[2.1.2]: https://github.com/cprogrammer1994/GLWindow/compare/2.1.1...2.1.2
[2.1.1]: https://github.com/cprogrammer1994/GLWindow/compare/2.1.0...2.1.1
[2.1.0]: https://github.com/cprogrammer1994/GLWindow/compare/2.0.4...2.1.0
[2.0.4]: https://github.com/cprogrammer1994/GLWindow/compare/2.0.3...2.0.4
[2.0.3]: https://github.com/cprogrammer1994/GLWindow/tree/2.0.3
[1.0.0]: https://github.com/cprogrammer1994/GLWindow/tree/1.0.0
