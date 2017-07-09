'''
    OpenGL Window

    Examples:

        A Simple window::

            import GLWindow

            wnd = GLWindow.create_window()

            # Initialize Scene

            while wnd.update():
                # Render Scene
'''

# pylint: disable=using-constant-test

import os
from typing import Tuple, List

try:
    from . import glwnd

except ImportError:

    if os.environ.get('READTHEDOCS') == 'True':
        from .mock import glwnd

    else:
        _IMPORT_ERROR = '\n'.join([
            'No implementation found for GLWindow',
            'Are you sure the source code is compiled properly?',
            'Hint: python3 setup.py build_ext -b .',
        ])

        raise ImportError(_IMPORT_ERROR) from None

from . import keys


__all__ = [
    'Window', 'create_window', 'get_window', 'keys',
]

__version__ = '2.2.1'


class Window:
    '''
        Window
    '''

    def __init__(self):
        self.wnd = None
        raise Exception('Call create_window()')

    def clear(self, red=0.0, green=0.0, blue=0.0, alpha=0.0) -> None:
        '''
            Clear the window.
        '''

        self.wnd.clear(red, green, blue, alpha)

    def fullscreen(self) -> None:
        '''
            Set the window to fullscreen mode.
        '''

        self.wnd.fullscreen()

    def windowed(self, size) -> None:
        '''
            Set the window to windowed mode.
        '''

        width, height = size

        self.wnd.windowed(width, height)

    def update(self) -> None:
        '''
            Process window events.
            Swap buffers.
            Update key states.
        '''

        return self.wnd.update()

    def make_current(self) -> None:
        '''
            Activate the opengl context associated with the window.
        '''

        self.wnd.make_current()

    def keys(self, key) -> List[str]:
        '''
            list: The keys down.
        '''

        return [keys.KEY_NAME.get(i, '%02X' % i) for i in range(256) if self.key_down(i)]

    def key_pressed(self, key) -> bool:
        '''
            Is the key pressed?

            Args:
                key (int or str): The key or keycode.

            Returns:
                bool: True if the key is pressed, otherwise False
        '''

        return self.wnd.key_pressed(key)

    def key_down(self, key) -> bool:
        '''
            Is the key down?

            Args:
                key (int or str): The key or keycode.

            Returns:
                bool: True if the key is down, otherwise False
        '''

        return self.wnd.key_down(key)

    def key_released(self, key) -> bool:
        '''
            Is the key released?

            Args:
                key (int or str): The key or keycode.

            Returns:
                bool: True if the key is released, otherwise False
        '''

        return self.wnd.key_released(key)

    def key_up(self, key) -> bool:
        '''
            Is the key up?

            Args:
                key (int or str): The key or keycode.

            Returns:
                bool: True if the key is up, otherwise False
        '''

        return self.wnd.key_up(key)

    def set_icon(self, filename):
        '''
            Set the window icon.
            To set the small icon use :py:meth:`~Window.set_small_icon` instead.
        '''

        self.wnd.set_icon(filename)

    def set_small_icon(self, filename):
        '''
            Set the small window icon.
            To set the large icon use :py:meth:`~Window.set_icon` instead.
        '''

        self.wnd.set_small_icon(filename)

    def grab_mouse(self, grab):
        '''
            Lock the mouse to the center of the window.
            Use the :py:attr:`mouse` or :py:attr:`mouse_delta` to get the mouse position.
        '''

        self.wnd.grab_mouse(grab)

    @property
    def mouse(self) -> Tuple[int, int]:
        '''
            tuple: The mouse of the window.
        '''

        return self.wnd.mouse

    @property
    def mouse_delta(self) -> Tuple[int, int]:
        '''
            tuple: The mouse delta of the window.
        '''

        return self.wnd.mouse_delta

    @property
    def width(self) -> int:
        '''
            int: The width of the window.
        '''

        return self.wnd.width

    @property
    def height(self) -> int:
        '''
            int: The height of the window.
        '''

        return self.wnd.height

    @property
    def size(self) -> Tuple[int, int]:
        '''
            tuple: The size of the window.
        '''

        return (self.width, self.height)

    @property
    def ratio(self) -> float:
        '''
            float: The ratio of the window.
        '''

        return self.width / self.height

    @property
    def viewport(self) -> Tuple[int, int, int, int]:
        '''
            tuple: The viewport of the window.
        '''

        return self.wnd.viewport

    @property
    def title(self) -> str:
        '''
            str: The title of the window.
        '''

        raise NotImplementedError('only setter')

    @title.setter
    def title(self, value):
        self.wnd.title = value

    @property
    def vsync(self) -> bool:
        '''
            bool: The vsync flag.
        '''

        return self.wnd.vsync

    @vsync.setter
    def vsync(self, value):
        self.wnd.vsync = value

    @property
    def time(self) -> float:
        '''
            float: The elapsed time.
        '''

        return self.wnd.time

    @property
    def text_input(self) -> str:
        '''
            str: The text input.
        '''

        return self.wnd.text_input

    @property
    def debug_hotkeys(self) -> bool:
        '''
            bool: Debug hotkeys enable flag.
        '''

        return self.wnd.debug_hotkeys

    @debug_hotkeys.setter
    def debug_hotkeys(self, value):
        self.wnd.debug_hotkeys = value


def create_window(size=None, samples=16, *, fullscreen=False, title=None, threaded=True) -> Window:
    '''
        Create the main window.

        Args:
            size (tuple): The width and height of the window.
            samples (int): The number of samples.

        Keyword Args:
            fullscreen (bool): Fullscreen?
            title (bool): The title of the window.
            threaded (bool): Threaded?

        Returns:
            Window: The main window.
    '''

    if size is None:
        width, height = 1280, 720

    else:
        width, height = size

    if samples < 0 or (samples & (samples - 1)) != 0:
        raise Exception('Invalid number of samples: %d' % samples)

    window = Window.__new__(Window)
    window.wnd = glwnd.create_window(width, height, samples, fullscreen, title, threaded)
    return window


def get_window() -> Window:
    '''
        The main window.

        Returns:
            Window: The main window.
    '''

    return glwnd.get_window()
