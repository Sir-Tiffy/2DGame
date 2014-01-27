#include "StdAfx.h"
#include "Observer.h"

void EventSubject::RegisterObserver(EventObserver* observer, EventType type){
	observers.emplace_back(observer,type);
}

void EventSubject::UnregisterObserver(EventObserver* observer, EventType type){
	for (auto i = observers.begin(); i != observers.end(); ++i){
		if ((*i).first==observer && (*i).second == type){
			if ((i+1) != observers.end()) (*i) = observers.back();
			observers.pop_back();
			break;
		}
	}
}

void EventSubject::UnregisterAllObservers(){
	auto temp = observers;
	for (auto observer:temp){
		observer.first->UnregisterSubject(this,observer.second);
	}
}

void EventSubject::BroadcastEvent(const Event* event){
	auto temp = observers;
	for (auto observer:temp){
		if (observer.second == event->type || observer.second == EVENT_ALL) observer.first->ReceiveEvent(event);
	}
}

EventSubject::~EventSubject(){
	UnregisterAllObservers();
}



void EventObserver::RegisterSubject(EventSubject* subject, EventType type){
	subject->RegisterObserver(this, type);
	subjects.emplace_back(subject, type);
}

void EventObserver::UnregisterSubject(EventSubject* subject, EventType type){
	subject->UnregisterObserver(this, type);
	for (auto i = subjects.begin(); i != subjects.end(); ++i){
		if ((*i).first==subject && (*i).second == type){
			(*i) = subjects.back();
			subjects.pop_back();
			break;
		}
	}
}

void EventObserver::UnregisterAllSubjects(){
	for (auto subject:subjects){
		subject.first->UnregisterObserver(this,subject.second);
	}
}

EventObserver::~EventObserver(){
	UnregisterAllSubjects();
}



Event::Event(EventSubject* source, EventType type):source(source),type(type){}
KeyEvent::KeyEvent(EventSubject* source, EventType type, unsigned char key):Event(source,type),key(key){}
MouseEvent::MouseEvent(EventSubject* source, EventType type, int x, int y, unsigned int flags):Event(source,type),x(x),y(y),flags(flags){}