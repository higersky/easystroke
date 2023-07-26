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
#ifndef __STROKEDB_H__
#define __STROKEDB_H__
#include <string>
#include <map>
#include <set>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <iostream>

#include "gesture.h"
#include "prefdb.h"

class Action;
class Command;
class SendKey;
class SendText;
class Scroll;
class Ignore;
class Button;
class Misc;
class Ranking;

typedef std::shared_ptr<Action> RAction;
typedef std::shared_ptr<Command> RCommand;
typedef std::shared_ptr<SendKey> RSendKey;
typedef std::shared_ptr<SendText> RSendText;
typedef std::shared_ptr<Scroll> RScroll;
typedef std::shared_ptr<Ignore> RIgnore;
typedef std::shared_ptr<Button> RButton;
typedef std::shared_ptr<Misc> RMisc;
typedef std::shared_ptr<Ranking> RRanking;

class Unique;

class Modifiers;
typedef std::shared_ptr<Modifiers> RModifiers;

bool mods_equal(RModifiers m1, RModifiers m2);

class Action {
	friend class boost::serialization::access;
	friend std::ostream& operator<<(std::ostream& output, const Action& c);
	template<class Archive> void serialize(Archive & ar, const unsigned int version);
public:
	virtual ~Action() {}
	virtual void run() {}
	virtual RModifiers prepare() { return RModifiers(); }
	virtual const Glib::ustring get_label() const = 0;
};

class Command : public Action {
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version);
public:
	std::string cmd;
	Command() {}
	Command(const std::string &c) : cmd(c) {}
	static RCommand create(const std::string &c) { return std::make_shared<Command>(c) ; }
	virtual void run();
	virtual const Glib::ustring get_label() const { return cmd; }
};

class ModAction : public Action {
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version);
protected:
	ModAction() {}
	Gdk::ModifierType mods;
	ModAction(Gdk::ModifierType mods_) : mods(mods_) {}
	virtual RModifiers prepare();
public:
	virtual const Glib::ustring get_label() const;
};

class SendKey : public ModAction {
	friend class boost::serialization::access;
	guint key;
	BOOST_SERIALIZATION_SPLIT_MEMBER()
	template<class Archive> void load(Archive & ar, const unsigned int version);
	template<class Archive> void save(Archive & ar, const unsigned int version) const;
public:
	SendKey() {}
	SendKey(guint key_, Gdk::ModifierType mods) :
		ModAction(mods), key(key_) {}
	static RSendKey create(guint key, Gdk::ModifierType mods) {
		return std::make_shared<SendKey>(key, mods);
	}

	virtual void run();
	virtual RModifiers prepare();
	virtual const Glib::ustring get_label() const;
};
BOOST_CLASS_VERSION(SendKey, 1)
#define IS_KEY(act) (act && dynamic_cast<SendKey *>(act.get()))

class SendText : public Action {
	friend class boost::serialization::access;
	Glib::ustring text;
	BOOST_SERIALIZATION_SPLIT_MEMBER()
	template<class Archive> void load(Archive & ar, const unsigned int version);
	template<class Archive> void save(Archive & ar, const unsigned int version) const;
public:
	SendText() {}
	SendText(Glib::ustring text_) : text(text_) {}
	static RSendText create(Glib::ustring text) { return std::make_shared<SendText>(text); }

	virtual void run();
	virtual const Glib::ustring get_label() const { return text; }
};

class Scroll : public ModAction {
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version);

public:
	Scroll() {}
	Scroll(Gdk::ModifierType mods) : ModAction(mods) {}
	static RScroll create(Gdk::ModifierType mods) { return std::make_shared<Scroll>(mods); }
	virtual const Glib::ustring get_label() const;
};
#define IS_SCROLL(act) (act && dynamic_cast<Scroll *>(act.get()))

class Ignore : public ModAction {
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version);
public:
	Ignore() {}
	Ignore(Gdk::ModifierType mods) : ModAction(mods) {}
	static RIgnore create(Gdk::ModifierType mods) { return std::make_shared<Ignore>(mods); }
	virtual const Glib::ustring get_label() const;
};
#define IS_IGNORE(act) (act && dynamic_cast<Ignore *>(act.get()))

