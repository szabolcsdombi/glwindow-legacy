GLWindow
========


.. automodule:: GLWindow

.. currentmodule:: GLWindow

.. autofunction:: GLWindow.create_window(size=None, samples=16, fullscreen=False, title=None, threaded=True) -> Window
.. autofunction:: GLWindow.get_window() -> Window

.. autoclass:: Window
	:members: mouse, mouse_delta, width, height, size, viewport, ratio, title, vsync, time, time_delta, text_input, debug_hotkeys, keys

	.. automethod:: fullscreen()
	.. automethod:: windowed(size)
	.. automethod:: update()
	.. automethod:: make_current()
	.. automethod:: key_pressed(key)
	.. automethod:: key_down(key)
	.. automethod:: key_released(key)
	.. automethod:: key_up(key)
	.. automethod:: set_icon(filename)
	.. automethod:: set_small_icon(filename)
	.. automethod:: grab_mouse(grab)


.. toctree::
   :maxdepth: 2
