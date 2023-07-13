/* compile with  valac -c cellrenderertextish.vala --pkg gtk+-3.0 -C -H cellrenderertextish.h */

public class CellRendererTextish : Gtk.CellRendererText {
	public enum Mode { Text, Key, Popup, Combo }
        public new Mode mode;
	public string[] items;

	public signal void key_edited(string path, Gdk.ModifierType mods, uint code);
	public signal void combo_edited(string path, uint row);

	private Gtk.CellEditable? cell;

	public CellRendererTextish() {
		mode = Mode.Text;
		cell = null;
		items = null;
	}

	public CellRendererTextish.with_items(string[] items) {
		mode = Mode.Text;
		cell = null;
		this.items = items;
	}

	public override unowned Gtk.CellEditable? start_editing (Gdk.Event? event, Gtk.Widget widget, string path, Gdk.Rectangle background_area, Gdk.Rectangle cell_area, Gtk.CellRendererState flags) {
		cell = null;
		if (!editable)
			return cell;
		switch (mode) {
			case Mode.Text:
				cell = base.start_editing(event, widget, path, background_area, cell_area, flags);
				break;
			case Mode.Key:
				cell = new CellEditableAccel(this, path, widget);
				break;
			case Mode.Combo:
				cell = new CellEditableCombo(this, path, widget, items);
				break;
			case Mode.Popup:
				cell = new CellEditableDummy();
				break;
		}
		return cell;
	}
}

class CellEditableDummy : Gtk.EventBox, Gtk.CellEditable {
	public bool editing_canceled { get; set; }
	protected virtual void start_editing(Gdk.Event? event) {
		editing_done();
		remove_widget();
	}
}

class CellEditableAccel : Gtk.EventBox, Gtk.CellEditable {
	public bool editing_canceled { get; set; }
	new CellRendererTextish parent;
	new string path;
	static bool background_color_added;

	static int inverse_premultiplied_color(int color, int alpha) {
		if (alpha == 0) {
			return 0;
		}
		return (255 * color + alpha - 1) / alpha;
	}

	public CellEditableAccel(CellRendererTextish parent, string path, Gtk.Widget widget) {
		this.parent = parent;
		this.path = path;
		editing_done.connect(on_editing_done);
		Gtk.Label label = new Gtk.Label(_("Key combination..."));
		label.xalign = 0.0f;
		label.yalign = 0.5f;
		add(label);
		if(!background_color_added) {
			var screen = this.get_screen();
			var css_provider = new Gtk.CssProvider();
			var styleContext = widget.get_style_context();
			styleContext.save();
			styleContext.set_state(Gtk.StateFlags.SELECTED);
			var surface = new Cairo.ImageSurface(Cairo.Format.ARGB32, 1, 1);
			var context = new Cairo.Context(surface);
			styleContext.render_background(context, -50, -50, 100, 100);
			context.fill();
			surface.flush();
			styleContext.restore();

			var data = surface.get_data();
			int a = data[3];
			int r = data[2];
			int g = data[1];
			int b = data[0];
			var rgba = Gdk.RGBA() {
				alpha = a / 255f,
				red   = inverse_premultiplied_color(r, a) / 255f,
				green = inverse_premultiplied_color(g, a) / 255f,
				blue  = inverse_premultiplied_color(b, a) / 255f		
			};
			
			string css = ".cell_editable_accel_bg { background-color: " + rgba.to_string() + ";}";
			try {
				css_provider.load_from_data(css);
				Gtk.StyleContext.add_provider_for_screen(screen, css_provider, Gtk.STYLE_PROVIDER_PRIORITY_USER);
			} catch(Error e) {
				error("Cannot load CSS stylesheet: %s", e.message);
			}
			background_color_added = true;
		}

		
		get_style_context().add_class("cell_editable_accel_bg");
		label.get_style_context().add_class("cell_editable_accel_bg");
		show_all();
	}

	protected virtual void start_editing(Gdk.Event? event) {
		Gtk.grab_add(this);
		Gdk.Seat seat = Gdk.Display.get_default().get_default_seat();
		seat.grab(get_window(),
				  Gdk.SeatCapabilities.KEYBOARD,
				  false,
				  null,
				  event,
				  null);
/*
		Gdk.DeviceManager dm = get_window().get_display().get_device_manager();
		foreach (Gdk.Device dev in dm.list_devices(Gdk.DeviceType.SLAVE))
			Gtk.device_grab_add(this, dev, true);
*/
		key_press_event.connect(on_key);
	}

	bool on_key(Gdk.EventKey event) {
		if (event.is_modifier != 0)
			return true;
		switch (event.keyval) {
			case Gdk.Key.Super_L:
			case Gdk.Key.Super_R:
			case Gdk.Key.Hyper_L:
			case Gdk.Key.Hyper_R:
				return true;
		}
		Gdk.ModifierType mods = event.state & Gtk.accelerator_get_default_mod_mask();

		editing_done();
		remove_widget();

		parent.key_edited(path, mods, event.hardware_keycode);
		return true;
	}
	void on_editing_done() {
		Gtk.grab_remove(this);
		Gdk.Display.get_default().get_default_seat().ungrab();
/*
		Gdk.DeviceManager dm = get_window().get_display().get_device_manager();
		foreach (Gdk.Device dev in dm.list_devices(Gdk.DeviceType.SLAVE))
			Gtk.device_grab_remove(this, dev);
*/
	}
}


class CellEditableCombo : Gtk.ComboBoxText {
	new CellRendererTextish parent;
	new string path;

	public CellEditableCombo(CellRendererTextish parent, string path, Gtk.Widget widget, string[] items) {
		this.parent = parent;
		this.path = path;
		foreach (string item in items) {
			append_text(_(item));
		}
		changed.connect(() => parent.combo_edited(path, active));
	}
}
