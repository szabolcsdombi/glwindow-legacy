from . import GLWindow
from . import keys

class Window:
	'''
		Window
	'''

	def __init__(self, width = None, height = None, samples = 16, fullscreen = False, title = None):
		self.wnd = GLWindow.create_window(width, height, samples, fullscreen, title)


	def fullscreen(self):
		'''
			fullscreen()

			set the window to fullscreen mode
		'''

		self.wnd.fullscreen()


	def windowed(self, width, height):
		'''
			windowed(width, height)

			set the window to windowed mode
		'''

		self.wnd.windowed(width, height)


	def update(self):
		'''
			update()

			process window events
			swap buffers
			update key states
		'''

		return self.wnd.update()


	def make_current(self):
		'''
			make_current()

			activate the opengl context associated with the window
		'''

		self.wnd.make_current()


	def swap_buffers(self):
		'''
			make_current()

			manually swap buffers
		'''

		self.wnd.swap_buffers()


	def key_pressed(self, key):
		'''
			key_pressed(key)
		'''

		return self.wnd.key_pressed(key)


	def key_down(self, key):
		'''
			key_down(key)
		'''

		return self.wnd.key_down(key)


	def key_released(self, key):
		'''
			key_released(key)
		'''

		return self.wnd.key_released(key)


	def key_up(self, key):
		'''
			key_up(key)
		'''

		return self.wnd.key_up(key)


	def set_icon(self, filename):
		'''
			set_icon(filename)

			set the window icon
			to set the small icon use ::py:method:`~Window.set_small_icon` instead
		'''

		self.wnd.set_icon(filename)


	def set_small_icon(self, filename):
		'''
			set_small_icon(filename)

			set the small window icon
			to set the large icon use ::py:method:`~Window.set_icon` instead
		'''

		self.wnd.set_small_icon(filename)


	def grab_mouse(self, grab):
		'''
			grab_mouse(grab)

			lock the mouse to the center of the window
			use the mouse or mouse_delta to get the mouse position
		'''

		self.wnd.grab_mouse(grab)


	@property
	def mouse(self):
		'''
			mouse

			mouse of the window
		'''

		return self.wnd.mouse


	@property
	def mouse_delta(self):
		'''
			mouse_delta

			mouse_delta of the window
		'''

		return self.wnd.mouse_delta


	@property
	def size(self):
		'''
			size

			size of the window
		'''

		return self.wnd.size


	@property
	def viewport(self):
		'''
			viewport

			viewport of the window
		'''

		return self.wnd.viewport


	@property
	def title(self):
		'''
			title

			title of the window
		'''

		raise NotImplementedError()


	@title.setter
	def title(self, value):
		self.wnd.title = value


	@property
	def vsync(self):
		'''
			vsync
		'''

		return self.wnd.vsync


	@vsync.setter
	def vsync(self, value):
		self.wnd.vsync = value


	@property
	def time(self):
		'''
			time
		'''

		return self.wnd.time

	@property
	def text_input(self):
		'''
			text_input
		'''

		return self.wnd.text_input


def create_window(width = None, height = None, samples = 16, fullscreen = False, title = None):
	'''
		create_window(width, height, samples = 16, fullscreen = False, title = None)

		create the main window
	'''

	return Window(width, height, samples, fullscreen, title)


import warnings


def Init(debug = False):
	global window
	window = create_window()
	warnings.warn("deprecated", DeprecationWarning)

def Update():
	return window.update()

def KeyPressed(key):
	return window.key_pressed(key)

def KeyDown(key):
	return window.key_down(key)

def KeyReleased(key):
	return window.key_released(key)

def KeyUp(key):
	return window.key_up(key)

def GetMouse():
	return window.mouse + (0,)

def GetSize():
	return window.size

def GetTime():
	return window.time