class Button : public ModAction {
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version);
	guint button;
public:
	Button() {}
	Button(Gdk::ModifierType mods, guint button_) : ModAction(mods), button(button_) {}
	ButtonInfo get_button_info() const;
	static unsigned int get_button(RAction act) {
		if (!act)
			return 0;
		Button *b = dynamic_cast<Button *>(act.get());
		if (!b)
			return 0;
		return b->get_button_info().button;
	}
	static RButton create(Gdk::ModifierType mods, guint button_) { return std::make_shared<Button>(mods, button_); }
	virtual const Glib::ustring get_label() const;
	virtual void run();
};
#define IF_BUTTON(act, b) if (unsigned int b = Button::get_button(act))

class Misc : public Action {
	friend class boost::serialization::access;
public:
	enum Type { NONE, UNMINIMIZE, SHOWHIDE, DISABLE };
	Type type;
private:
	template<class Archive> void serialize(Archive & ar, const unsigned int version);
public:
	static const char *types[5];
	Misc() {}
	Misc(Type t) : type(t) {}
	virtual const Glib::ustring get_label() const;
	static RMisc create(Type t) { return std::make_shared<Misc>(t); }
	virtual void run();
};

class StrokeSet : public std::set<RStroke> {
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version);
};

// Internal use only
class Click : public Action {
	virtual const Glib::ustring get_label() const { return "Click"; }
};
#define IS_CLICK(act) (act && dynamic_cast<Click *>(act.get()))

class StrokeInfo {
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version);
public:
	StrokeInfo(RStroke s, RAction a) : action(a) { strokes.insert(s); }
	StrokeInfo() {}
	StrokeSet strokes;
	RAction action;
	std::string name;
};
typedef std::shared_ptr<StrokeInfo> RStrokeInfo;
BOOST_CLASS_VERSION(StrokeInfo, 1)

class Ranking {
	static bool show(RRanking r);
	int x, y;
public:
	RStroke stroke, best_stroke;
	RAction action;
	double score;
	std::string name;
	std::multimap<double, std::pair<std::string, RStroke> > r;
	static void queue_show(RRanking r, RTriple e);
};

class Unique {
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version);
public:
	int level;
	int i;
};

class ActionListDiff {
	friend class boost::serialization::access;
	friend class ActionDB;
	template<class Archive> void serialize(Archive & ar, const unsigned int version);
	ActionListDiff *parent;
	std::set<Unique *> deleted;
	std::map<Unique *, StrokeInfo> added;
	std::list<Unique *> order;
	std::list<ActionListDiff> children;

	void update_order() {
		int j = 0;
		for (std::list<Unique *>::iterator i = order.begin(); i != order.end(); i++, j++) {
			(*i)->level = level;
			(*i)->i = j;
		}
	}

	void fix_tree(bool rebuild_order) {
		if (rebuild_order)
			for (std::map<Unique *, StrokeInfo>::iterator i = added.begin(); i != added.end(); i++)
				if (!(parent && parent->contains(i->first)))
					order.push_back(i->first);
		update_order();
		for (std::list<ActionListDiff>::iterator i = children.begin(); i != children.end(); i++) {
			i->parent = this;
			i->level = level + 1;
			i->fix_tree(rebuild_order);
		}
	}
public:
	int level;
	bool app;
	std::string name;

	ActionListDiff() : parent(0), level(0), app(false) {}

	typedef std::list<ActionListDiff>::iterator iterator;
	iterator begin() { return children.begin(); }
	iterator end() { return children.end(); }

	RStrokeInfo get_info(Unique *id, bool *deleted = 0, bool *stroke = 0, bool *name = 0, bool *action = 0) const;
	int order_size() const { return order.size(); }
	int size_rec() const {
		int size = added.size();
		for (std::list<ActionListDiff>::const_iterator i = children.begin(); i != children.end(); i++)
			size += i->size_rec();
		return size;
	}
	bool resettable(Unique *id) const {
		return parent && (added.count(id) || deleted.count(id)) && parent->contains(id);
	}

