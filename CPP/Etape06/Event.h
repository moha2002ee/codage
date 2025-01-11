
#ifndef EVENT_H
#define EVENT_H
#include <iostream>
#include <string.h>
using namespace std;
#include "Timing.h"
namespace planning
{

    class Event
    {
    private:
        int code;
        char* title;
        Timing* timing;
    public:
        static int currentCode;
        Event();
        Event(int c, const char* t);
        Event(const Event& e);
        ~Event();

        void setCode(int c);
        void setTitle(const char* t);
        void setTiming(const Timing& tm);

        Timing getTiming() const;
        int getCode() const;
        const char* getTitle() const;

        void display() const;
                friend std::ostream& operator<<(std::ostream &, const Event &);
        friend std::istream& operator>>(std::istream &, Event &);

    };
}
#endif