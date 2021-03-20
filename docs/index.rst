glwindow
========

.. code:: sh

    pip install glwindow

- `Documentation <https://glnext.readthedocs.io/>`_
- `glnext on Github <https://github.com/glnext/glnext/>`_
- `glnext on PyPI <https://pypi.org/project/glnext/>`_

A simple window with **your** own main loop.

Methods
=======

.. py:method:: glwindow.window(size: tuple, title: str, visible: bool) -> Window

.. py:method:: glwindow.update()

.. py:class:: Window

.. py:method:: Window.key_pressed(key: str) -> bool

.. py:method:: Window.key_down(key: str) -> bool

.. py:method:: Window.key_released(key: str) -> bool

.. py:method:: Window.grab_mouse(grab: bool) -> bool

.. py:method:: Window.show()

.. py:method:: Window.hide()

.. py:attribute:: Window.visible
    :type: bool

.. py:attribute:: Window.mouse
    :type: tuple

.. py:attribute:: Window.size
    :type: tuple

.. py:attribute:: Window.handle
    :type: tuple

.. note::

    Properties changes only after :func:`glwindow.update` is called.
