/*
 * Copyright (c) 2008-2009, Thomas Jaeger <ThJaeger@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "prefdb.h"
#include "composite.h"
#include <gdkmm.h>
#include <glibmm/i18n.h>
#include <gdk/gdk.h>
#include <vector>
#include <memory>


double red, green, blue, alpha, width;
std::vector<Trace::Point> points;

Popup::Popup(int x1, int y1, int x2, int y2) : Gtk::Window(Gtk::WINDOW_POPUP), rect(x1, y1, x2-x1, y2-y1) {
	if (!is_composited())
		throw std::runtime_error(_("'composite' not available"));

	Glib::RefPtr<Gdk::Visual> visual = get_screen()->get_rgba_visual();
	gtk_widget_set_visual(Widget::gobj(), visual->gobj());
	gtk_widget_set_app_paintable (Widget::gobj(), TRUE);
	signal_draw().connect(sigc::mem_fun(*this, &Popup::on_draw));
	realize();
	move(x1, y1);
	resize(x2-x1, y2-y1);
	get_window()->input_shape_combine_region(Cairo::Region::create(), 0, 0);
	// tell compiz to leave this window the hell alone
	get_window()->set_type_hint(Gdk::WINDOW_TYPE_HINT_DESKTOP);
}

void Popup::invalidate(int x1, int y1, int x2, int y2) {
	if (get_mapped()) {
		Gdk::Rectangle inv(x1 - rect.get_x(), y1 - rect.get_y(), x2-x1, y2-y1);
		get_window()->invalidate_rect(inv, false);
	} else
		show();
}

Composite::Composite() {
#define N 128
	GdkRectangle work_area;
	gdk_monitor_get_workarea(gdk_display_get_primary_monitor(gdk_display_get_default()),
                             &work_area);
	int w = work_area.width;
	int h = work_area.height;
	GdkDisplay* dp = gdk_display_get_default();
	GdkMonitor* mon = gdk_display_get_primary_monitor(dp);
	scale_factor = gdk_monitor_get_scale_factor(mon);
	num_x = (work_area.width - 1)/N + 1;
	num_y = (work_area.height - 1)/N + 1;

	pieces.reserve(num_x * num_y);
	for (int i = 0; i < num_x; i++) {
		for(int j = 0; j < num_y; j++) {
			pieces.emplace_back(std::make_unique<Popup>(i*N,j*N,MIN((i+1)*N,w),MIN((j+1)*N,h)));
		}
	}
}

size_t Composite::get_pieces_index(size_t x, size_t y) {
	return x * num_y + y;
}

void Composite::draw(Point p, Point q) {
	if(scale_factor > 0) {
		p.x /= scale_factor;
		p.y /= scale_factor;
		q.x /= scale_factor;
		q.y /= scale_factor;
	}
	if (!points.size()) {
		points.push_back(p);
	}
	points.push_back(q);
	int x1 = (int)(p.x < q.x ? p.x : q.x);
	int x2 = (int)(p.x < q.x ? q.x : p.x);
	int y1 = (int)(p.y < q.y ? p.y : q.y);
	int y2 = (int)(p.y < q.y ? q.y : p.y);
	int bw = (int)(width/2.0) + 2;
	x1 -= bw; y1 -= bw;
	x2 += bw; y2 += bw;
	if (x1 < 0)
		x1 = 0;
	if (y1 < 0)
		y1 = 0;
	for (int i = x1/N; i<num_x && i<=x2/N; i++)
		for (int j = y1/N; j<num_y && j<=y2/N; j++)
			pieces[get_pieces_index(i, j)]->invalidate(x1, y1, x2, y2);
}

void Composite::start_() {
	RGBA rgba = prefs.color.get();
	red = rgba.color.get_red_p();
	green = rgba.color.get_green_p();
	blue = rgba.color.get_blue_p();
	alpha = ((double)rgba.alpha)/65535.0;
	width = prefs.trace_width.get();
}

void Popup::draw_line(Cairo::RefPtr<Cairo::Context> ctx) {
	if (!points.size())
		return;
	auto i = points.begin();
	ctx->move_to (i->x, i->y);
	for (; i != points.end(); i++)
		ctx->line_to (i->x, i->y);
	ctx->set_source_rgba((red+0.5)/2.0, (green+0.5)/2.0, (blue+0.5)/2.0, alpha/2.0);
	ctx->set_line_width(width+1.0);
	ctx->set_line_cap(Cairo::LINE_CAP_ROUND);
	ctx->set_line_join(Cairo::LINE_JOIN_ROUND);
	ctx->stroke_preserve();

	ctx->set_source_rgba(red, green, blue, alpha);
	ctx->set_line_width(width*0.7);
	ctx->stroke();

}

bool Popup::on_draw(const ::Cairo::RefPtr< ::Cairo::Context>& ctx) {
	ctx->set_antialias(Cairo::ANTIALIAS_DEFAULT);
	ctx->set_operator(Cairo::OPERATOR_SOURCE);
	ctx->set_source_rgba(0.0, 0.0, 0.0, 0.0);
	ctx->paint();

	ctx->translate(-rect.get_x(), -rect.get_y());
	draw_line(ctx);

	return false;
}

void Composite::end_() {
	points.clear();
	for (int i = 0; i < num_x; i++)
		for (int j = 0; j < num_y; j++)
			pieces[get_pieces_index(i, j)]->hide();
}

Composite::~Composite() {

}
