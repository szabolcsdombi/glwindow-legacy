GLWindow
========


.. automodule:: GLWindow

.. currentmodule:: GLWindow

.. autofunction:: GLWindow.create_window(width=None, height=None, samples=16, fullscreen=False, title=None, threaded=True) -> Window
.. autofunction:: GLWindow.get_window() -> Window

.. autoclass:: Window
	:members: mouse, mouse_delta, size, viewport, title, vsync, time, text_input

	.. automethod:: fullscreen()
	.. automethod:: windowed(width, height)
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
