#include "Timing.h"

namespace planning
{
   //***** Constructeurs *****

  const string Timing::MONDAY = "Lundi";
  const string Timing::TUESDAY = "Mardi";
  const string Timing::WEDNESDAY = "Mercredi";
  const string Timing::THURSDAY = "Jeudi";
  const string Timing::FRIDAY = "Vendredi";
  const string Timing::SATURDAY = "Samedi";
  const string Timing::SUNDAY = "Dimanche";
  const string Timing::DAYS[7] = {Timing::MONDAY,Timing::TUESDAY,Timing::WEDNESDAY,Timing::THURSDAY,Timing::FRIDAY,Timing::SATURDAY,Timing::SUNDAY};

  Timing::Timing() noexcept
  {
    day = "---";
  }

  Timing::Timing(string d, Time s, Time dt)
  {
    setDay(d);
    setStart(s);
    setDuration(dt);
  }

  Timing::Timing(const Timing& t) noexcept
  {
    setDay(t.getDay());
    setStart(t.getStart());
    setDuration(t.getDuration());
  }

  // ***** Destructeur *****
  Timing::~Timing()
  {
  }

  // ***** getXXX / setXXX *****
  string Timing::getDay() const noexcept
  {
    return day;
  }

  Time Timing::getStart() const noexcept
  { 
    return start;
  }

  Time Timing::getDuration() const noexcept
  {
    return duration;
  }

  void Timing::setDay(string d)
  {
    for(int i = 0; i < 7; i++)
    {
      if(d == DAYS[i]){
        day = d;
        return;
      }
    }
    throw TimingException("---Jour saisie invalide---", TimingException::INVALID_DAY);
  }

  void Timing::setStart(Time s)
  {
    start.setHour(s.getHour());
    start.setMinute(s.getMinute());
  }

  void Timing::setDuration(Time dt)
  {
    duration.setHour(dt.getHour());
    duration.setMinute(dt.getMinute());
  }

  // ***** Autres methodes de l'interface *****
  void Timing::display() const noexcept
  {
    cout << "jours : " << day << endl;
    start.display();
    duration.display();
  }

  string Timing::toString() const noexcept
  {
    ostringstream oss;
    oss << day << ";" << start.toString() << ";" << duration.toString();
    return oss.str();
  }

  bool Timing::intersect(const Timing& t)
  {
    if (day != t.getDay())
    {
      return false;
    }

    Time thisFin = start + duration;
    Time tFin = t.getStart() + t.getDuration();

    if (start.getHour() > tFin.getHour() || (this->start.getHour() == tFin.getHour() && this->start.getMinute() >= tFin.getMinute())) {
        return false;
    }

    // Si l'heure de fin de l'objet courant est avant le début de l'autre intervalle, il n'y a pas de chevauchement
    if (thisFin.getHour() < t.start.getHour() || (thisFin.getHour() == t.start.getHour() && thisFin.getMinute() <= t.start.getMinute())) {
        return false;
    }

    // Si aucune des conditions ci-dessus n'est vraie, alors il y a un chevauchement
    return true;
  }

  ostream& operator<<(ostream& o, const Timing& tmg)
  {
    o << "<Timing>" << endl;
    o << "<day>" << endl;
    o << tmg.day << endl;
    o << "</day>" << endl;
    o << "<start>" << endl;
    o << tmg.start << endl;
    o << "</start>" << endl;
    o << "<duration>" << endl;
    o << tmg.duration << endl;
    o << "</duration>" << endl;
    o << "</Timing>";
    return o;
  }

  istream& operator>>(istream& i, Timing& tmg)
  {
    string sDay, tag;
    Time tempStart, tempDuration;
    getline(i, tag);
    getline(i, tag);
    getline(i, sDay);
    getline(i, tag);
    getline(i, tag);
    i >> tempStart;
    getline(i, tag);
    getline(i, tag);
    i >> tempDuration;
    getline(i, tag);
    getline(i, tag);

    tmg.setDay(sDay);
    tmg.setStart(tempStart);
    tmg.setDuration(tempDuration);
    return i;
  }

  Timing& Timing::operator=(const Timing& t) noexcept
  {
    setDay(t.getDay());
    setStart(t.getStart());
    setDuration(t.getDuration());
    return(*this);
  }

  int Timing::operator<(const Timing& t) noexcept{
    return compT(t) == -1;
  }

  int Timing::operator>(const Timing& t) noexcept{
    return compT(t) == 1;
  }

  int Timing::operator==(const Timing& t) noexcept{
    return compT(t) == 0;
  }

  int Timing::compT(const Timing& t) noexcept
  {
    if(day < t.day) return -1;
    if(day > t.day) return 1;
    if (start < t.start) return -1;
    if (start > t.start) return 1;
    if (duration < t.duration) return -1;
    if (duration > t.duration) return 1;
    return 0; 
  }
}