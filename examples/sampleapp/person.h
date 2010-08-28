#ifndef PERSON_H
#define PERSON_H

#include <string>

class Person
{
public:
    Person(const std::string &firstName, const std::string &lastName);
    std::string toString() const;

private:
    std::string m_firstName;
    std::string m_lastName;
};


#endif
