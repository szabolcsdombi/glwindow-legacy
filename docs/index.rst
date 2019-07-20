Welcome to glwindow's documentation!
====================================

Installation
------------

Install glwindow with :command:`pip`:

.. code-block:: sh

    $ pip install glwindow

Reference
---------

.. function:: glwindow.window(size=(width, height), title: str, glversion: int) -> Window

    The default `size` is ``(1280, 720)``.

    The default `title` is ``'glwindow'``.

    The default `glversion` is ``330`` (OpenGL 3.3 core).

.. method:: Window.update() -> bool

    Updates the window and the user inputs.
    Return ``True`` if the window is still open.
    Returns ``False`` if the window is closed.

.. method:: Window.key_pressed(key: str) -> bool

    Returns ``True`` if the key was pressed during the last window update.

.. method:: Window.key_released(key: str) -> bool

    Returns ``True`` if the key was released during the last window update.

.. method:: Window.key_up(key: str) -> bool

    Returns ``True`` if the key is down.

    The return value is ``True`` if :func:`Window.key_released` returned ``True``.

.. method:: Window.key_down(key: str) -> bool

    Returns ``True`` if the key is up.

    The return value is ``True`` if :func:`Window.key_pressed` returned ``True``.

.. method:: Window.grab_mouse(grab: bool)

    If `grab` is ``True`` the cursor is hidden and the mouse input is relative.

    If `grab` is ``False`` the delta mouse is reset to zero and the cursor is shown.

.. attribute:: Window.size:Tuple[int, int]

    The size of the window.

.. attribute:: Window.mouse:Tuple[int, int]

    The mouse coordinates relative to the upper left corner of the window.
    If the mouse was previously grabbed with :func:`Window.grab_mouse` this attribute contains the relative mouse movement.

.. attribute:: Window.text_input:str

    The unicode text inputed to the window during the last window update.

.. attribute:: Window.time:float

    The time is seconds since the window was created.

Examples
--------

.. rubric:: simple window

.. code-block:: python

    import glwindow

    wnd = glwindow.window((840, 480))

    while wnd.update():
        pass

.. rubric:: glwindow and moderngl

.. code-block:: python

    import glwindow
    import moderngl

    wnd = glwindow.window()
    ctx = moderngl.create_context()

    while wnd.update():
        ctx.clear(0.0, 0.0, 0.0)

.. rubric:: inputs

.. code-block:: python

    import glwindow

    wnd = glwindow.window()

    while wnd.update():
        wnd.grab_mouse(wnd.key_down('space'))
        print(wnd.mouse)

.. toctree::
    :maxdepth: 2
