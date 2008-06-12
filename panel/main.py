import ibus
import gtk
import dbus
import dbus.mainloop.glib
import panel

class PanelApplication:
	def __init__ (self):
		self._dbusconn = dbus.connection.Connection (ibus.IBUS_ADDR)
		self._dbusconn.add_signal_receiver (self._disconnected_cb,
							"Disconnected",
							dbus_interface = dbus.LOCAL_IFACE)

		self._ibus = self._dbusconn.get_object (ibus.IBUS_NAME, ibus.IBUS_PATH)
		self._panel = panel.PanelProxy (self._dbusconn, "/org/freedesktop/IBus/Panel", self._ibus)

		self._ibus.RegisterPanel (self._panel, True)

	def run (self):
		gtk.main ()

	def _disconnected_cb (self):
		print "disconnected"
		gtk.main_quit ()


def main ():
	# gtk.settings_get_default ().props.gtk_theme_name = "/home/phuang/.themes/aud-Default/gtk-2.0/gtkrc"
	# gtk.rc_parse ("./themes/default/gtkrc")
	PanelApplication ().run ()

if __name__ == "__main__":
	dbus.mainloop.glib.DBusGMainLoop (set_as_default=True)
	main ()
