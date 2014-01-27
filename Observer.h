#pragma once
#include "StdAfx.h"

enum EventType{
	EVENT_ALL,
	EVENT_WINDOW_CLOSE,

	EVENT_WINDOW_CUT,
	EVENT_WINDOW_COPY,
	EVENT_WINDOW_PASTE,
	EVENT_WINDOW_UNDO,

	EVENT_KEY_DOWN,
	EVENT_KEY_UP,

	EVENT_MOUSE_MOVE,
	EVENT_MOUSE_MOVE_HARDWARE,
	EVENT_MOUSE_BUTTON_UP,
	EVENT_MOUSE_BUTTON_DOWN,
	EVENT_MOUSE_BUTTON_DOUBLECLICK,
	EVENT_MOUSE_WHEEL,
};

class EventSubject;

class Event{
public:
	EventSubject* source;
	EventType type;
	Event(EventSubject* subject, EventType type);
};



class EventObserver{
private:
	std::vector<std::pair<EventSubject*,EventType>> subjects;
public:
	virtual void ReceiveEvent(const Event* Event)=0;

	void RegisterSubject(EventSubject* subject, EventType type);
	void UnregisterSubject(EventSubject* subject, EventType type);
	void UnregisterAllSubjects();

	~EventObserver();
};


class EventSubject{
private:
	std::vector<std::pair<EventObserver*,EventType>> observers;
protected:

	void BroadcastEvent(const Event* event);

public:
	void RegisterObserver(EventObserver* observer, EventType type);
	void UnregisterObserver(EventObserver* observer, EventType type);
	void UnregisterAllObservers();

	~EventSubject();
};



class KeyEvent:public Event{
public:
	unsigned char key;
	KeyEvent(EventSubject* source, EventType type, unsigned char key);
};

class MouseEvent:public Event{
public:
	int x, y;
	unsigned int flags;

	enum FLAG_CONSTS{
		CONTROL = MK_CONTROL,
		SHIFT = MK_SHIFT,

		BUTTON_LEFT = MK_LBUTTON,
		BUTTON_RIGHT = MK_RBUTTON,
		BUTTON_MIDDLE = MK_MBUTTON,
		BUTTON_4 = MK_XBUTTON1,
		BUTTON_5 = MK_XBUTTON2,
	};

	MouseEvent(EventSubject* source, EventType type, int x, int y, unsigned int flags);
};