	Unique *add(StrokeInfo &si, Unique *before = 0) {
		Unique *id = new Unique;
		added.insert(std::pair<Unique *, StrokeInfo>(id, si));
		id->level = level;
		id->i = order.size();
		if (before)
			order.insert(std::find(order.begin(), order.end(), before), id);
		else
			order.push_back(id);
		update_order();
		return id;
	}
	void set_action(Unique *id, RAction action) { added[id].action = action; }
	void set_strokes(Unique *id, StrokeSet strokes) { added[id].strokes = strokes; }
	void set_name(Unique *id, std::string name) { added[id].name = name; }
	bool contains(Unique *id) const {
		if (deleted.count(id))
			return false;
		if (added.count(id))
			return true;
		return parent && parent->contains(id);
	}
	bool remove(Unique *id) {
		bool really = !(parent && parent->contains(id));
		if (really) {
			added.erase(id);
			order.remove(id);
			update_order();
		} else
			deleted.insert(id);
		for (std::list<ActionListDiff>::iterator i = children.begin(); i != children.end(); i++)
			i->remove(id);
		return really;
	}
	void reset(Unique *id) {
		if (!parent)
			return;
		added.erase(id);
		deleted.erase(id);
	}
	void add_apps(std::map<std::string, ActionListDiff *> &apps) {
		if (app)
			apps[name] = this;
		for (std::list<ActionListDiff>::iterator i = children.begin(); i != children.end(); i++)
			i->add_apps(apps);
	}
	ActionListDiff *add_child(std::string name, bool app) {
		children.push_back(ActionListDiff());
		ActionListDiff *child = &(children.back());
		child->name = name;
		child->app = app;
		child->parent = this;
		child->level = level + 1;
		return child;
	}
	bool remove() {
		if (!parent)
			return false;
		for (std::list<ActionListDiff>::iterator i = parent->children.begin(); i != parent->children.end(); i++) {
			if (&*i == this) {
				parent->children.erase(i);
				return true;
			}

		}
		return false;
	}
	bool move(Unique *src, Unique *dest) {
		if (!src)
			return false;
		if (src == dest)
			return false;
		if (parent && parent->contains(src))
			return false;
		if (dest && parent && parent->contains(dest))
			return false;
		if (!added.count(src))
			return false;
		if (dest && !added.count(dest))
			return false;
		order.remove(src);
		order.insert(dest ? std::find(order.begin(), order.end(), dest) : order.end(), src);
		update_order();
		return true;
	}

	std::shared_ptr<std::map<Unique *, StrokeSet> > get_strokes() const;
	std::shared_ptr<std::set<Unique *> > get_ids(bool include_deleted) const;
	int count_actions() const {
		return (parent ? parent->count_actions() : 0) + order.size() - deleted.size();
	}
	void all_strokes(std::list<RStroke> &strokes) const;
	RAction handle(RStroke s, RRanking &r) const;
	// b1 is always reported as b2
	void handle_advanced(RStroke s, std::map<guint, RAction> &a, std::map<guint, RRanking> &r, int b1, int b2) const;

	~ActionListDiff();
};
BOOST_CLASS_VERSION(ActionListDiff, 1)

class ActionDB {
	friend class boost::serialization::access;
	friend class ActionDBWatcher;
	template<class Archive> void load(Archive & ar, const unsigned int version);
	template<class Archive> void save(Archive & ar, const unsigned int version) const;
	BOOST_SERIALIZATION_SPLIT_MEMBER()

public:
	std::map<std::string, ActionListDiff *> apps;
private:
	ActionListDiff root;
public:
	typedef std::map<Unique *, StrokeInfo>::const_iterator const_iterator;
	const const_iterator begin() const { return root.added.begin(); }
	const const_iterator end() const { return root.added.end(); }

	ActionListDiff *get_root() { return &root; }

	const ActionListDiff *get_action_list(std::string wm_class) const {
		std::map<std::string, ActionListDiff *>::const_iterator i = apps.find(wm_class);
		return i == apps.end() ? &root : i->second;
	}
	ActionDB();
};
BOOST_CLASS_VERSION(ActionDB, 3)

class ActionDBWatcher : public TimeoutWatcher {
	bool good_state;
public:
	ActionDBWatcher() : TimeoutWatcher(5000), good_state(true) {}
	void init();
	virtual void timeout();
};

extern ActionDB actions;
void update_actions();
#endif
